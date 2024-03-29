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

#include "MPortForwardingDialog.hpp"
#include "MAlerts.hpp"
#include "MError.hpp"
#include "MHTTPProxy.hpp"
#include "MPreferences.hpp"

#include <pinch.hpp>

#include <zeep/crypto.hpp>

#include <regex>

using namespace std;

// --------------------------------------------------------------------

MPortForwardingDialog::MPortForwardingDialog(MWindow *inTerminal, std::shared_ptr<pinch::basic_connection> inConnection)
	: MDialog("port-forwarding-dialog")
	, mConnection(inConnection)
{
	SetText("listen", MPrefs::GetString("port-forwarding-port", "2080"));
	SetText("connect", MPrefs::GetString("port-forwarding-host", "localhost:80"));
	Show(inTerminal);
	SetFocus("listen");
}

MPortForwardingDialog::~MPortForwardingDialog()
{
}

bool MPortForwardingDialog::OKClicked()
{
	bool result = false;

	try
	{
		uint16_t listenPort = std::stoi(GetText("listen"));
		string connect = GetText("connect");

		uint16_t connectPort = listenPort;

		std::smatch m;
		std::regex rx("([-[:alnum:].]+)(?::(\\d+)?)");

		if (not std::regex_match(connect, m, rx))
			throw runtime_error("Invalid host");

		if (m[2].matched)
			connectPort = std::stoi(m[2]);

		mConnection->forward_port(listenPort, m[1], connectPort);

		MPrefs::SetString("port-forwarding-host", connect);
		MPrefs::SetString("port-forwarding-port", std::to_string(listenPort));

		result = true;
	}
	catch (const exception &e)
	{
		DisplayError(e);
	}

	return result;
}

// --------------------------------------------------------------------

MSOCKS5ProxyDialog::MSOCKS5ProxyDialog(MWindow *inTerminal, std::shared_ptr<pinch::basic_connection> inConnection)
	: MDialog("socks5-proxy-dialog")
	, mConnection(inConnection)
{
	SetText("listen", MPrefs::GetString("socks5-proxy-port", "2080"));
	Show(inTerminal);
	SetFocus("listen");
}

MSOCKS5ProxyDialog::~MSOCKS5ProxyDialog()
{
}

bool MSOCKS5ProxyDialog::OKClicked()
{
	bool result = false;

	try
	{
		uint16_t listenPort = std::stoi(GetText("listen"));
		mConnection->forward_socks5(listenPort);
		MPrefs::SetString("socks5-proxy-port", std::to_string(listenPort));
		result = true;
	}
	catch (const exception &e)
	{
		DisplayError(e);
	}

	return result;
}

// --------------------------------------------------------------------

MHTTPProxyDialog::MHTTPProxyDialog(MWindow *inTerminal, std::shared_ptr<pinch::basic_connection> inConnection)
	: MDialog("http-proxy-dialog")
	, mConnection(inConnection)
{
	SetText("listen", MPrefs::GetString("http-proxy-port", "3128"));
	SetChecked("log", MPrefs::GetBoolean("http-proxy-log", false));

	string user = MPrefs::GetString("http-proxy-user", "");

	SetText("user", user);
	SetPasswordChar("password");

	if (not user.empty())
		SetText("password", "xxxxxxxxxxxx");

	Show(inTerminal);
	SetFocus("listen");

	m_password_changed = false;
}

MHTTPProxyDialog::~MHTTPProxyDialog()
{
}

bool MHTTPProxyDialog::OKClicked()
{
	bool result = false;

	try
	{
		uint16_t listenPort = std::stoi(GetText("listen"));
		bool log = IsChecked("log");

		string user = GetText("user");

		MPrefs::SetString("http-proxy-port", std::to_string(listenPort));
		MPrefs::SetBoolean("http-proxy-log", log);
		MPrefs::SetString("http-proxy-user", user);

		if (user.empty())
			MPrefs::SetString("http-proxy-password", "");
		else if (m_password_changed)
		{
			string password = GetText("password");
			MPrefs::SetString("http-proxy-password", zeep::encode_hex(zeep::md5(user + ':' + kSaltProxyRealm + ':' + password)));
		}

		MHTTPProxy::instance().Init(mConnection, listenPort, not user.empty(), log ? log_level::request : log_level::none);

		result = true;
	}
	catch (const exception &e)
	{
		DisplayError(e);
	}

	return result;
}

void MHTTPProxyDialog::TextChanged(const string &inID, const string &inValue)
{
	if (inID == "password")
		m_password_changed = true;
}
