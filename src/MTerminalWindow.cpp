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

#include "MTerminalWindow.hpp"
#include "MAlerts.hpp"
#include "MAnimation.hpp"
#include "MAuthDialog.hpp"
#include "MClipboard.hpp"
#include "MControls.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MPortForwardingDialog.hpp"
#include "MPreferences.hpp"
#include "MPtyTerminalChannel.hpp"
#include "MSaltApp.hpp"
#include "MSearchPanel.hpp"
#include "MStrings.hpp"
#include "MTerminalChannel.hpp"
#include "MTerminalView.hpp"

#include "MSalt.hpp"

#include <zeep/crypto.hpp>
#include <zeep/unicode-support.hpp>

// ------------------------------------------------------------------
//

class MSshTerminalWindow : public MTerminalWindow, public pinch::connection_callback_interface
{
  public:
	MSshTerminalWindow(const std::string &inUser, const std::string &inHost, uint16_t inPort,
		const std::string &inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection);

	MTerminalWindow *Clone(MTerminalWindow *) override
	{
		return new MSshTerminalWindow(mUser, mServer, mPort, mSSHCommand, mConnection);
	}

	void OnDisconnect();
	void OnRenewKeys();
	void OnInstallPublicKey(int inKeyNr);

	void OnForwardPort();
	void OnProxySOCKS();
	void OnProxyHTTP();

  protected:
	// std::string Password();
	// std::vector<std::string> Credentials(const std::string &name, const std::string &instruction,
	// 	const std::string &lang, const std::vector<pinch::prompt> &prompts);
	// pinch::host_key_reply AcceptHostKey(const std::string &inHost,
	// 	const std::string &inAlg, const pinch::blob &inHostKey, pinch::host_key_state inState);

	callback_executor_type get_executor() override
	{
		MAppExecutor my_executor{ &MSaltApp::Instance().get_context() };
		return my_executor;
	}

	void accepts_hostkey(const std::string &host, const std::string &algorithm, const pinch::blob &key,
		pinch::host_key_state state, std::promise<pinch::host_key_reply> result) override;

	void provide_password(std::promise<std::string> result) override;

	void provide_credentials(const std::string &name, const std::string &instruction, const std::string &lang,
		const std::vector<pinch::prompt> &prompts, std::promise<std::vector<std::string>> result) override;

	MCommand<void()> cDisconnect;
	MCommand<void()> cRenewKeys;
	MCommand<void(int)> cInstallPublicKey;

	MCommand<void()> cForwardPort;
	MCommand<void()> cProxySOCKS;
	MCommand<void()> cProxyHTTP;

	std::shared_ptr<pinch::basic_connection> mConnection;
	std::string mUser, mServer;
	uint16_t mPort;
	std::string mSSHCommand;
	pinch::channel_ptr mKeyDropper;
};

MSshTerminalWindow::MSshTerminalWindow(const std::string &inUser, const std::string &inHost, uint16_t inPort,
	const std::string &inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection)
	: MTerminalWindow(MTerminalChannel::Create(inConnection), { inSSHCommand })

	, cDisconnect(this, "disconnect", &MSshTerminalWindow::OnDisconnect)
	, cRenewKeys(this, "renew-keys", &MSshTerminalWindow::OnRenewKeys)
	, cInstallPublicKey(this, "install-public-key", &MSshTerminalWindow::OnInstallPublicKey)

	, cForwardPort(this, "forward-port", &MSshTerminalWindow::OnForwardPort)
	, cProxySOCKS(this, "proxy-socks", &MSshTerminalWindow::OnProxySOCKS)
	, cProxyHTTP(this, "proxy-http", &MSshTerminalWindow::OnProxyHTTP)

	, mConnection(inConnection)
	, mUser(inUser)
	, mServer(inHost)
	, mPort(inPort)
	, mSSHCommand(inSSHCommand)
{
	using namespace std::placeholders;

	MAppExecutor my_executor{ &MSaltApp::Instance().get_context() };

	using namespace std::placeholders;

	mConnection->set_connection_callback_interface(this);

	std::stringstream title;
	title << mUser << '@' << mServer;
	if (mPort != 22)
		title << ':' << mPort;
	SetTitle(title.str());
}

