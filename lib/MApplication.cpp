//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// $Id$

#include "MLib.hpp"

#include <iostream>
#include <filesystem>

#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options.hpp>

#include "MApplication.hpp"
#include "MWindow.hpp"
#include "MApplicationImpl.hpp"
#include "MCommands.hpp"
#include "MPreferences.hpp"
#include "MUtils.hpp"
#include "MMenu.hpp"
#include "MStrings.hpp"
#include "MError.hpp"
#include "MControls.hpp"

using namespace std;
namespace ba = boost::algorithm;
namespace po = boost::program_options;
namespace fs = std::filesystem;

#if DEBUG
int VERBOSE, TRACE;
#endif

MApplication* gApp;
fs::path gExecutablePath, gPrefixPath;

// --------------------------------------------------------------------

MApplication::MApplication(
	MApplicationImpl*		inImpl)
	: MHandler(nullptr)
	, mImpl(inImpl)
	, mQuit(false)
	, mQuitPending(false)
{
	// set the global pointing to us
	gApp = this;
}

MApplication::~MApplication()
{
	delete mImpl;
}

void MApplication::Initialise()
{
	mImpl->Initialise();
}

void MApplication::SaveGlobals()
{
	Preferences::SaveIfDirty();
}

void MApplication::DoNew()
{
}

void MApplication::DoOpen()
{
}

void MApplication::Open(const string& inURI)
{
}

bool MApplication::ProcessCommand(
	uint32_t			inCommand,
	const MMenu*	inMenu,
	uint32_t			inItemIndex,
	uint32_t			inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_New:
			DoNew();
			break;

		case cmd_Open:
			DoOpen();
			break;
		
		case cmd_SelectWindowFromMenu:
			DoSelectWindowFromWindowMenu(inItemIndex - 2);
			break;
		
		case cmd_Quit:
			if (AllowQuit(false))
				DoQuit();
			break;

		default:
			result = false;
			break;
	}
	
	return result;
}

bool MApplication::UpdateCommandStatus(
	uint32_t			inCommand,
	MMenu*			inMenu,
	uint32_t			inItemIndex,
	bool&			outEnabled,
	bool&			outChecked)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_New:
		case cmd_Open:
		case cmd_SelectWindowFromMenu:
		case cmd_Quit:
			outEnabled = true;
			break;
		
		default:
			result = false;
			break;
	}
	
	return result;
}

void MApplication::UpdateSpecialMenu(const string& inName, MMenu* inMenu)
{
	if (inName == "window")
		UpdateWindowMenu(inMenu);
	else
		PRINT(("Unknown special menu %s", inName.c_str()));
}

void MApplication::UpdateWindowMenu(MMenu* inMenu)
{
}

void MApplication::DoSelectWindowFromWindowMenu(uint32_t inIndex)
{
}	

int MApplication::RunEventLoop()
{
	return mImpl->RunEventLoop();
}

bool MApplication::AllowQuit(bool inLogOff)
{
	bool result = mQuitPending;
	
	if (result == false)
	{
		result = true;

		MWindow* window = MWindow::GetFirstWindow();
		while (window != nullptr)
		{
			if (not window->AllowClose(true))
			{
				result = false;
				break;
			}
			
			window = window->GetNextWindow();
		}
		
		mQuitPending = result;
	}
	
	return result;
}

void MApplication::DoQuit()
{
	mQuit = true;
	mQuitPending = true;

	SaveGlobals();

	mImpl->Quit();
}

void MApplication::Pulse()
{
	// if there are no visible windows left, we quit
	MWindow* front = MWindow::GetFirstWindow();
	while (front != nullptr and not front->IsVisible())
		front = front->GetNextWindow();
	
	if (front == nullptr)
		DoQuit();
	else
		eIdle(GetLocalTime());

	Preferences::SaveIfDirty();
}

