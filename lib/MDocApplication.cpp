//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// $Id$

#include "MLib.hpp"

#include <iostream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options.hpp>

#include "MDocApplication.hpp"
#include "MApplicationImpl.hpp"
#include "MCommands.hpp"
#include "MDocument.hpp"
#include "MPreferences.hpp"
#include "MDocWindow.hpp"
#include "MUtils.hpp"
#include "MMenu.hpp"
#include "MStrings.hpp"
#include "MError.hpp"

#include "MControls.hpp"

using namespace std;
namespace ba = boost::algorithm;
namespace po = boost::program_options;
namespace fs = std::filesystem;

// --------------------------------------------------------------------

MDocApplication* gDocApp;

MDocApplication::MDocApplication(MApplicationImpl* inImpl)
	: MApplication(inImpl)
{
	gDocApp = this;
}

MDocApplication::~MDocApplication()
{
	gDocApp = nullptr;
}

void MDocApplication::Initialise()
{
	MApplication::Initialise();

	mRecentFiles.clear();

	vector<string> recent;
	Preferences::GetArray("recent", recent);
	reverse(recent.begin(), recent.end());

	for (string path: recent)
	{
		MFile file(path);
		if (not file.IsLocal() or file.Exists())
			AddToRecentMenu(path);
	}
}

void MDocApplication::SaveGlobals()
{
	vector<string> recent;
	copy(mRecentFiles.begin(), mRecentFiles.end(), back_inserter(recent));
	Preferences::SetArray("recent", recent);
	
	MApplication::SaveGlobals();
}

bool MDocApplication::ProcessCommand(uint32_t inCommand, const MMenu*	inMenu,
	uint32_t inItemIndex, uint32_t inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		//case cmd_About:
		//{
		//	MWindow* w = MWindow::GetFirstWindow();
		//	GtkWidget* ww = nullptr;
		//	if (w != nullptr)
		//		ww = w->GetGtkWidget();
		//	
		//	gtk_show_about_dialog(GTK_WINDOW(ww),
		//		"program_name", kAppName,
		//		"version", kVersionString,
		//		"copyright", "Copyright © 2007-2009 Maarten L. Hekkelman",
		//		"comments", _("A simple development environment"),
		//		"website", "http://www.hekkelman.com/",
		//		nullptr);
		//	break;
		//}
		
		//case cmd_PageSetup:
		//	MPrinter::DoPageSetup();
		//	break;
		
		//case cmd_Preferences:
		//	MPrefsDialog::Create();
		//	break;
		
		case cmd_CloseAll:
			CloseAll(kSaveChangesClosingAllDocuments);
			break;
		
		case cmd_SaveAll:
			DoSaveAll();
			break;
		
		case cmd_SelectWindowFromMenu:
			DoSelectWindowFromWindowMenu(inItemIndex - 2);
			break;
		
		case cmd_OpenRecent:
			if (inMenu != nullptr and inItemIndex < mRecentFiles.size())
				Open(mRecentFiles[inItemIndex]);
			break;
		
		default:
			result = MApplication::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}
	
	return result;
}

bool MDocApplication::UpdateCommandStatus(uint32_t inCommand, MMenu* inMenu, uint32_t inItemIndex,
	bool& outEnabled, bool& outChecked)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_New:
		case cmd_Open:
		case cmd_CloseAll:
		case cmd_SaveAll:
		case cmd_PageSetup:
		case cmd_Quit:
		case cmd_Find:
		case cmd_About:
		case cmd_SelectWindowFromMenu:
			outEnabled = true;
			break;

		case cmd_OpenRecent:
		{
			MFile file(mRecentFiles[inItemIndex]);
			outEnabled = file.IsLocal() == false or file.Exists();
			break;
		}
		
		default:
			result = MApplication::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
			break;
	}
	
	return result;
}

void MDocApplication::UpdateSpecialMenu(const string& inName, MMenu* inMenu)
{
	if (inName == "window")
		UpdateWindowMenu(inMenu);
	else if (inName == "recent")
		UpdateRecentMenu(inMenu);
	else
		PRINT(("Unknown special menu %s", inName.c_str()));
}

void MDocApplication::UpdateWindowMenu(MMenu* inMenu)
{
	inMenu->RemoveItems(2, inMenu->CountItems() - 2);
	
	MDocument* doc = MDocument::GetFirstDocument();
	while (doc != nullptr)
	{
		string label;

		MDocWindow* w = MDocWindow::FindWindowForDocument(doc);
		if (w != nullptr)
			label = w->GetTitle();
		else
			label = _("weird");
			
		inMenu->AppendItem(label, cmd_SelectWindowFromMenu);
		
		doc = doc->GetNextDocument();
	}	
}

void MDocApplication::UpdateRecentMenu(MMenu* inMenu)
{
	inMenu->RemoveItems(0, inMenu->CountItems());

//	mRecentFiles.erase(
//		remove_if(mRecentFiles.begin(), mRecentFiles.end(),
//			std::bind(&MFile::IsLocal, _1) == true and std::bind(&MFile::Exists, _1) == false),
//		mRecentFiles.end());

	for (string& url: mRecentFiles)
	{
		MFile file(url);
		
		if (file.IsLocal())
		{
			inMenu->AppendItem(file.GetPath().string(), cmd_OpenRecent);
//			if (not file.Exists())
//				inMenu->
		}
		else
			inMenu->AppendItem(file.GetURL(), cmd_OpenRecent);
	}
}

