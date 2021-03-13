//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MApplicationImpl.hpp"

class MGtkApplicationImpl : public MApplicationImpl
{
public:
	MGtkApplicationImpl();
	virtual ~MGtkApplicationImpl();

	static MGtkApplicationImpl *
	GetInstance() { return sInstance; }

	void Initialise();
	virtual int RunEventLoop();
	virtual void Quit();

private:
	static gboolean Timeout(gpointer inData);
	static gboolean Idle(gpointer inData);
	static gboolean HandleAsyncCallback(gpointer inData);

	void ProcessAsyncTasks(GMainContext* context);
	static void DeleteAsyncHandler(gpointer inData);

	static MGtkApplicationImpl *sInstance;

	guint mPulseID = 0;
	std::thread mAsyncTaskThread, mIOContextThread;
};

extern std::filesystem::path gExecutablePath, gPrefixPath;
