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
	virtual bool		UpdateCommandStatus(uint32_t inCommand, MMenu* inMenu, uint32_t inItemIndex,
							bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32_t inCommand, const MMenu* inMenu, uint32_t inItemIndex,
							uint32_t inModifiers);
	virtual void		UpdateSpecialMenu(const std::string& inMenuKind, MMenu* inMenu);
	virtual void		UpdateWindowMenu(MMenu* inMenu);
	virtual void		UpdateRecentMenu(MMenu* inMenu);

	void				AddToRecentMenu(std::string inURL);
	const std::string&	GetRecent(uint32_t inIndex);

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
	virtual void		DoSelectWindowFromWindowMenu(uint32_t inIndex);

	virtual void		SaveGlobals();

	std::string			mCurrentFolder;
	std::deque<std::string>
						mRecentFiles;
};

extern MDocApplication* gDocApp;
