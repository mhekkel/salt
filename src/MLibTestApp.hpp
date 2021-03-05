// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MTypes.hpp"
#include "MApplication.hpp"

extern const char kAppName[], kVersionString[];

class MWindow;

// ===========================================================================

class MLibTestApp : public MApplication
{
  public:

						MLibTestApp(MApplicationImpl* inImpl);
	virtual 			~MLibTestApp();

	virtual void		DoNew();

	virtual bool		UpdateCommandStatus(uint32_t inCommand, MMenu* inMenu, uint32_t inItemIndex, bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32_t inCommand, const MMenu* inMenu, uint32_t inItemIndex, uint32_t inModifiers);

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