void MSshTerminalWindow::OnDisconnect()
{
	mConnection->close();
}

void MSshTerminalWindow::OnRenewKeys()
{
	mConnection->rekey();
}

void MSshTerminalWindow::OnInstallPublicKey(int inKeyNr)
{
	auto &key = pinch::ssh_agent::instance().at(inKeyNr);

	// create the public key
	auto b = key.get_blob();
	std::string blob = zeep::encode_base64(std::string_view(reinterpret_cast<const char *>(b.data()), b.size()));

	// create a command
	zeep::replace_all(blob, "\n", "");
	std::string publickey = key.get_type() + ' ' + blob + ' ' + key.get_comment();
	zeep::replace_all(publickey, "'", "'\\''");

	std::string command =
		std::string("umask 077 ; test -d .ssh || mkdir .ssh ; echo '") + publickey + "' >> .ssh/authorized_keys";
	std::string comment = key.get_comment();

	MAppExecutor my_executor{ &MSaltApp::Instance().get_context() };

	mKeyDropper.reset(new pinch::exec_channel(
		mConnection, command, [this, comment](const std::string &, int status)
		{
		if (status == 0)
			DisplayAlert(this, "installed-public-key", {comment, this->mServer});
		else
			DisplayAlert(this, "failed-to-install-public-key", {comment, this->mServer}); },
		my_executor));

	mKeyDropper->open();
}

void MSshTerminalWindow::OnForwardPort()
{
	new MPortForwardingDialog(this, mConnection);
}

void MSshTerminalWindow::OnProxySOCKS()
{
	new MSOCKS5ProxyDialog(this, mConnection);
}

void MSshTerminalWindow::OnProxyHTTP()
{
	new MHTTPProxyDialog(this, mConnection);
}

void MSshTerminalWindow::accepts_hostkey(const std::string &host, const std::string &algorithm, const pinch::blob &key,
	pinch::host_key_state state, std::promise<pinch::host_key_reply> reply)
{
	std::string_view hsv(reinterpret_cast<const char *>(key.data()), key.size());

	std::string value = zeep::encode_base64(hsv);
	std::string H = zeep::md5(hsv);

	std::string fingerprint;

	for (auto b : H)
	{
		if (not fingerprint.empty())
			fingerprint += ':';

		fingerprint += kHexChars[(b >> 4) & 0x0f];
		fingerprint += kHexChars[b & 0x0f];
	}

	if (state == pinch::host_key_state::keys_differ)
	{
		DisplayAlert(this, "host-key-changed-alert",
			[reply = std::move(reply)](int btn) mutable
			{
				reply.set_value(btn == 2 ? pinch::host_key_reply::trusted : pinch::host_key_reply::reject);
			},
			{ host, fingerprint });
	}
	else
	{
		DisplayAlert(this, "unknown-host-alert",
			[reply = std::move(reply)](int btn) mutable
			{
				switch (btn)
				{
					case 1: // Add
						reply.set_value(pinch::host_key_reply::trusted);
						break;

					case 2: // Cancel
						reply.set_value(pinch::host_key_reply::reject);
						break;

					case 3: // Once
						reply.set_value(pinch::host_key_reply::trust_once);
						break;
				}
			},
			{ host, fingerprint });
	}
}

void MSshTerminalWindow::provide_password(std::promise<std::string> result)
{
	auto dlog = new MAuthDialog(_("Logging in"), this, std::move(result));
	dlog->Select();
}

