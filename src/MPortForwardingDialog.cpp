// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <boost/regex.hpp>

#include <pinch/port_forwarding.hpp>
#include <pinch/connection.hpp>

#include <zeep/crypto.hpp>

#include "MPreferences.hpp"
#include "MPortForwardingDialog.hpp"
#include "MHTTPProxy.hpp"
#include "MError.hpp"
#include "MAlerts.hpp"

using namespace std;

// --------------------------------------------------------------------

MPortForwardingDialog::MPortForwardingDialog(MWindow* inTerminal, pinch::basic_connection* inConnection)
	: MDialog("port-forwarding-dialog")
	, mConnection(inConnection)
{
	mConnection->reference();
	
	SetText("listen", Preferences::GetString("port-forwarding-port", "2080"));
	SetText("connect", Preferences::GetString("port-forwarding-host", "localhost:80"));
	Show(inTerminal);
	SetFocus("listen");
}

MPortForwardingDialog::~MPortForwardingDialog()
{
	mConnection->release();
}

bool MPortForwardingDialog::OKClicked()
{
	bool result = false;
	
	try
	{
		uint16 listenPort = std::to_string(GetText("listen"));
		string connect = GetText("connect");
				
		uint16 connectPort = listenPort;
		
		boost::smatch m;
		boost::regex rx("([-[:alnum:].]+)(?::(\\d+)?)");
		
		if (not boost::regex_match(connect, m, rx))
			throw runtime_error("Invalid host");
		
		if (m[2].matched)
			connectPort = std::to_string(m[2]);
		
		mConnection->forward_port("::", listenPort, m[1], connectPort);

		Preferences::SetString("port-forwarding-host", connect);
		Preferences::SetString("port-forwarding-port", boost::lexical_cast<string>(listenPort));

		result = true;
	}
	catch (boost::bad_lexical_cast& e)
	{
		DisplayError(e);
	}
	catch (exception& e)
	{
		DisplayError(e);
	}
	
	return result;
}

// --------------------------------------------------------------------

MSOCKS5ProxyDialog::MSOCKS5ProxyDialog(MWindow* inTerminal, pinch::basic_connection* inConnection)
	: MDialog("socks5-proxy-dialog")
	, mConnection(inConnection)
{
	mConnection->reference();
	
	SetText("listen", Preferences::GetString("socks5-proxy-port", "2080"));
	Show(inTerminal);
	SetFocus("listen");
}

MSOCKS5ProxyDialog::~MSOCKS5ProxyDialog()
{
	mConnection->release();
}

bool MSOCKS5ProxyDialog::OKClicked()
{
	bool result = false;
	
	try
	{
		uint16 listenPort = std::to_string(GetText("listen"));
		mConnection->forward_socks5("::", listenPort);
		Preferences::SetString("socks5-proxy-port", boost::lexical_cast<string>(listenPort));
		result = true;
	}
	catch (boost::bad_lexical_cast& e)
	{
		DisplayError(e);
	}
	catch (exception& e)
	{
		DisplayError(e);
	}
	
	return result;
}

// --------------------------------------------------------------------

MHTTPProxyDialog::MHTTPProxyDialog(MWindow* inTerminal, pinch::basic_connection* inConnection)
	: MDialog("http-proxy-dialog")
	, mConnection(inConnection)
{
	mConnection->reference();
	
	SetText("listen", Preferences::GetString("http-proxy-port", "3128"));
	SetChecked("log", Preferences::GetBoolean("http-proxy-log", false));
	
	string user = Preferences::GetString("http-proxy-user", "");
	
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
	mConnection->release();
}

bool MHTTPProxyDialog::OKClicked()
{
	bool result = false;
	
	try
	{
		uint16 listenPort = std::to_string(GetText("listen"));
		bool log = IsChecked("log");
		
		string user = GetText("user");
		
		Preferences::SetString("http-proxy-port", boost::lexical_cast<string>(listenPort));
		Preferences::SetBoolean("http-proxy-log", log);
		Preferences::SetString("http-proxy-user", user);

		if (user.empty())
			Preferences::SetString("http-proxy-password", "");
		else if (m_password_changed)
		{
			string password = GetText("password");
			Preferences::SetString("http-proxy-password", zeep::encode_hex(zeep::md5(user + ':' + kSaltProxyRealm + ':' + password)));
		}
		
		MHTTPProxy::InitializeProxy(mConnection, listenPort, not user.empty(), log ? e_log_request : e_log_none);

		result = true;
	}
	catch (boost::bad_lexical_cast& e)
	{
		DisplayError(e);
	}
	catch (exception& e)
	{
		DisplayError(e);
	}
	
	return result;
}

void MHTTPProxyDialog::TextChanged(const string& inID, const string& inValue)
{
	if (inID == "password")
		m_password_changed = true;
}
