/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MApplication.hpp"
#include "MCommand.hpp"
#include "MConnectDialog.hpp"
#include "MTypes.hpp"

#include <pinch.hpp>

extern const char kAppName[], kVersionString[];

// ===========================================================================

class MSaltApp : public MApplication
{
  public:
	MSaltApp(MApplicationImpl *inImpl);

	~MSaltApp();

	int HandleCommandLine(int argc, const char * const argv[]) override;

	void DoNew() override;

	void Execute(const std::string &inCommand,
		const std::vector<std::string> &inArguments) override;

	void Open(const ConnectInfo &inRecent, const std::string &inCommand = {});

	pinch::connection_pool &GetConnectionPool() { return mConnectionPool; }

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

	static MSaltApp &Instance()
	{
		return static_cast<MSaltApp &>(*gApp);
	}

	template <typename Handler>
	void execute(Handler &&h)
	{
		mImpl->execute(std::move(h));
	}

	void UpdateWindowMenu();
	void UpdateRecentSessionMenu();
	void UpdatePublicKeyMenu();
	void UpdateTOTPMenu();

  private:
	bool AllowQuit(bool inLogOff) override;
	void DoQuit() override;

	void Initialise() override;
	void SaveGlobals() override;

	void OnNew();
	void OnConnect();
	void OnAddNewTOTP();
	void OnQuit();
	void OnShowPreferences();
	void OnManual();
	void OnAbout();
	void OnSelectTerminal(int inTerminalNr);
	void OnClearRecentMenu();
	void OnOpenRecent(int inConnectionNr);

	MCommand<void()> cNew;
	MCommand<void()> cConnect;
	MCommand<void()> cAddNewTOTP;
	MCommand<void()> cQuit;
	MCommand<void()> cShowPreferences;
	MCommand<void()> cManual;
	MCommand<void()> cAbout;
	MCommand<void(int)> cSelectTerminal;
	MCommand<void()> cClearRecentMenu;
	MCommand<void(int)> cOpenRecent;

	void OnPreferencesChanged();
	MEventIn<void()> ePreferencesChanged;

	asio_ns::io_context mIOContext;
	asio_ns::execution_context *mExContext = &mIOContext;
	std::thread mIOContextThread;

	std::deque<std::pair<ConnectInfo,uint32_t>> mRecent;
	uint32_t mNextRecentNr = 1;

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
		MSaltApp::Instance().execute(std::move(f));
	}
};