void MSshTerminalWindow::provide_credentials(const std::string &name, const std::string &instruction, const std::string &lang,
	const std::vector<pinch::prompt> &prompts, std::promise<std::vector<std::string>> result)
{
	auto dlog = new MAuthDialog(_("Logging in"), name,
		instruction.empty() ? FormatString("Please enter the requested info for account ^0", name) : instruction,
		prompts, this, std::move(result));
	dlog->Select();
}

// ------------------------------------------------------------------
//

class MPtyTerminalWindow : public MTerminalWindow
{
  public:
	MPtyTerminalWindow(const std::vector<std::string> &inArgv);
	MPtyTerminalWindow(MPtyTerminalWindow *inOriginal = nullptr);

	MTerminalWindow *Clone(MTerminalWindow *inOriginal)
	{
		MPtyTerminalWindow *parent = dynamic_cast<MPtyTerminalWindow *>(inOriginal);
		return new MPtyTerminalWindow(parent);
	}
};

MPtyTerminalWindow::MPtyTerminalWindow(const std::vector<std::string> &inArgv)
	: MTerminalWindow(MTerminalChannel::Create(), inArgv)
{
	SetTitle("Salt - terminal");
}

MPtyTerminalWindow::MPtyTerminalWindow(MPtyTerminalWindow *inOriginal)
	: MTerminalWindow(MTerminalChannel::Create(), {})
{
	SetTitle("Salt - terminal");

	if (inOriginal != nullptr)
	{
		auto channel = dynamic_cast<MPtyTerminalChannel *>(inOriginal->mChannel);
		if (channel != nullptr)
			static_cast<MPtyTerminalChannel *>(mChannel)->SetCWD(channel->GetCWD());
	}
}

// ------------------------------------------------------------------
//

MTerminalWindow *MTerminalWindow::sFirst = nullptr;
uint32_t MTerminalWindow::sNextNr = 1;

MTerminalWindow::MTerminalWindow(MTerminalChannel *inTerminalChannel, const std::vector<std::string> &inArgv)
	: MWindow("Terminal", GetPreferredBounds(),
		  MWindowFlags(kMPostionDefault | kMNoEraseOnUpdate | kMShowMenubar | kMDoNotHandleF10))

	, cClose(this, "close", &MTerminalWindow::OnClose, 'W', kControlKey | kShiftKey)
	, cCloneTerminal(this, "clone-terminal", &MTerminalWindow::OnCloneTerminal, 'D', kControlKey | kShiftKey)

	, cFind(this, "find", &MTerminalWindow::OnFind, 'F', kControlKey | kShiftKey)

	, cNextTerminal(this, "select-next-window", &MTerminalWindow::OnNextTerminal, kTabKeyCode, kControlKey)
	, cPrevTerminal(this, "select-previous-window", &MTerminalWindow::OnPrevTerminal, kTabKeyCode, kControlKey | kShiftKey)

	, mChannel(inTerminalChannel)
	, mNext(nullptr)
	, mNr(sNextNr++)
{
	SetIconName("salt");

	// create views
	MRect bounds;

	// Main VBox first
	bounds = GetBounds();
	mMainVBox = new MBoxControl("main-vbox", bounds, false);
	mMainVBox->SetLayout({ true, 0 });
	AddChild(mMainVBox);

	// create search panel
	bounds = GetBounds();
	bounds.height = kSearchPanelHeight;
	mSearchPanel = new MSearchPanel("searchpanel", bounds);
	mSearchPanel->SetLayout({ true, false, 0, 0, 0, 0 });
	mMainVBox->AddChild(mSearchPanel);

	// create status bar
	bounds = GetBounds();
	bounds.y += bounds.height - kScrollbarWidth;
	bounds.height = kScrollbarWidth;

	MStatusBarElement parts[] = {
		{ 250, { 4, 0, 4, 0 }, false },
		{ 275, { 4, 0, 4, 0 }, true },
		{ 60, { 4, 0, 4, 0 }, false }
	};

	mStatusbar = new MStatusbar("status", bounds, 3, parts);
	mMainVBox->AddChild(mStatusbar);
	bounds = mStatusbar->GetBounds();

	int32_t statusbarHeight = bounds.height;

	// hbox: force correct autolayout in Gtk
	MBoxControl *hbox = new MBoxControl("hbox", bounds, true);
	hbox->SetLayout({ true, 0 });
	mMainVBox->AddChild(hbox, mStatusbar);

	// create scrollbar
	bounds = GetBounds();
	bounds.height -= statusbarHeight + kSearchPanelHeight;
	bounds.y += kSearchPanelHeight;
	bounds.x += bounds.width - kScrollbarWidth;
	bounds.width = kScrollbarWidth;
	mScrollbar = new MScrollbar("scrollbar", bounds);
	hbox->AddChild(mScrollbar);

	uint32_t w, h;
	MTerminalView::GetTerminalMetrics(80, 24, false, w, h);
	bounds = MRect(0, 0, w, h);

	mTerminalView = new MTerminalView("terminalview", bounds, mStatusbar, mScrollbar, mSearchPanel, inTerminalChannel, inArgv);
	mTerminalView->SetLayout({ true, 0 });

	hbox->AddChild(mTerminalView, mScrollbar);

	AddRoute(mTerminalView->eScroll, mScrollbar->eScroll);

	mSearchPanel->Hide();

	if (MPrefs::GetBoolean("show-status-bar", true) == false)
		mStatusbar->Hide();

	// add to bottom of the list
	if (sFirst == nullptr)
		sFirst = this;
	else
	{
		MTerminalWindow *w = sFirst;
		while (w->mNext != nullptr)
			w = w->mNext;
		w->mNext = this;
	}

	SetLatentFocus(mTerminalView);

	mTerminalView->Open();
}

