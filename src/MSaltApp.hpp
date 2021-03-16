// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#if defined(_MSC_VER)
#pragma comment(lib, "libzeep")
#pragma comment(lib, "cryptlib")
#endif

#include <pinch/connection_pool.hpp>

#include "MApplication.hpp"
#include "MTypes.hpp"

extern const char kAppName[], kVersionString[];

const uint32_t
	cmd_Connect = 'Conn',
	cmd_Disconnect = 'Disc',
	cmd_Reset = 'rset',
	cmd_NextTerminal = 'nxtt',
	cmd_PrevTerminal = 'prvt',
	cmd_OpenRecentSession = 'recS',
	cmd_ClearRecentSessions = 'recC',
	cmd_CloneTerminal = 'clon',
	cmd_DropPublicKey = 'DPbK',
	cmd_DropTerminfo = 'DTin',
	cmd_Register = 'regi',
	cmd_ForwardPort = 'TnlP',
	cmd_ProxySOCKS = 'TnlS',
	cmd_ProxyHTTP = 'TnlH',
	cmd_Rekey = 'ReKy',
	cmd_Explore = 'Brws',
	cmd_AddNewTOTPHash = '+otp',
	cmd_EnterTOTP = 'totp';

class MWindow;

// ===========================================================================

class MSaltApp : public MApplication
{
  public:
	MSaltApp(MApplicationImpl *inImpl);

	~MSaltApp();

	virtual void DoNew();
	virtual void Open(const std::string &inURL);

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked);
	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers);

	void AddRecent(const std::string &inRecent);
	void OpenRecent(const std::string &inRecent);

	pinch::connection_pool &GetConnectionPool() { return mConnectionPool; }

  private:
	virtual void DoAbout();

	virtual bool AllowQuit(bool inLogOff);
	virtual void DoQuit();

	virtual void UpdateSpecialMenu(const std::string &inName, MMenu *inMenu);
	void UpdateWindowMenu(MMenu *inMenu);
	void UpdateRecentSessionMenu(MMenu *inMenu);
	void UpdatePublicKeyMenu(MMenu *inMenu);
	void UpdateTOTPMenu(MMenu *inMenu);

	virtual void Initialise();
	virtual void SaveGlobals();

	std::deque<std::string> mRecent;
	pinch::connection_pool mConnectionPool;
};
