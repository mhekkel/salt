//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <cstring>
#include <boost/algorithm/string.hpp>

#include "MDocument.hpp"
#include "MController.hpp"
#include "MUtils.hpp"
#include "MMenu.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MDocClosedNotifier.hpp"
#include "MDocApplication.hpp"
#include "MAlerts.hpp"

using namespace std;
namespace ba = boost::algorithm;

MDocument* MDocument::sFirst;

// ---------------------------------------------------------------------------
//	MDocument

MDocument::MDocument(MHandler* inSuper)
	: MHandler(inSuper)
	, mWarnedReadOnly(false)
	, mDirty(false)
	, mNext(nullptr)
{
	mNext = sFirst;
	sFirst = this;
}

MDocument::MDocument(MHandler* inSuper, const std::string& inURL)
	: MHandler(inSuper)
	, mFile(inURL)
	, mWarnedReadOnly(false)
	, mDirty(false)
	, mNext(nullptr)
{
	mNext = sFirst;
	sFirst = this;
}

// ---------------------------------------------------------------------------
//	~MDocument

MDocument::~MDocument()
{
	if (mFile.IsIOActive())
		mFile.CancelIO();
	
	if (sFirst == this)
		sFirst = mNext;
	else
	{
		MDocument* doc = sFirst;
		while (doc != nullptr)
		{
			MDocument* next = doc->mNext;
			if (next == this)
			{
				doc->mNext = mNext;
				break;
			}
			doc = next;
		}
	}
	
	eDocumentClosed(this);
}

// ---------------------------------------------------------------------------
//	SetFile

void MDocument::SetFile(const string& inURL)
{
	mFile.SetURL(inURL);
	eFileSpecChanged(this);
}

// ---------------------------------------------------------------------------
//	MDocument::DoLoad

void MDocument::DoLoad()
{
	mFile.Load(this);
}

// ---------------------------------------------------------------------------
//	DoSave

bool MDocument::DoSave()
{
	mFile.Save(this);

	static_cast<MDocApplication*>(gApp)->AddToRecentMenu(mFile.GetURL());
		
	return true;
}

// ---------------------------------------------------------------------------
//	DoSaveAs

bool MDocument::DoSaveAs(const fs::path& inPath)
{
	string url = "file://";
	url += inPath.string();
	return DoSaveAs(url);
}

bool MDocument::DoSaveAs(const string& inURL)
{
	bool result = false;
	
	string savedURL = mFile.GetURL();
	mFile.SetURL(inURL);

	if (DoSave())
	{
		eFileSpecChanged(this);
		mNotifiers.clear();
		result = true;
	}
	else
		mFile.SetURL(savedURL);
	
	return result;
}

// ---------------------------------------------------------------------------
//	RevertDocument

void MDocument::RevertDocument()
{
	if (IsSpecified())
		DoLoad();
}

// ---------------------------------------------------------------------------
//	UsesFile

bool MDocument::UsesFile(const string& inURL) const
{
	return mFile.IsValid() and mFile == MFile(inURL);
}

MDocument* MDocument::GetDocumentForFile(const string& inURL)
{
	MDocument* doc = sFirst;

	while (doc != nullptr and not doc->UsesFile(inURL))
		doc = doc->mNext;
	
	return doc;
}

// ---------------------------------------------------------------------------
//	AddNotifier

void MDocument::AddNotifier(
	MDocClosedNotifier&		inNotifier,
	bool					inRead)
{
	mNotifiers.push_back(inNotifier);
}

// ---------------------------------------------------------------------------
//	AddController

void MDocument::AddController(MController* inController)
{
	if (find(mControllers.begin(), mControllers.end(), inController) == mControllers.end())
		mControllers.push_back(inController);
}

// ---------------------------------------------------------------------------
//	RemoveController

void MDocument::RemoveController(MController* inController)
{
	assert(find(mControllers.begin(), mControllers.end(), inController) != mControllers.end());
	
	mControllers.erase(
		remove(mControllers.begin(), mControllers.end(), inController),
		mControllers.end());

	if (mControllers.size() == 0)
		CloseDocument();
}

