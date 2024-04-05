/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MConnectDialog.hpp"
#include "MAlerts.hpp"
#include "MAuthDialog.hpp"
#include "MFile.hpp"
#include "MPreferences.hpp"
#include "MPreferencesDialog.hpp"
#include "MSaltApp.hpp"
#include "MSound.hpp"
#include "MStrings.hpp"
#include "MTerminalView.hpp"
#include "MTerminalWindow.hpp"

#include <filesystem>
#include <fstream>
#include <regex>

namespace fs = std::filesystem;

namespace
{

#define USER "(?:([-$_.+!*'(),a-zA-Z0-9;?&=]+)@)?"
#define HOST "([-a-zA-Z0-9.]+)"
#define PORT R"((?::(\d+))?)"

#define CONNECT_INFO_BASE USER HOST PORT
#define PROXY_INFO CONNECT_INFO_BASE R"((?:;(.+))?)"
#define CONNECT_INFO CONNECT_INFO_BASE "(?: via " PROXY_INFO ")?"

const std::regex kRecentRX(CONNECT_INFO);
const std::regex kProxyRX(PROXY_INFO);
} // namespace

// --------------------------------------------------------------------

std::ostream &operator<<(std::ostream &os, const ConnectInfoBase &c)
{
	if (not c.user.empty())
		os << c.user << '@';
	os << c.host;
	if (c.port != 0 and c.port != 22)
		os << ':' << c.port;
	return os;
}

std::ostream &operator<<(std::ostream &os, const ProxyInfo &c)
{
	os << static_cast<const ConnectInfoBase &>(c);
	if (not c.command.empty())
		os << ';' << c.command;
	return os;
}

std::ostream &operator<<(std::ostream &os, const ConnectInfo &c)
{
	os << static_cast<const ConnectInfoBase &>(c);
	if (c.proxy.has_value())
		os << " via " << c.proxy.value();

	return os;
}

std::string ConnectInfo::DisplayString() const
{
	std::stringstream os;
	if (not user.empty())
		os << user << '@';
	os << host;

	if (proxy.has_value())
		os << " (via " << proxy->host << ")";

	return os.str();
}

ConnectInfo ConnectInfo::parse(const std::string &s)
{
	ConnectInfo ci{};

	std::smatch m;
	if (std::regex_match(s, m, kRecentRX))
	{
		ci.user = m[1];
		ci.host = m[2];
		if (m[3].matched)
			ci.port = std::stoi(m[3]);
		if (m[5].matched)
		{
			ProxyInfo pi;
			pi.user = m[4];
			pi.host = m[5];
			if (m[6].matched)
				pi.port = std::stoi(m[6]);
			pi.command = m[7];
			ci.proxy = pi;
		}
	}

	return ci;
}

// --------------------------------------------------------------------

MConnectDialog::MConnectDialog()
	: MDialog("connect-dialog")
{
	SetOpen("more-expander", false);

	std::smatch m;
	std::vector<std::string> hosts;

	for (auto &s : MPrefs::GetArray("recent-proxies"))
	{
		if (not std::regex_match(s, m, kProxyRX))
			continue;

		hosts.push_back(m[2]);

		ProxyInfo pi;
		pi.user = m[1];
		pi.host = m[2];
		if (m[3].matched)
			pi.port = std::stoi(m[3]);
		pi.command = m[4];

		mRecentProxies.push_back(pi);
	}

	SetChoices("proxy-host", hosts);

	hosts.clear();

	mRecentSessions = GetRecentHosts();

	for (auto &ci : mRecentSessions)
		hosts.emplace_back(ci.HostAndPortString());

	SetChoices("host", hosts);

	if (not mRecentSessions.empty())
		SelectRecent(mRecentSessions.front());

	Show(nullptr);
	SetFocus("host");

	if (IsChecked("use-proxy"))
		CheckboxChanged("use-proxy", true);
}

std::vector<ConnectInfo> MConnectDialog::GetRecentHosts()
{
	std::vector<ConnectInfo> result;

	for (auto &s : MPrefs::GetArray("recent-sessions"))
	{
		auto ci = ConnectInfo::parse(s);
		if (ci)
			result.push_back(std::move(ci));
	}

	return result;
}

