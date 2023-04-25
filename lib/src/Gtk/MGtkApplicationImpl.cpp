//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Gtk/MGtkLib.hpp"

#include <cstring>

#include "MUtils.hpp"

#include "MAcceleratorTable.hpp"
#include "MAlerts.hpp"
#include "MApplication.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "Gtk/MGtkApplicationImpl.hpp"
#include "Gtk/MGtkWindowImpl.hpp"
#include "mrsrc.hpp"

using namespace std;

MGtkApplicationImpl::MGtkApplicationImpl()
{

	sInstance = this;
}

void MGtkApplicationImpl::Initialise()
{
	GList *iconList = nullptr;

	mrsrc::rsrc appIconResource("Icons/appicon.png");
	GInputStream *s = g_memory_input_stream_new_from_data(appIconResource.data(), appIconResource.size(), nullptr);
	THROW_IF_NIL(s);

	GError *error = nullptr;
	GdkPixbuf *icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
	if (icon)
		iconList = g_list_append(iconList, icon);

	if (error)
		g_free(error);

	mrsrc::rsrc smallAppIconResource("Icons/appicon.png");
	s = g_memory_input_stream_new_from_data(smallAppIconResource.data(), smallAppIconResource.size(), nullptr);
	THROW_IF_NIL(s);

	icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
	if (icon)
		iconList = g_list_append(iconList, icon);

	if (error)
		g_free(error);

	if (iconList)
		gtk_window_set_default_icon_list(iconList);

	// now start up the normal executable
	gtk_window_set_default_icon_name("salt-terminal");

	gdk_notify_startup_complete();
}

MGtkApplicationImpl::~MGtkApplicationImpl()
{
	mHandlerQueue.push_front(nullptr);
	mCV.notify_one();
	if (mAsyncTaskThread.joinable())
		mAsyncTaskThread.join();
}

int MGtkApplicationImpl::RunEventLoop()
{
	mPulseID = g_timeout_add(100, &MGtkApplicationImpl::Timeout, nullptr);

	// Start processing async tasks

	mAsyncTaskThread = std::thread([this, context = g_main_context_get_thread_default()]() {
		ProcessAsyncTasks(context);
	});

	gtk_main();

	return 0;
}

void MGtkApplicationImpl::Quit()
{
	if (mPulseID)
		g_source_remove(mPulseID);

	gtk_main_quit();

	std::unique_lock lock(mMutex);
	mHandlerQueue.push_front(nullptr);
	mCV.notify_one();
}

void MGtkApplicationImpl::ProcessAsyncTasks(GMainContext *context)
{
	std::unique_lock lock(mMutex);

	bool done = false;

	while (not done)
	{
		mCV.wait(lock, [this]() { return not mHandlerQueue.empty(); });

		while (not mHandlerQueue.empty())
		{
			auto ah = mHandlerQueue.front();
			mHandlerQueue.pop_front();

			if (not ah)
			{
				done = true;
				break;
			}

			g_main_context_invoke_full(context, G_PRIORITY_DEFAULT,
			                           &MGtkApplicationImpl::HandleAsyncCallback, ah,
			                           (GDestroyNotify)&MGtkApplicationImpl::DeleteAsyncHandler);
		}
	}
}

gboolean MGtkApplicationImpl::HandleAsyncCallback(gpointer inData)
{
PRINT(("Handle Async Task in Thread ID = %p", std::this_thread::get_id()));

	MAsyncHandlerBase *handler = reinterpret_cast<MAsyncHandlerBase *>(inData);
	handler->execute();
	return G_SOURCE_REMOVE;
}

void MGtkApplicationImpl::DeleteAsyncHandler(gpointer inData)
{
	MAsyncHandlerBase *handler = reinterpret_cast<MAsyncHandlerBase *>(inData);
	delete handler;
}

gboolean MGtkApplicationImpl::Timeout(gpointer inData)
{
	try
	{
		MGtkWindowImpl::RecycleWindows();

		gApp->Pulse();
	}
	catch (exception &e)
	{
		DisplayError(e);
	}

	return true;
}
