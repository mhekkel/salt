// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

#include "MConnectDialog.hpp"
#include "MTerminalWindow.hpp"
#include "MTerminalView.hpp"
#include "MPreferences.hpp"
#include "MPreferencesDialog.hpp"
#include "MSound.hpp"
#include "MFile.hpp"
#include "MSaltApp.hpp"
#include "MStrings.hpp"
#include "MAuthDialog.hpp"

using namespace std;

namespace {

#define USER "(?:([-$_.+!*'(),[:alnum:];?&=]+)@)?"
#define HOST "([-[:alnum:].]+(?::\\d+)?)"

const boost::regex
	kServerPortRE("^([-[:alnum:].]+):(\\d+)$"),
//	kSessionRE("^(?:([-$_.+!*'(),[:alnum:];?&=]+)@)?([-[:alnum:].]+(?::\\d+)?)(?:;(.+))?$"),
	kRecentRE("^" USER HOST "(?:;" USER HOST ";(.+))?(?: >> (.+))?$"),
	kProxyRE("^" USER HOST ";(.+)$");
}

MConnectDialog::MConnectDialog()
	: MDialog("connect-dialog")
{
//	SetVisible("more-box", false);
	SetOpen("more-expander", false);
	SetText("proxy-command", "/usr/bin/nc %h %p");

	//SetChecked("use-proxy", false);
	//SetEnabled("proxy-user", false);
	//SetEnabled("proxy-host", false);
	//SetEnabled("proxy-command", false);

	boost::smatch m;
	vector<string> ss, hosts;

	Preferences::GetArray("recent-proxies", ss);
	for (string& s: ss)
	{
		if (boost::regex_match(s, m, kProxyRE))
		{
			hosts.push_back(m[2]);
			
			if (mRecentProxies.empty())
			{
				SetText("proxy-user", m[1]);
				SetText("proxy-host", m[2]);
				SetText("proxy-command", m[3]);
			}
			
			mRecentProxies.push_back(s);
		}
	}
	
	SetChoices("proxy-host", hosts);
	
	hosts.clear();

	Preferences::GetArray("recent-sessions", ss);
	for (string& s: ss)
	{
		if (boost::regex_match(s, m, kRecentRE))
		{
			hosts.push_back(m[2]);
			mRecentSessions.push_back(s);
		}
	}
	
	if (not hosts.empty())
	{
		SetChoices("host", hosts);
		SetText("host", hosts.front());
		SelectRecent(hosts.front());
	}

	Show(nullptr);
	SetFocus("host");

	if (IsChecked("use-proxy"))
		CheckboxChanged("use-proxy", true);
}