void MConnectDialog::SelectedPrivateKey(const std::filesystem::path &inPemFile)
{
	std::ifstream file(inPemFile);
	if (not file.is_open())
		throw std::runtime_error("Could not open private key file");

	std::string key;
	for (;;)
	{
		std::string line;
		getline(file, line);
		if (line.empty() and file.eof())
			break;
		key += line + "\n";
	}

	auto fileName = inPemFile.filename().string();

	if (not pinch::ssh_agent::instance().add(key, fileName))
	{
		MAuthDialog::RequestSimplePassword(_("Adding Private Key"),
			FormatString("Please enter password for the private key ^0", fileName),
			this, [key, fileName, this](const std::string &password)
			{ pinch::ssh_agent::instance().add(key, fileName, password); });
	}
}

void MConnectDialog::ButtonClicked(const std::string &inID)
{
	//	if (inID == "more-expander")
	//		SetVisible("more-box", IsOpen("more-expander"));
	//	else
	if (inID == "priv-key")
		MFileDialogs::ChooseOneFile(this, std::bind(&MConnectDialog::SelectedPrivateKey, this, std::placeholders::_1));
	else
		MDialog::ButtonClicked(inID);
}

bool MConnectDialog::CancelClicked()
{
	if (MTerminalView::GetFrontTerminal() == nullptr)
		MPreferencesDialog::Instance().Select();
	return true;
}

bool MConnectDialog::OKClicked()
{
	MWindow *w = nullptr;

	ConnectInfo ci{};

	std::string host = GetText("host");

	const std::regex hostRX("^" HOST PORT "$");
	std::smatch m;
	if (not std::regex_match(host, m, hostRX))
		throw std::runtime_error("Invalid host name");

	ci.host = m[1];
	if (m[2].matched)
		ci.port = std::stoi(m[2]);

	ci.user = GetText("user");

	if (IsChecked("use-proxy"))
	{
		ProxyInfo pi{};

		host = GetText("proxy-host");

		if (not std::regex_match(host, m, hostRX))
			throw std::runtime_error("Invalid proxy host name");

		pi.host = m[1];
		if (m[2].matched)
			pi.port = std::stoi(m[2]);

		pi.user = GetText("proxy-user");
		pi.command = GetText("proxy-command");
		ci.proxy = pi;
	}

	try
	{
		MSaltApp::Instance().Open(ci, GetText("ssh-command"));
	}
	catch (const std::exception &e)
	{
		DisplayError(e);
		return false;
	}

	return true;
}

void MConnectDialog::ValueChanged(const std::string &inID, int32_t inValue)
{
	if (inID == "host" and inValue >= 0 and inValue < mRecentSessions.size())
		SelectRecent(mRecentSessions.at(inValue));
	else if (inID == "proxy-host" and inValue >= 0 and inValue <= mRecentProxies.size())
		SelectProxy(mRecentProxies.at(inValue));
}

void MConnectDialog::SelectRecent(const ConnectInfo &inRecent)
{
	for (auto &recent : mRecentSessions)
	{
		if (inRecent.host != recent.host or inRecent.proxy != recent.proxy)
			continue;

		SetText("user", recent.user);
		if (GetText("host") != recent.HostAndPortString())
			SetText("host", recent.HostAndPortString());

		bool expand = false;

		if (recent.proxy.has_value())
		{
			expand = true;
			SetChecked("use-proxy", true);
			SelectProxy(recent.proxy.value());
		}
		else
			SelectProxy({});

		SetOpen("more-expander", expand);
		break;
	}
}

void MConnectDialog::SelectProxy(const ProxyInfo &inProxy)
{
	if (not inProxy)
	{
		SetChecked("use-proxy", false);
		SetEnabled("proxy-user", false);
		SetEnabled("proxy-host", false);
		SetEnabled("proxy-command", false);
	}
	else
	{
		SetEnabled("proxy-user", true);
		SetEnabled("proxy-host", true);
		SetEnabled("proxy-command", true);

		SetText("proxy-user", inProxy.user);
		SetText("proxy-host", inProxy.HostAndPortString());
		SetText("proxy-command", inProxy.command);
	}
}

void MConnectDialog::CheckboxChanged(const std::string &inID, bool inValue)
{
	if (inID == "use-proxy")
	{
		SetEnabled("proxy-user", inValue);
		SetEnabled("proxy-host", inValue);
		SetEnabled("proxy-command", inValue);
	}
}
