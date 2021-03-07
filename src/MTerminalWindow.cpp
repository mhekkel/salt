// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <cryptopp/base64.h>

#include <boost/algorithm/string.hpp>

#include "MTerminalWindow.hpp"
#include "MControls.hpp"
#include "MTerminalView.hpp"
#include "MSaltApp.hpp"
#include "MPreferences.hpp"
#include "MMenu.hpp"
#include "MResources.hpp"
#include "MAuthDialog.hpp"
#include "MStrings.hpp"
#include "MPortForwardingDialog.hpp"
#include "MSearchPanel.hpp"
#include "MAnimation.hpp"
#include "MAlerts.hpp"
#include "MTerminalChannel.hpp"
#include "MError.hpp"

using namespace std;
namespace ba = boost::algorithm;

// ------------------------------------------------------------------
//

class MSshTerminalWindow : public MTerminalWindow
{
  public:
	MSshTerminalWindow(const string& inUser, const string& inHost, uint16_t inPort,
							const string& inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection);

	MTerminalWindow*	Clone()
	{
		return new MSshTerminalWindow(mUser, mServer, mPort, mSSHCommand, mConnection);
	}

	virtual bool		UpdateCommandStatus(uint32_t inCommand, MMenu* inMenu, uint32_t inItemIndex, bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32_t inCommand, const MMenu* inMenu, uint32_t inItemIndex, uint32_t inModifiers);

  protected:


	void				RequestedPassword();
	void				KeyboardInteractive(const string& name, const string& language,
							const vector<pinch::prompt>& prompts);

	void				ReceivedPassword(vector<string>& inPassword);
	MEventIn<void(vector<string>&)>						eRecvPassword;

	void				DropPublicKey(pinch::ssh_private_key inKey);

	std::shared_ptr<pinch::basic_connection>
						mConnection;
	string				mUser, mServer;
	uint16_t				mPort;
	string				mSSHCommand;
	pinch::channel_ptr	mKeyDropper;
};

MSshTerminalWindow::MSshTerminalWindow(const string& inUser, const string& inHost, uint16_t inPort,
	const string& inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection)
	: MTerminalWindow(
		MTerminalChannel::Create(inConnection),
		inSSHCommand)
	, eRecvPassword(this, &MSshTerminalWindow::ReceivedPassword)
	, mConnection(inConnection)
	, mUser(inUser)
	, mServer(inHost)
	, mPort(inPort)
	, mSSHCommand(inSSHCommand)
{
	using namespace std::placeholders;

	mConnection->set_validate_callback(std::bind(&MSaltApp::ValidateHost, static_cast<MSaltApp*>(gApp), _1, _2, _3));
	mConnection->set_password_callback(std::bind(&MSshTerminalWindow::RequestedPassword, this));
	mConnection->set_keyboard_interactive_callback(std::bind(&MSshTerminalWindow::KeyboardInteractive, this, _1, _2, _3));

	stringstream title;
	title << mUser << '@' << mServer;
	if (mPort != 22)
		title << ':' << mPort;
	SetTitle(title.str());
}

bool MSshTerminalWindow::UpdateCommandStatus(
	uint32_t			inCommand,
	MMenu*			inMenu,
	uint32_t			inItemIndex,
	bool&			outEnabled,
	bool&			outChecked)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_DropPublicKey:
			outEnabled = true;
			break;
		
		case cmd_ForwardPort:
		case cmd_ProxySOCKS:
		case cmd_ProxyHTTP:
		case cmd_Rekey:
			outEnabled = mConnection->is_connected();
			break;
		
		default:
			result = MTerminalWindow::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
			break;
	}
	
	return result;
}

bool MSshTerminalWindow::ProcessCommand(
	uint32_t			inCommand,
	const MMenu*	inMenu,
	uint32_t			inItemIndex,
	uint32_t			inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_DropPublicKey:
		{
			pinch::ssh_agent& agent(pinch::ssh_agent::instance());
			
			if (inItemIndex < agent.size())
			{
				uint32_t n = inItemIndex;
				for (auto key = agent.begin(); key != agent.end(); ++key)
				{
					if (n-- > 0)
						continue;

					if (key->get_comment() == inMenu->GetItemLabel(inItemIndex))
						DropPublicKey(*key);
					
					break;
				}
			}
			break;
		}

		case cmd_Rekey:
#warning "fix"
			// mConnection->rekey();
			break;

		// case cmd_ForwardPort:
		// 	new MPortForwardingDialog(this, mConnection);
		// 	break;
		
		// case cmd_ProxySOCKS:
		// 	new MSOCKS5ProxyDialog(this, mConnection);
		// 	break;
		
		// case cmd_ProxyHTTP:
		// 	new MHTTPProxyDialog(this, mConnection);
		// 	break;
		
		default:
			result = MTerminalWindow::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}
	
	return result;
}

