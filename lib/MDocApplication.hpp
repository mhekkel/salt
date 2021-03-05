//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MApplication.hpp"
#include "MController.hpp"

// ===========================================================================

class MDocApplication : public MApplication
{
  public:
						~MDocApplication();

	virtual void		Initialise();
	virtual bool		UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex,
							bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex,
							uint32 inModifiers);
	virtual void		UpdateSpecialMenu(const std::string& inMenuKind, MMenu* inMenu);
	virtual void		UpdateWindowMenu(MMenu* inMenu);
	virtual void		UpdateRecentMenu(MMenu* inMenu);

	void				AddToRecentMenu(std::string inURL);
	const std::string&	GetRecent(uint32 inIndex);

	virtual MWindow*	DisplayDocument(MDocument* inDocument) = 0;
	virtual bool		CloseAll(MCloseReason inReason);

	const std::string&	GetCurrentFolder() const							{ return mCurrentFolder; }
	virtual void		SetCurrentFolder(const std::string& inFolder)		{ mCurrentFolder = inFolder; }

  protected:

						MDocApplication(MApplicationImpl* inImpl);
	virtual bool		IsCloseAllCandidate(MDocument* inDocument)			{ return true; }

	virtual void		DoNew() = 0;
	virtual void		DoOpen();
	virtual void		DoQuit();
	virtual bool		AllowQuit(bool inLogOff);
	virtual void		DoSaveAll();
	virtual void		DoSelectWindowFromWindowMenu(uint32 inIndex);

	virtual void		SaveGlobals();

	std::string			mCurrentFolder;
	std::deque<std::string>
						mRecentFiles;
};

extern MDocApplication* gDocApp;