// ---------------------------------------------------------------------------
//	GetFirstController

MController* MDocument::GetFirstController() const
{
	MController* controller = nullptr;
	if (mControllers.size() > 0)
		controller = mControllers.front();
	return controller;
}

// ---------------------------------------------------------------------------
//	GetWindow

MWindow* MDocument::GetWindow() const
{
	MWindow* result = nullptr;
	MController* controller = GetFirstController();
	
	if (controller != nullptr)
		result = controller->GetWindow();
	
	return result;
}

// ---------------------------------------------------------------------------
//	MakeFirstDocument

void MDocument::MakeFirstDocument()
{
	MDocument* d = sFirst;
	
	if (d != this)
	{
		while (d != nullptr and d->mNext != this)
			d = d->mNext;
		
		assert(d->mNext == this);
		d->mNext = mNext;
		mNext = sFirst;
		sFirst = this;
	}
}

// ---------------------------------------------------------------------------
//	SetModified

void MDocument::SetModified(bool inModified)
{
	if (inModified != mDirty)
	{
		mDirty = inModified;
		eModifiedChanged(mDirty);
	}
}

// ---------------------------------------------------------------------------
//	CheckIfModifiedOnDisk

void MDocument::CheckIfModifiedOnDisk(MWindow* inDocWindow)
{
	if (mFile.IsModifiedOnDisk() and DisplayAlert(inDocWindow, "reload-modified-file", { mFile.GetPath().string() }) == 2)
		RevertDocument();
	//else
	//	mFile.ResetFileInfo();
}

// ---------------------------------------------------------------------------
//	CloseDocument

void MDocument::CloseDocument()
{
	try
	{
		eDocumentClosed(this);
	}
	catch (...) {}
	
	delete this;
}

// ---------------------------------------------------------------------------
//	UpdateCommandStatus

bool MDocument::UpdateCommandStatus(
	uint32_t			inCommand,
	MMenu*			inMenu,
	uint32_t			inItemIndex,
	bool&			outEnabled,
	bool&			outChecked)
{
	return false;
}

// ---------------------------------------------------------------------------
//	ProcessCommand

bool MDocument::ProcessCommand(
	uint32_t			inCommand,
	const MMenu*	inMenu,
	uint32_t			inItemIndex,
	uint32_t			inModifiers)
{
	return false;
}

// ---------------------------------------------------------------------------
//	HandleKeyDown

bool MDocument::HandleKeyDown(
	uint32_t			inKeyCode,
	uint32_t			inModifiers,
	bool			inRepeat)
{
	return false;
}

// ---------------------------------------------------------------------------
//	HandleCharacter

bool MDocument::HandleCharacter(
	const string&	inText,
	bool			inRepeat)
{
	return false;
}

// ---------------------------------------------------------------------------
//	GetWindowTitle

string MDocument::GetWindowTitle() const
{
	string result;

	if (mFile.IsLocal())
	{
		fs::path file = fs::canonical(mFile.GetPath());
		
		NormalizePath(file);
		result = file.string();
		
		// strip off HOME, if any
		string home = GetHomeDirectory();
		if (not home.empty() and ba::istarts_with(result, home))
		{
			result.erase(0, home.length());
			result.insert(0, "~");
		}
	}
	else
		result = mFile.GetURL();
	
	return result;
}

// ---------------------------------------------------------------------------
//	GetDocumentName

string MDocument::GetDocumentName() const
{
	return mFile.GetPath().filename().string();
}

// ---------------------------------------------------------------------------
//	IOProgress

void MDocument::IOProgress(float inProgress, const string& inMessage)
{
	if (inProgress == 1.0f)
		SetModified(false);
}

// ---------------------------------------------------------------------------
//	IOError

void MDocument::IOError(const std::string& inError)
{
	DisplayError(inError);
}

// ---------------------------------------------------------------------------
//	IOAskOverwriteNewer

bool MDocument::IOAskOverwriteNewer()
{
	fs::path path = mFile.GetPath();
	return DisplayAlert(nullptr, "ask-overwrite-newer", { path.filename().string() }) == 2;
}

