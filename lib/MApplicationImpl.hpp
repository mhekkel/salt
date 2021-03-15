//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>

class MAsyncHandlerBase
{
public:
	MAsyncHandlerBase() {}
	MAsyncHandlerBase(const MAsyncHandlerBase&) = delete;
	MAsyncHandlerBase& operator=(const MAsyncHandlerBase&) = delete;

	virtual ~MAsyncHandlerBase() {}

	void execute();

protected:
	virtual void execute_self() = 0;
};

template<typename Handler>
class MAsyncHandler : public MAsyncHandlerBase
{
public:
	MAsyncHandler(Handler&& handler)
		: mHandler(std::move(handler)) {}
	
	virtual void execute_self() override
	{
		mHandler();
	}

	Handler mHandler;
};

class MApplicationImpl
{
public:
	MApplicationImpl() {}
	virtual ~MApplicationImpl() {}

	virtual void Initialise() = 0;
	virtual int RunEventLoop() = 0;
	virtual void Quit() = 0;

	bool operator==(const MApplicationImpl &other) const noexcept
	{
		return mExContext == other.mExContext && &mHandlerQueue == &other.mHandlerQueue;
	}

	bool operator!=(const MApplicationImpl &other) const noexcept
	{
		return !(*this == other);
	}

	boost::asio::execution_context &query(boost::asio::execution::context_t) const noexcept
	{
		return *mExContext;
	}

	static constexpr boost::asio::execution::blocking_t::never_t query(boost::asio::execution::blocking_t) noexcept
	{
		// This executor always has blocking.never semantics.
		return boost::asio::execution::blocking.never;
	}

	template<typename Handler>
	void execute(Handler&& h)
	{
		std::unique_lock lock(mMutex);

		mHandlerQueue.push_back(new MAsyncHandler{std::move(h)});

		mCV.notify_one();
	}

	boost::asio::io_context mIOContext;
	boost::asio::execution_context* mExContext = &mIOContext;

	std::deque<MAsyncHandlerBase*> mHandlerQueue;
	std::mutex mMutex;
	std::condition_variable mCV;
};