void MDocApplication::AddToRecentMenu(string inURL)
{
	if (mRecentFiles.empty() or not (mRecentFiles.front() == inURL))
	{
		MFile url(inURL);

		mRecentFiles.erase(
			remove_if(mRecentFiles.begin(), mRecentFiles.end(),
				[&url](const string& r) { return url == r; }),
			mRecentFiles.end());

		mRecentFiles.push_front(inURL);

		if (mRecentFiles.size() > static_cast<uint32_t>(Preferences::GetInteger("recent-size", 20)))
			mRecentFiles.pop_back();
	}
}

const string& MDocApplication::GetRecent(uint32_t inIndex)
{
	if (inIndex >= mRecentFiles.size())
		THROW(("Recent index out of range"));
	return mRecentFiles[inIndex];
}

void MDocApplication::DoSelectWindowFromWindowMenu(uint32_t inIndex)
{
	MDocument* doc = MDocument::GetFirstDocument();

	while (doc != nullptr and inIndex-- > 0)
		doc = doc->GetNextDocument();
	
	if (doc != nullptr)
		DisplayDocument(doc);	
}	

void MDocApplication::DoSaveAll()
{
	MDocument* doc = MDocument::GetFirstDocument();
	
	while (doc != nullptr)
	{
		if (doc->IsSpecified() and doc->IsModified())
			doc->DoSave();
		doc = doc->GetNextDocument();
	}
	
	doc = MDocument::GetFirstDocument();

	while (doc != nullptr)
	{
		if (not doc->IsSpecified() and doc->IsModified())
		{
			MController* controller = doc->GetFirstController();

			assert(controller != nullptr);
			
			controller->SaveDocumentAs();
		}
		
		doc = doc->GetNextDocument();
	}
}

bool MDocApplication::CloseAll(MCloseReason inAction)
{
	bool result = true;
	// first close all that can be closed

	MDocument* doc = MDocument::GetFirstDocument();
	
	while (doc != nullptr)
	{
		MDocument* next = doc->GetNextDocument();
		
		if (not doc->IsModified() and IsCloseAllCandidate(doc))
		{
			MController* controller = doc->GetFirstController();
			if (controller != nullptr)
			{
				MWindow* w = controller->GetWindow();
				if (controller->TryCloseDocument(inAction))
					w->Close();
			}
			else
				cerr << _("Weird, document without controller: ") << doc->GetFile().GetURL() << endl;
		}
		
		doc = next;
	}
	
	// then close what remains
	doc = MDocument::GetFirstDocument();

	while (doc != nullptr)
	{
		MDocument* next = doc->GetNextDocument();

		MController* controller = doc->GetFirstController();
		assert(controller != nullptr);

		if (IsCloseAllCandidate(doc) or
			inAction == kSaveChangesQuittingApplication)
		{
			if (not controller->TryCloseDocument(inAction))
			{
				result = false;
				break;
			}
		}
		
		doc = next;
	}
	
	return result;
}

bool MDocApplication::AllowQuit(bool inLogOff)
{
	vector<string> unsaved;
	bool result = true;

	MDocument* doc = MDocument::GetFirstDocument();
	while (doc != nullptr)
	{
		MDocument* next = doc->GetNextDocument();

		MController* controller = doc->GetFirstController();
		assert(controller != nullptr);
		
		if (not controller->TryCloseDocument(kSaveChangesQuittingApplication))
		{
			result = false;
			break;
		}

//		if (IsCloseAllCandidate(doc) or
//			inAction == kSaveChangesQuittingApplication)
//		{
//			if (not controller->AllowClose(true))
//			{
//				result = false;
//				break;
//			}
//			
//			
//			
//			if (not controller->TryCloseDocument(inAction))
//			{
//				result = false;
//				break;
//			}
//		}
		
		doc = next;
	}

	return result;
}

void MDocApplication::DoQuit()
{
	if (CloseAll(kSaveChangesQuittingApplication))
		MApplication::DoQuit();
}

void MDocApplication::DoOpen()
{
	vector<fs::path> files;
	
	if (MFileDialogs::ChooseFiles(nullptr, false, files))
	{
		for (vector<fs::path>::iterator file = files.begin(); file != files.end(); ++file)
			Open(file->string());
	}
}

//// ---------------------------------------------------------------------------
//
//MDocument* MDocApplication::OpenDocument(const string& inURL)
//{
//	MFile file(inURL);
//	if (file.IsLocal() and fs::is_directory(file.GetPath()))
//		THROW(("Cannot open a directory"));
//	
//	MDocument* doc = MDocument::GetDocumentForFile(file.GetURL());
//	
//	if (doc != nullptr)
//		AddToRecentMenu(file.GetURL());
//	
//	return doc;
//}
