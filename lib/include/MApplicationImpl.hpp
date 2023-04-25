//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>

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

	template<typename Handler>
	void execute(Handler&& h)
	{
		std::unique_lock lock(mMutex);

		mHandlerQueue.push_back(new MAsyncHandler{std::move(h)});

		mCV.notify_one();
	}

	std::deque<MAsyncHandlerBase*> mHandlerQueue;
	std::mutex mMutex;
	std::condition_variable mCV;
};
