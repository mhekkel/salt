// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#if defined(_MSC_VER)
#pragma comment(lib, "libzeep")
#pragma comment(lib, "cryptlib")
#endif

#include <pinch.hpp>

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

	int RunEventLoop();

	MSaltApp &get_executor()
	{
		return *this;
	}

	asio_ns::execution_context &get_context()
	{
		return *mExContext;
	}

	asio_ns::io_context &get_io_context()
	{
		return mIOContext;
	}

	static MSaltApp& instance()
	{
		return static_cast<MSaltApp &>(*gApp);
	}

	template<typename Handler>
	void execute(Handler&& h)
	{
		mImpl->execute(std::move(h));
	}

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

	asio_ns::io_context mIOContext;
	asio_ns::execution_context* mExContext = &mIOContext;
	std::thread mIOContextThread;

	std::deque<std::string> mRecent;
	pinch::connection_pool mConnectionPool;
};


// --------------------------------------------------------------------

class MAppExecutor
{
  public:
	asio_ns::execution_context *m_context;

	bool operator==(const MAppExecutor &other) const noexcept
	{
		return m_context == other.m_context;
	}

	bool operator!=(const MAppExecutor &other) const noexcept
	{
		return !(*this == other);
	}

	asio_ns::execution_context &query(asio_ns::execution::context_t) const noexcept
	{
		return *m_context;
	}

	static constexpr asio_ns::execution::blocking_t::never_t query(
		asio_ns::execution::blocking_t) noexcept
	{
		// This executor always has blocking.never semantics.
		return asio_ns::execution::blocking.never;
	}

	template <class F>
	void execute(F f) const
	{
		static_cast<MSaltApp *>(gApp)->execute(std::move(f));
	}
};
