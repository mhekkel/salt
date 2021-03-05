//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <list>
#include <vector>
#include <deque>

#include <boost/filesystem/path.hpp>

#include "MTypes.h"
#include "MHandler.h"
#include "MP2PEvents.h"

extern const char kAppName[], kVersionString[];

extern boost::filesystem::path gExecutablePath;

class MWindow;
class MApplicationImpl;

// ===========================================================================

class MApplication : public MHandler
{
  public:
	
	static MApplication*
						Create(MApplicationImpl* inImpl);
	static void			Install(const std::string& inPrefix);

						~MApplication();
	virtual void		Initialise();

	virtual void		DoNew();
	virtual void		DoOpen();
	virtual void		Open(const std::string& inURL);

	virtual bool		UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex,
							bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex,
							uint32 inModifiers);

	virtual void		UpdateSpecialMenu(const std::string& inMenuKind, MMenu* inMenu);
	virtual void		UpdateWindowMenu(MMenu* inMenu);

	MEventOut<void(double)>						eIdle;

	int					RunEventLoop();
	virtual void		Pulse();

	virtual bool		AllowQuit(bool inLogOff);
	virtual void		DoQuit();

	bool				IsQuitting() const						{ return mQuitPending; }
	void				CancelQuit()							{ mQuitPending = false; }

  protected:

						MApplication(MApplicationImpl* inImpl);

	typedef std::list<MWindow*>		MWindowList;

	virtual void		DoSelectWindowFromWindowMenu(uint32 inIndex);

	virtual void		SaveGlobals();

	MApplicationImpl*	mImpl;

	bool				mQuit;
	bool				mQuitPending;
};

extern MApplication*	gApp;
