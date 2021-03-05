// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MTypes.h"
#include "MApplication.h"

extern const char kAppName[], kVersionString[];

class MWindow;

// ===========================================================================

class MLibTestApp : public MApplication
{
  public:

						MLibTestApp(MApplicationImpl* inImpl);
	virtual 			~MLibTestApp();

	virtual void		DoNew();

	virtual bool		UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex, bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex, uint32 inModifiers);

//	void				AddRecent(const std::string& inRecent);
//	void				OpenRecent(const std::string& inRecent);

  private:

	virtual void		DoAbout();

//	virtual bool		AllowQuit(bool inLogOff);
	virtual void		DoQuit();

	virtual void		UpdateSpecialMenu(const std::string& inName, MMenu* inMenu);

	virtual void		Initialise();
	virtual void		SaveGlobals();

//	virtual void		Pulse();
};
