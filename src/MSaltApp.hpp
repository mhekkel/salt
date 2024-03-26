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

	void DoNew() override;

	void Execute(const std::string &inCommand,
		const std::vector<std::string> &inArguments) override;

	void AddRecent(const ConnectInfo &inRecent);
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

	static MSaltApp &instance()
	{
		return static_cast<MSaltApp &>(*gApp);
	}

	template <typename Handler>
	void execute(Handler &&h)
	{
		mImpl->execute(std::move(h));
	}

  private:
	bool AllowQuit(bool inLogOff) override;
	void DoQuit() override;

	void UpdateSpecialMenu(const std::string &inName, MMenu *inMenu) override;
	void UpdateWindowMenu(MMenu *inMenu) override;
	void UpdateRecentSessionMenu(MMenu *inMenu);
	void UpdatePublicKeyMenu(MMenu *inMenu);
	void UpdateTOTPMenu(MMenu *inMenu);

	void Initialise() override;
	void SaveGlobals() override;

	void OnNew();
	void OnConnect();
	void OnQuit();
	void OnNextTerminal();
	void OnPrevTerminal();
	void OnAbout();

	MCommand<void()> cNew;
	MCommand<void()> cConnect;
	MCommand<void()> cQuit;
	MCommand<void()> cNextTerminal;
	MCommand<void()> cPrevTerminal;
	MCommand<void()> cAbout;

	asio_ns::io_context mIOContext;
	asio_ns::execution_context *mExContext = &mIOContext;
	std::thread mIOContextThread;

	std::deque<ConnectInfo> mRecent;
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
