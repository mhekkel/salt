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
	//	SetVisible("more-box", false);
	SetOpen("more-expander", false);
	// SetText("proxy-command", "/usr/bin/nc %h %p");

	// SetChecked("use-proxy", false);
	// SetEnabled("proxy-user", false);
	// SetEnabled("proxy-host", false);
	// SetEnabled("proxy-command", false);

	std::smatch m;
	std::vector<std::string> ss, hosts;

	Preferences::GetArray("recent-proxies", ss);

	for (auto &s : ss)
	{
		if (std::regex_match(s, m, kProxyRX))
		{
			hosts.push_back(m[2]);

			ProxyInfo pi;
			pi.user = m[1];
			pi.host = m[2];
			if (m[3].matched)
				pi.port = std::stoi(m[3]);
			pi.command = m[4];

			mRecentProxies.push_back(pi);
		}
	}

	SetChoices("proxy-host", hosts);

	hosts.clear();

	std::vector<std::string> sa;
	Preferences::GetArray("recent-sessions", sa);

	for (auto &s : sa)
	{
		mRecentSessions.push_back(ConnectInfo::parse(s));
		hosts.emplace_back(mRecentSessions.back().host);
	}

	if (not hosts.empty())
	{
		SetChoices("host", hosts);
		SetText("host", hosts.front());
		// SelectRecent(hosts.front());
	}

	if (not mRecentSessions.empty())
		SelectRecent(mRecentSessions.front());

	Show(nullptr);
	SetFocus("host");

	if (IsChecked("use-proxy"))
		CheckboxChanged("use-proxy", true);
}

std::vector<ConnectInfo> MConnectDialog::GetRecentHosts()
{
	std::vector<std::string> sa;
	Preferences::GetArray("recent-sessions", sa);

	std::vector<ConnectInfo> result;

	for (auto &s : sa)
		result.push_back(ConnectInfo::parse(s));
	
	return result;
}

void MConnectDialog::ButtonClicked(const std::string &inID)
{
	//	if (inID == "more-expander")
	//		SetVisible("more-box", IsOpen("more-expander"));
	//	else
	if (inID == "priv-key")
		MFileDialogs::ChooseOneFile(this, [this](std::filesystem::path pemFile)
			{
				std::ifstream file(pemFile);
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

				pinch::ssh_agent::instance().add(key, pemFile.filename().string(),
					MAuthDialog::RequestSimplePassword(_("Adding Private Key"),
						FormatString("Please enter password for the private key ^0", pemFile.filename().string()), this));
			});
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

	

	// std::string host = GetText("host");

	// auto via = host.find(" via ");
	// if (via != std::string::npos)
	// 	host = host.substr(0, via);

	// std::string user = GetText("user");
	// std::string port = "22";
	// std::string command = GetText("ssh-command");

	// std::string recent;

	// for (;;)
	// {
	// 	if (host.empty())
	// 		break;

	// 	recent = user + '@' + host;

	// 	std::smatch m;
	// 	if (std::regex_match(host, m, kServerPortRE))
	// 	{
	// 		port = m[2];
	// 		host = m[1];
	// 	}

	// 	if (host.empty() or user.empty())
	// 		break;

	// 	pinch::connection_pool &pool(static_cast<MSaltApp *>(gApp)->GetConnectionPool());

	// 	if (not IsChecked("use-proxy"))
	// 	{
	// 		auto connection = pool.get(user, host, std::stoi(port));
	// 		w = MTerminalWindow::Create(user, host, std::stoi(port), command, connection);
	// 		break;
	// 	}

	// 	string proxy_host = GetText("proxy-host");
	// 	string proxy_user = GetText("proxy-user");
	// 	string proxy_port = "22";
	// 	string proxy_cmd = GetText("proxy-command");

	// 	string proxy = proxy_user + '@' + proxy_host + ';' + proxy_cmd;
	// 	recent = recent + ';' + proxy;

	// 	if (std::regex_match(proxy_host, m, kServerPortRE))
	// 	{
	// 		proxy_port = m[2];
	// 		proxy_host = m[1];
	// 	}

	// 	if (proxy_host.empty() or proxy_user.empty())
	// 		break;

	// 	std::shared_ptr<pinch::basic_connection> connection(
	// 		pool.get(user, host, std::stoi(port),
	// 			proxy_user, proxy_host, std::stoi(proxy_port), proxy_cmd));

	// 	w = MTerminalWindow::Create(user, host, std::stoi(port), command, connection);

	// 	// store this recent proxy
	// 	vector<string> ss;
	// 	Preferences::GetArray("recent-proxies", ss);
	// 	ss.erase(remove(ss.begin(), ss.end(), proxy), ss.end());
	// 	ss.insert(ss.begin(), proxy);
	// 	if (ss.size() > 10)
	// 		ss.erase(ss.begin() + 10, ss.end());
	// 	Preferences::SetArray("recent-proxies", ss);

	// 	break;
	// }

	// if (w == nullptr)
	// 	PlaySound("warning");
	// else
	// {
	// 	w->Select();

	// 	if (not command.empty())
	// 		static_cast<MSaltApp *>(gApp)->AddRecent(recent + " >> " + command);
	// 	else
	// 		static_cast<MSaltApp *>(gApp)->AddRecent(recent);
	// }

	return w != nullptr;
}

void MConnectDialog::TextChanged(const std::string &inID, const std::string &inText)
{
	if (inID == "host")
		SelectRecent(ConnectInfo::parse(inText));
	// else if (inID == "proxy-host")
	// 	SelectProxy(inText);
}

void MConnectDialog::SelectRecent(const ConnectInfo &inRecent)
{
	for (auto &recent : mRecentSessions)
	{
		if (inRecent.host != recent.host or inRecent.proxy != recent.proxy)
			continue;

		SetText("user", recent.user);

		bool expand = false;

		if (recent.proxy.has_value())
		{
			expand = true;
			SetText("ssh-command", recent.proxy->command);

			SetChecked("use-proxy", true);

			SetText("proxy-user", recent.proxy->user);
			SetText("proxy-host", recent.proxy->host);
			SetText("proxy-command", recent.proxy->command);

			SetEnabled("proxy-user", true);
			SetEnabled("proxy-host", true);
			SetEnabled("proxy-command", true);
		}
		else
		{
			SetText("ssh-command", "");
			SetChecked("use-proxy", false);
			SetEnabled("proxy-user", false);
			SetEnabled("proxy-host", false);
			SetEnabled("proxy-command", false);
		}

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