void MConnectDialog::ButtonClicked(const string& inID)
{
//	if (inID == "more-expander")
//		SetVisible("more-box", IsOpen("more-expander"));
//	else
	if (inID == "priv-key")
	{
		fs::path pemFile;

		if (MFileDialogs::ChooseOneFile(this, pemFile))
		{
			fs::ifstream file(pemFile);
			if (not file.is_open())
				throw runtime_error("Could not open private key file");

			string key;
			for (;;)
			{
				string line;
				getline(file, line);
				if (line.empty() and file.eof())
					break;
				key += line + "\n";
			}

			pinch::ssh_agent::instance().add(key, pemFile.filename().string(),
				MAuthDialog::RequestSimplePassword(_("Adding Private Key"),
						FormatString("Please enter password for the private key ^0", pemFile.filename().string())));
		}
	}
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
	MWindow* w = nullptr;
	
	string host = GetText("host");
	string user = GetText("user");
	string port = "22";
	string command = GetText("ssh-command");
	
	string recent;
	
	for (;;)
	{
		if (host.empty())
			break;
		
		recent = user + '@' + host;
		
		boost::smatch m;
		if (boost::regex_match(host, m, kServerPortRE))
		{
			port = m[2];
			host = m[1];
		}
		
		if (host.empty() or user.empty())
			break;

		pinch::connection_pool& pool(static_cast<MSaltApp*>(gApp)->GetConnectionPool());
		
		if (not IsChecked("use-proxy"))
		{
			auto connection = pool.get(user, host, std::stoi(port));
			w = MTerminalWindow::Create(user, host, std::stoi(port), command, connection);
			break;
		}

		string proxy_host = GetText("proxy-host");
		string proxy_user = GetText("proxy-user");
		string proxy_port = "22";
		string proxy_cmd = GetText("proxy-command");

		string proxy = proxy_user + '@' + proxy_host + ';' + proxy_cmd;
		recent = recent + ';' + proxy;

		if (boost::regex_match(proxy_host, m, kServerPortRE))
		{
			proxy_port = m[2];
			proxy_host = m[1];
		}
		
		if (proxy_host.empty() or proxy_user.empty() or proxy_cmd.empty())
			break;
		
		std::shared_ptr<pinch::basic_connection> connection(
			pool.get(user, host, std::stoi(port),
				proxy_user, proxy_host, std::stoi(proxy_port), proxy_cmd));

		w = MTerminalWindow::Create(user, host, std::stoi(port), command, connection);
		
		// store this recent proxy
		vector<string> ss;
		Preferences::GetArray("recent-proxies", ss);
		ss.erase(remove(ss.begin(), ss.end(), proxy), ss.end());
		ss.insert(ss.begin(), proxy);
		if (ss.size() > 10)
			ss.erase(ss.begin() + 10, ss.end());
		Preferences::SetArray("recent-proxies", ss);
		
		break;
	}
	
	if (w == nullptr)
		PlaySound("warning");
	else
	{
		w->Select();

		if (not command.empty())
			static_cast<MSaltApp*>(gApp)->AddRecent(recent + " >> " + command);
		else
			static_cast<MSaltApp*>(gApp)->AddRecent(recent);
	}
	
	return w != nullptr;
}

void MConnectDialog::TextChanged(const string& inID, const string& inText)
{
	boost::smatch m;

	if (inID == "host")
		SelectRecent(inText);
	else if (inID == "proxy-host")
		SelectProxy(inText);
}

void MConnectDialog::SelectRecent(const string& inRecent)
{
	for (const string& recent: mRecentSessions)
	{
		boost::smatch m;
		if (boost::regex_match(recent, m, kRecentRE) and m[2] == inRecent)
		{
			SetText("user", m[1]);

			bool expand = false;
			
			if (m[6].matched)
			{
				expand = true;
				SetText("ssh-command", m[6]);
			}
			else
				SetText("ssh-command", "");

			// proxied?
			if (m[4].matched)
			{
				expand = true;

				SetChecked("use-proxy", true);

				SetText("proxy-user", m[3]);
				SetText("proxy-host", m[4]);
				SetText("proxy-command", m[5]);

				SetEnabled("proxy-user", true);
				SetEnabled("proxy-host", true);
				SetEnabled("proxy-command", true);
			}
			else
			{
				SetChecked("use-proxy", false);
				SetEnabled("proxy-user", false);
				SetEnabled("proxy-host", false);
				SetEnabled("proxy-command", false);
			}

			SetOpen("more-expander", expand);
//			SetVisible("more-box", expand);

			break;
		}
	}
}

void MConnectDialog::SelectProxy(const string& inProxy)
{
	if (inProxy.empty())
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

		string proxy_user, proxy_command = "/usr/bin/nc %h %p";
		
		for (string& recent: mRecentProxies)
		{
			boost::smatch m;
			if (boost::regex_match(recent, m, kProxyRE) and m[2] == inProxy)
			{	// we have a recent match for this host
				// automatically fill in the user
				proxy_user = m[1];
				proxy_command = m[3];
				break;
			}
		}

		SetText("proxy-user", proxy_user);
		SetText("proxy-command", proxy_command);
	}
}

void MConnectDialog::CheckboxChanged(const string& inID, bool inValue)
{
	if (inID == "use-proxy")
	{
		SetEnabled("proxy-user", inValue);
		SetEnabled("proxy-host", inValue);
		SetEnabled("proxy-command", inValue);
	}
}