MTerminalWindow::~MTerminalWindow()
{
	// remove from the list
	if (sFirst == this)
		sFirst = mNext;
	else
	{
		MTerminalWindow *w = sFirst;
		while (w->mNext != this)
			w = w->mNext;
		assert(w != nullptr);
		if (w != nullptr)
			w->mNext = mNext;
	}

	MSaltApp::Instance().UpdateWindowMenu();
}

void MTerminalWindow::SetTitle(const std::string &inTitle)
{
	MWindow::SetTitle(inTitle);
	MSaltApp::Instance().UpdateWindowMenu();
}

void MTerminalWindow::ShowSelf()
{
	mTerminalView->SetFocus();
}

void MTerminalWindow::FocusTerminalView()
{
	mTerminalView->SetFocus();
}

MRect MTerminalWindow::GetPreferredBounds()
{
	uint32_t w, h;
	MTerminalView::GetTerminalMetrics(80, 24, false, w, h);

	return MRect(0, 0, w, h + kScrollbarWidth);
}

void MTerminalWindow::OnClose()
{
	if (AllowClose(false))
		Close();
}

void MTerminalWindow::OnCloneTerminal()
{
	MTerminalWindow *clone = Clone(this);
	clone->Select();
}

void MTerminalWindow::OnFind()
{
	if (mSearchPanel->IsVisible())
		HideSearchPanel();
	else
		ShowSearchPanel();
}

void MTerminalWindow::OnNextTerminal()
{
	if (mNext != nullptr)
		mNext->Select();
	else if (sFirst != this)
		sFirst->Select();
}

void MTerminalWindow::OnPrevTerminal()
{
	if (sFirst == this)
	{
		MTerminalWindow *w = sFirst;
		while (w->mNext != nullptr)
			w = w->mNext;
		if (w != this)
			w->Select();
	}
	else
	{
		MTerminalWindow *w = sFirst;
		while (w != nullptr and w->mNext != this)
			w = w->mNext;
		if (w != nullptr and w != this)
			w->Select();
	}
}

void MTerminalWindow::ShowSearchPanel()
{
	mSearchPanel->Show();
	mSearchPanel->SetFocus();
}

void MTerminalWindow::HideSearchPanel()
{
	mSearchPanel->Hide();
	mTerminalView->SetFocus();
}