void MSshTerminalWindow::RequestedPassword()
{
	vector<pinch::prompt> prompts(1);
	
//	p[0] = MStrings::GetIndString(1011, 2);
	prompts[0].str = _("Password");
	prompts[0].echo = false;

	unique_ptr<MAuthDialog> dlog(new MAuthDialog(_("Logging in"),
		FormatString("Please enter password for account ^0", mUser),
		prompts));

	AddRoute(dlog->eAuthInfo, eRecvPassword);

	dlog->ShowModal(this);
	dlog.release();	
}

void MSshTerminalWindow::KeyboardInteractive(const string& name, const string& instruction,
	const vector<pinch::prompt>& prompts)
{
	unique_ptr<MAuthDialog> dlog(new MAuthDialog(_("Logging in"),
		instruction.empty() ? FormatString("Please enter the requested info for account ^0", mUser) : instruction,
		prompts));

	AddRoute(dlog->eAuthInfo, eRecvPassword);

	dlog->Show(this);
	dlog.release();
}

void MSshTerminalWindow::ReceivedPassword(vector<string>& inPassword)
{
	if (inPassword.empty())
		mConnection->disconnect();
#warning "fix"
	// else
	// 	mConnection->response(inPassword);
}

void MSshTerminalWindow::DropPublicKey(pinch::ssh_private_key inKeyToDrop)
{
	// create the public key
	pinch::opacket p;
	p << inKeyToDrop;
	
	string blob;
	CryptoPP::Base64Encoder enc(new CryptoPP::StringSink(blob));
	
	const vector<uint8_t>& data(p);
	enc.Put(&data[0], data.size());
	enc.MessageEnd(true);
	
	// create a command
	ba::replace_all(blob, "\n", "");
	string publickey = "ssh-rsa " + blob + ' ' + inKeyToDrop.get_comment();
	ba::replace_all(publickey, "'", "'\\''");
	
	string command = 
		string("umask 077 ; test -d .ssh || mkdir .ssh ; echo '") + publickey + "' >> .ssh/authorized_keys";
	string comment = inKeyToDrop.get_comment();
	
	mKeyDropper.reset(new pinch::exec_channel(mConnection, command, [this, comment](const string&, int status)
	{
		if (status == 0)
			DisplayAlert(this, "installed-public-key", comment, this->mServer);
		else
			DisplayAlert(this, "failed-to-install-public-key", comment, this->mServer);
	}));
	
	mKeyDropper->open();
}

// ------------------------------------------------------------------
//

class MPtyTerminalWindow : public MTerminalWindow
{
  public:
	MPtyTerminalWindow(boost::asio::io_service& inIOService);
	
	MTerminalWindow*	Clone()
	{
		// return new MPtyTerminalWindow(mChannel->GetIOService());
		return new MPtyTerminalWindow(static_cast<MSaltApp*>(gApp)->GetIOService());
	}
	
};

MPtyTerminalWindow::MPtyTerminalWindow(boost::asio::io_service& inIOService)
	: MTerminalWindow(MTerminalChannel::Create(inIOService), "")
{
	SetTitle("Salt - terminal");
}

// ------------------------------------------------------------------
//

MTerminalWindow* MTerminalWindow::sFirst = nullptr;

