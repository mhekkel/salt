// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MDialog.hpp"

// class MPortForwardingDialog : public MDialog
// {
//   public:
// 	 MPortForwardingDialog(MWindow* inTerminal, std::shared_ptr<pinch::basic_connection> inConnection);
// 	 ~MPortForwardingDialog();

// 	virtual bool OKClicked();

//   private:
// 	std::shared_ptr<pinch::basic_connection> mConnection;
// };

// class MSOCKS5ProxyDialog : public MDialog
// {
//   public:
// 	 MSOCKS5ProxyDialog(MWindow* inTerminal, std::shared_ptr<pinch::basic_connection> inConnection);
// 	 ~MSOCKS5ProxyDialog();

// 	virtual bool OKClicked();

//   private:
// 	std::shared_ptr<pinch::basic_connection> mConnection;
// };

// class MHTTPProxyDialog : public MDialog
// {
//   public:
// 	 MHTTPProxyDialog(MWindow* inTerminal, std::shared_ptr<pinch::basic_connection> inConnection);
// 	 ~MHTTPProxyDialog();

// 	virtual bool OKClicked();
// 	virtual void TextChanged(const std::string& inID, const std::string& inValue);

//   private:
// 	std::shared_ptr<pinch::basic_connection> mConnection;
// 	bool m_password_changed;
// };