// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#if defined(_MSC_VER)
#pragma comment (lib, "libzeep")
#pragma comment (lib, "cryptlib")
#pragma comment (lib, "MLib")
#pragma comment (lib, "MWinLib")
#endif

#include <assh/connection_pool.hpp>

#include "MTypes.h"
#include "MApplication.h"

extern const char kAppName[], kVersionString[];

const uint32
	cmd_Connect				= 'Conn',
	cmd_Disconnect			= 'Disc',
	cmd_Reset				= 'rset',
	cmd_NextTerminal		= 'nxtt',
	cmd_PrevTerminal		= 'prvt',
	cmd_OpenRecentSession	= 'recS',
	cmd_ClearRecentSessions	= 'recC',
	cmd_CloneTerminal		= 'clon',
	cmd_DropPublicKey		= 'DPbK',
	cmd_DropTerminfo		= 'DTin',
	cmd_Register			= 'regi',
	cmd_ForwardPort			= 'TnlP',
	cmd_ProxySOCKS			= 'TnlS',
	cmd_ProxyHTTP			= 'TnlH',
	cmd_Rekey				= 'ReKy',
	cmd_Explore				= 'Brws',
	cmd_AddNewTOTPHash		= '+otp',
	cmd_EnterTOTP			= 'totp';

class MWindow;

// ===========================================================================

class MSaltApp : public MApplication
{
  public:

						MSaltApp(MApplicationImpl* inImpl);
	
						~MSaltApp();

	virtual void		DoNew();
	virtual void		Open(const std::string& inURL);

	virtual bool		UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex, bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex, uint32 inModifiers);

	void				AddRecent(const std::string& inRecent);
	void				OpenRecent(const std::string& inRecent);

	boost::asio::io_service&
						GetIOService()						{ return mIOService; }
	assh::connection_pool&
						GetConnectionPool()					{ return mConnectionPool; }
	
	bool				ValidateHost(const std::string& inHost,
							const std::string& inAlg, const std::vector<uint8>& inHostKey);
	
  private:

	virtual void		DoAbout();

	virtual bool		AllowQuit(bool inLogOff);
	virtual void		DoQuit();

	virtual void		UpdateSpecialMenu(const std::string& inName, MMenu* inMenu);
	void				UpdateWindowMenu(MMenu* inMenu);
	void				UpdateRecentSessionMenu(MMenu* inMenu);
	void				UpdatePublicKeyMenu(MMenu* inMenu);
	void				UpdateTOTPMenu(MMenu* inMenu);

	virtual void		Initialise();
	virtual void		SaveGlobals();

	virtual void		Pulse();

	struct MKnownHost
	{
		std::string	host;
		std::string	alg;
		std::string	key;
		
		bool operator==(const MKnownHost& rhs) const
		{
			return host == rhs.host and alg == rhs.alg;
		}
	};

	typedef std::list<MKnownHost>	MKnownHostsList;

	std::deque<std::string>		mRecent;
	boost::asio::io_service		mIOService;
	assh::connection_pool		mConnectionPool;
	MKnownHostsList				mKnownHosts;
};