MTerminalWindow::MTerminalWindow(MTerminalChannel* inTerminalChannel, const string& inCommand)
	: MWindow("Terminal", GetPrefferedBounds(),
		MWindowFlags(kMPostionDefault | kMNoEraseOnUpdate /* | kMCustomNonClient */),
		"terminal-window-menu")
	, eAnimate(this, &MTerminalWindow::Animate)
	, mChannel(inTerminalChannel)
	, mNext(nullptr)
	, mAnimationManager(new MAnimationManager())
	, mAnimationVariable(mAnimationManager->CreateVariable(0, 0, kSearchPanelHeight))
{
	// animations
	AddRoute(eAnimate, mAnimationManager->eAnimate);

	// create views
	MRect bounds;

	// create search panel
	GetBounds(bounds);
	bounds.height = kSearchPanelHeight;
	mSearchPanel = new MSearchPanel("searchpanel", bounds);
	mSearchPanel->SetBindings(true, true, true, false);
	mSearchPanel->SetLayout(ePackStart, false, false, 0);
	AddChild(mSearchPanel);
	
	// create status bar
	GetBounds(bounds);
	bounds.y += bounds.height - kScrollbarWidth;
	bounds.height = kScrollbarWidth;

	MStatusBarElement parts[] = {
		{ 250, ePackStart, 4, false, false },
		{ 275, ePackStart, 4, true, true },
		{ 60, ePackEnd, 4, false, false }
	};

	mStatusbar = new MStatusbar("status", bounds, 3, parts);
	AddChild(mStatusbar);
	mStatusbar->GetBounds(bounds);

	int32_t statusbarHeight = bounds.height;

	// hbox: force correct autolayout in Gtk
	MBoxControl* hbox = new MBoxControl("hbox", bounds, true, false, true, true, 0, 0);
	hbox->SetLayout(ePackStart, true, true, 0);
	AddChild(hbox);

	// create scrollbar
	GetBounds(bounds);
	bounds.height -= statusbarHeight + kSearchPanelHeight;
	bounds.y += kSearchPanelHeight;
	bounds.x += bounds.width - kScrollbarWidth;
	bounds.width = kScrollbarWidth;
	mScrollbar = new MScrollbar("scrollbar", bounds);
	mScrollbar->SetBindings(false, true, true, true);
	mScrollbar->SetLayout(ePackEnd, false, false, 0);
	hbox->AddChild(mScrollbar);
	
	// create terminal view
//	GetBounds(bounds);
//	bounds.y += kSearchPanelHeight;
//	bounds.height -= statusbarHeight + kSearchPanelHeight;
//	bounds.width -= kScrollbarWidth;

	uint32_t w, h;
	MTerminalView::GetTerminalMetrics(80, 24, false, w, h);
	bounds = MRect(0, 0, w, h);

	mTerminalView = new MTerminalView("terminalview", bounds, mStatusbar, mScrollbar, mSearchPanel, inTerminalChannel, inCommand);
	mTerminalView->SetBindings(true, true, true, true);
	mTerminalView->SetLayout(ePackStart, true, true, 0);
	hbox->AddChild(mTerminalView);
	AddRoute(mTerminalView->eScroll, mScrollbar->eScroll);
	
//	SetFocus(mTerminalView);
	SetLatentFocus(mTerminalView);
	
	// and now resize the search panel...
//	Animate();
	mSearchPanel->Hide();

	if (Preferences::GetBoolean("show-status-bar", true) == false)
		mStatusbar->Hide();

	// add to bottom of the list
	if (sFirst == nullptr)
		sFirst = this;
	else
	{
		MTerminalWindow* w = sFirst;
		while (w->mNext != nullptr)
			w = w->mNext;
		w->mNext = this;
	}
}

MTerminalWindow::~MTerminalWindow()
{
	// remove from the list
	if (sFirst == this)
		sFirst = mNext;
	else
	{
		MTerminalWindow* w = sFirst;
		while (w->mNext != this)
			w = w->mNext;
		assert(w != nullptr);
		if (w != nullptr)
			w->mNext = mNext;
	}
	
	delete mAnimationVariable;
	delete mAnimationManager;
}

void MTerminalWindow::Mapped()
{
//	mTerminalView->ResizeTerminal(80, 24);
	mTerminalView->Open();
}

MRect MTerminalWindow::GetPrefferedBounds()
{
	uint32_t w, h;
	MTerminalView::GetTerminalMetrics(80, 24, false, w, h);

	return MRect(0, 0, w, h + kScrollbarWidth);
}

void MTerminalWindow::ShowSearchPanel()
{
	mSearchPanel->Show();

//	(void)mAnimationManager->Update();
//
//	MStoryboard* storyboard = mAnimationManager->CreateStoryboard();
//	storyboard->AddTransition(mAnimationVariable, kSearchPanelHeight, 0.25,
//		"acceleration-decelleration");
//	mAnimationManager->Schedule(storyboard);
}

