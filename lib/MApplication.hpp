//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <list>
#include <vector>
#include <deque>
#include <filesystem>

#include "MTypes.hpp"
#include "MHandler.hpp"
#include "MP2PEvents.hpp"
#include "MApplicationImpl.hpp"

extern const char kAppName[], kVersionString[];

extern std::filesystem::path gExecutablePath;

class MWindow;

// ===========================================================================

class MApplication : public MHandler
{
public:
	static MApplication *Create(MApplicationImpl *inImpl);
	static void Install(const std::string &inPrefix);

	~MApplication();
	virtual void Initialise();

	virtual void DoNew();
	virtual void DoOpen();
	virtual void Open(const std::string &inURL);

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex,
									 bool &outEnabled, bool &outChecked);
	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex,
								uint32_t inModifiers);

	virtual void UpdateSpecialMenu(const std::string &inMenuKind, MMenu *inMenu);
	virtual void UpdateWindowMenu(MMenu *inMenu);

	MEventOut<void(double)> eIdle;

	int RunEventLoop();
	virtual void Pulse();

	virtual bool AllowQuit(bool inLogOff);
	virtual void DoQuit();

	bool IsQuitting() const { return mQuitPending; }
	void CancelQuit() { mQuitPending = false; }

	MApplicationImpl& get_executor()
	{
		return *mImpl;
	}

	boost::asio::execution_context& get_context()
	{
		return *mImpl->mExContext;
	}

	boost::asio::io_context& get_io_context()
	{
		return mImpl->mIOContext;
	}

protected:
	MApplication(MApplicationImpl *inImpl);

	typedef std::list<MWindow *> MWindowList;

	virtual void DoSelectWindowFromWindowMenu(uint32_t inIndex);

	virtual void SaveGlobals();

	MApplicationImpl *mImpl;

	bool mQuit;
	bool mQuitPending;
};

// --------------------------------------------------------------------

extern MApplication *gApp;

// --------------------------------------------------------------------

class MAppExecutor
{
public:
	boost::asio::execution_context *m_context;

	bool operator==(const MAppExecutor &other) const noexcept
	{
		return m_context == other.m_context;
	}

	bool operator!=(const MAppExecutor &other) const noexcept
	{
		return !(*this == other);
	}

	boost::asio::execution_context &query(boost::asio::execution::context_t) const noexcept
	{
		return *m_context;
	}

	static constexpr boost::asio::execution::blocking_t::never_t query(
		boost::asio::execution::blocking_t) noexcept
	{
		// This executor always has blocking.never semantics.
		return boost::asio::execution::blocking.never;
	}

	template <class F>
	void execute(F f) const
	{
		gApp->get_executor().execute(std::move(f));
	}
};

