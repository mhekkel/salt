// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "MAcceleratorTable.hpp"
#include "MMenu.hpp"
#include "MLibTestApp.hpp"
#include "MPreferences.hpp"
#include "MUtils.hpp"
#include "MAlerts.hpp"
#include "MWindow.hpp"
#include "MControls.hpp"

using namespace std;
namespace ba = boost::algorithm;

const char
	kAppName[] = "lib-test", kRevision[] = "0.1";

// --------------------------------------------------------------------

class MLibTestWindow : public MWindow
{
  public:
	MLibTestWindow();

	MStatusbar* mStatusbar;
};

MLibTestWindow::MLibTestWindow()
	: MWindow("Nieuw venster", MRect(0, 0, 400, 400), MWindowFlags(0), "terminal-window-menu")
{
	int32 partWidths[5] = { 250, 250, 45, -1 };
	mStatusbar = new MStatusbar("status", MRect(0, 0, 16, 400), 4, partWidths);
	AddChild(mStatusbar);
}

// --------------------------------------------------------------------

MLibTestApp::MLibTestApp(MApplicationImpl* inImpl)
	: MApplication(inImpl)
{
	MAcceleratorTable& at = MAcceleratorTable::Instance();

	at.RegisterAcceleratorKey(cmd_New,				'N', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Close,			'W', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Quit,				'Q', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Cut,				'X', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Copy,				'C', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Paste,			'V', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_SelectAll,		'A', kControlKey | kShiftKey);
//	at.RegisterAcceleratorKey(cmd_CloneTerminal,	'D', kControlKey | kShiftKey);
//	at.RegisterAcceleratorKey(cmd_Reset,			'R', kControlKey | kShiftKey);
//	at.RegisterAcceleratorKey(cmd_NextTerminal,		kTabKeyCode, kControlKey);
//	at.RegisterAcceleratorKey(cmd_PrevTerminal,		kTabKeyCode, kControlKey | kShiftKey);
//	
//	at.RegisterAcceleratorKey(cmd_Find,				'F', kControlKey | kShiftKey);
//	at.RegisterAcceleratorKey(cmd_FindNext,			kF3KeyCode, kControlKey);
//	at.RegisterAcceleratorKey(cmd_FindPrev,			kF3KeyCode, kControlKey | kShiftKey);
}

MLibTestApp::~MLibTestApp()
{
}

void MLibTestApp::Initialise()
{
	MApplication::Initialise();
	
	ProcessCommand(cmd_New, nullptr, 0, 0);
}

void MLibTestApp::SaveGlobals()
{
	MApplication::SaveGlobals();
}

MApplication* MApplication::Create(MApplicationImpl* inImpl)
{
	return new MLibTestApp(inImpl);
}

// --------------------------------------------------------------------

bool MLibTestApp::ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex, uint32 inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_About:
			DoAbout();
			break;
		
//		case cmd_OpenRecentSession:
//			if (inItemIndex - 2 < mRecent.size())
//				OpenRecent(*(mRecent.begin() + inItemIndex - 2));
//			break;
//		
//		case cmd_ClearRecentSessions:
//			mRecent.clear();
//			break;
//
//		case cmd_SelectWindowFromMenu:
//		{
//			MTerminalWindow* term = MTerminalWindow::GetFirstTerminal();
//			while (inItemIndex-- > 3 and term != nullptr)
//				term = term->GetNextTerminal();
//			if (term != nullptr)
//				term->Select();
//			break;
//		}
//		
//		case cmd_Preferences:
//			MPreferencesDialog::Instance().Select();
//			break;
//
////#if DEBUG
////		case 'test':
////			new MTestWindow();
////			break;
////#endif
		
		default:
			result = MApplication::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}
	
	return result;
}

bool MLibTestApp::UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex, bool& outEnabled, bool& outChecked)
{
	bool result = true;

	switch (inCommand)
	{
//		case cmd_Preferences:
		case cmd_About:
//		case cmd_Find:
		case 'test':
			outEnabled = true;
			break;

		default:
			result = MApplication::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
			break;
	}
	
	return result;
}

void MLibTestApp::UpdateSpecialMenu(const std::string& inName, MMenu* inMenu)
{
	if (inName == "window")
		UpdateWindowMenu(inMenu);
//	else if (inName == "recent-session")
//		UpdateRecentSessionMenu(inMenu);
//	else if (inName == "public-keys")
//		UpdatePublicKeyMenu(inMenu);
	else
		MApplication::UpdateSpecialMenu(inName, inMenu);
}

void MLibTestApp::DoAbout()
{
	DisplayAlert(nullptr, "about-alert", "x", kRevision);
}

void MLibTestApp::DoQuit()
{
	mQuit = true;
	mQuitPending = true;

//	mConnectionPool.disconnect_all();
//
//	// closing windows happens asynchronously
//	list<MWindow*> windows;
//	MWindow* w = MWindow::GetFirstWindow();
//	while (w != nullptr)
//	{
//		windows.push_back(w);
//		w = w->GetNextWindow();
//	}
//
//	for_each(windows.begin(), windows.end(), [](MWindow* w) { w->Close(); });
//
//	// poll the io_service until all windows are closed
//	mIOService.reset();
//		
//	for (;;)
//	{
//		size_t n = mIOService.poll_one();
//		if (n == 0 or MWindow::GetFirstWindow() == nullptr)
//			break;
//	}

	MApplication::DoQuit();
}

void MLibTestApp::DoNew()
{
//	MDialog* d = new MConnectDialog();
//	d->Select();
	MWindow* w = new MLibTestWindow();
	w->Select();
}