bool MTerminalWindow::IsAnyTerminalOpen()
{
	bool result = false;
	MTerminalWindow *w = GetFirstTerminal();

	while (result == false and w != nullptr)
	{
		result = w->mTerminalView->IsOpen();
		w = w->GetNextTerminal();
	}

	return result;
}

bool MTerminalWindow::AllowClose(bool inLogOff)
{
	bool result = true;

	if (mTerminalView->IsOpen())
	{
		Select();
		DisplayAlert(this, "close-session-alert", [this](int result)
			{ if (result == 1) Close(); },
			{});
		result = false;
	}

	return result;
}

void MTerminalWindow::Close()
{
	mTerminalView->Destroy();
	MWindow::Close();
}

// bool MTerminalWindow::UpdateCommandStatus(
// 	uint32_t inCommand,
// 	MMenu *inMenu,
// 	uint32_t inItemIndex,
// 	bool &outEnabled,
// 	bool &outChecked)
// {
// 	bool result = true;

// 	switch (inCommand)
// 	{
// 		case cmd_Disconnect:
// 			outEnabled = mChannel != nullptr && mChannel->CanDisconnect();
// 			break;

// 		case cmd_NextTerminal:
// 		case cmd_PrevTerminal:
// 			outEnabled = sFirst != this or mNext != nullptr;
// 			break;

// 		case cmd_Find:
// 		case cmd_CloneTerminal:
// 			outEnabled = true;
// 			break;

// 		default:
// 			result = MWindow::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
// 			break;
// 	}

// 	return result;
// }

// bool MTerminalWindow::ProcessCommand(
// 	uint32_t inCommand,
// 	const MMenu *inMenu,
// 	uint32_t inItemIndex,
// 	uint32_t inModifiers)
// {
// 	bool result = true;

// 	switch (inCommand)
// 	{
// 		case cmd_Disconnect:
// 			mChannel->Disconnect(inModifiers & kControlKey);
// 			break;

// 		case cmd_CloneTerminal:
// 		{
// 			MTerminalWindow *clone = Clone(this);
// 			clone->Select();
// 			break;
// 		}

// 			// case cmd_Explore:
// 			//{
// 			//	MExploreBrowserWindow* explorer = new MExploreBrowserWindow(mConnection);
// 			//	explorer->Select();
// 			//	break;
// 			// }

// 		case cmd_NextTerminal:
// 			if (mNext != nullptr)
// 				mNext->Select();
// 			else if (sFirst != this)
// 				sFirst->Select();
// 			break;

// 		case cmd_PrevTerminal:
// 			if (sFirst == this)
// 			{
// 				MTerminalWindow *w = sFirst;
// 				while (w->mNext != nullptr)
// 					w = w->mNext;
// 				if (w != this)
// 					w->Select();
// 			}
// 			else
// 			{
// 				MTerminalWindow *w = sFirst;
// 				while (w != nullptr and w->mNext != this)
// 					w = w->mNext;
// 				if (w != nullptr and w != this)
// 					w->Select();
// 			}
// 			break;

// 		case cmd_Find:
// 			if (mSearchPanel->IsVisible())
// 				HideSearchPanel();
// 			else
// 				ShowSearchPanel();
// 			break;

// 		case cmd_HideSearchPanel:
// 			HideSearchPanel();
// 			break;

// 		default:
// 			result = MWindow::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
// 			break;
// 	}

// 	return result;
// }

// ------------------------------------------------------------------
//

MTerminalWindow *MTerminalWindow::Create(const std::vector<std::string> &inArgv)
{
	return new MPtyTerminalWindow(inArgv);
}

MTerminalWindow *MTerminalWindow::Create(const std::string &inUser, const std::string &inHost, uint16_t inPort,
	const std::string &inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection)
{
	return new MSshTerminalWindow(inUser, inHost, inPort, inSSHCommand, inConnection);
}