void MTerminalWindow::HideSearchPanel()
{
	mSearchPanel->Hide();

//	MStoryboard* storyboard = mAnimationManager->CreateStoryboard();
//	storyboard->AddTransition(mAnimationVariable, 0, 0.25,
//		"acceleration-decelleration");
//	mAnimationManager->Schedule(storyboard);
	
	mTerminalView->SetFocus();
}

void MTerminalWindow::Animate()
{
//	ResizeSearchPanelTo(static_cast<uint32_t>(mAnimationVariable->GetValue()));

	MRect frame;
	mSearchPanel->GetFrame(frame);
	
	uint32_t newHeight = static_cast<uint32_t>(mAnimationVariable->GetValue());

	int32_t delta = newHeight - frame.height;
	
	mSearchPanel->ResizeFrame(0, delta);
	mTerminalView->ResizeFrame(0, -delta);
	mTerminalView->MoveFrame(0, delta);
	mScrollbar->ResizeFrame(0, -delta);
	mScrollbar->MoveFrame(0, delta);

	if (newHeight >= kSearchPanelHeight)
		mSearchPanel->GetTextBox()->SetFocus();
	
	ResizeWindow(0, delta);
}

bool MTerminalWindow::IsAnyTerminalOpen()
{
	bool result = false;
	MTerminalWindow* w = GetFirstTerminal();
	
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
		result = DisplayAlert(this, "close-session-alert") == 1;
	}
	
	return result;
}

void MTerminalWindow::Close()
{
#warning "fix"
	// mAnimationManager->Stop();
	mTerminalView->Destroy();

	mChannel->GetIOService().post([this]()
	{
		this->MWindow::Close();
	});
}

bool MTerminalWindow::UpdateCommandStatus(
	uint32_t			inCommand,
	MMenu*			inMenu,
	uint32_t			inItemIndex,
	bool&			outEnabled,
	bool&			outChecked)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_Disconnect:
			outEnabled = mChannel != nullptr && mChannel->CanDisconnect();
			break;
		
		case cmd_NextTerminal:
		case cmd_PrevTerminal:
			outEnabled = sFirst != this or mNext != nullptr;
			break;

		case cmd_Find:
		case cmd_CloneTerminal:
			outEnabled = true;
			break;
		
		default:
			result = MWindow::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
			break;
	}
	
	return result;
}

bool MTerminalWindow::ProcessCommand(
	uint32_t			inCommand,
	const MMenu*	inMenu,
	uint32_t			inItemIndex,
	uint32_t			inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_Disconnect:
			mChannel->Disconnect(inModifiers & kControlKey);
			break;
		
		case cmd_CloneTerminal:
		{
			MTerminalWindow* clone = Clone();
			clone->Select();
			break;
		}

		//case cmd_Explore:
		//{
		//	MExploreBrowserWindow* explorer = new MExploreBrowserWindow(mConnection);
		//	explorer->Select();
		//	break;
		//}

		case cmd_NextTerminal:
			if (mNext != nullptr)
				mNext->Select();
			else if (sFirst != this)
				sFirst->Select();
			break;

		case cmd_PrevTerminal:
			if (sFirst == this)
			{
				MTerminalWindow* w = sFirst;
				while (w->mNext != nullptr)
					w = w->mNext;
				if (w != this)
					w->Select();
			}
			else
			{
				MTerminalWindow* w = sFirst;
				while (w != nullptr and w->mNext != this)
					w = w->mNext;
				if (w != nullptr and w != this)
					w->Select();
			}
			break;

		case cmd_Find:
			if (mSearchPanel->IsVisible())
				HideSearchPanel();
			else
				ShowSearchPanel();
			break;
		
		case cmd_HideSearchPanel:
			HideSearchPanel();
			break;
		
		default:
			result = MWindow::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}
	
	return result;
}

// ------------------------------------------------------------------
//

MTerminalWindow* MTerminalWindow::Create(boost::asio::io_service& inIOService)
{
	return new MPtyTerminalWindow(inIOService);
}

MTerminalWindow* MTerminalWindow::Create(const string& inUser, const string& inHost, uint16_t inPort,
	const string& inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection)
{
	return new MSshTerminalWindow(inUser, inHost, inPort, inSSHCommand, inConnection);
}						
