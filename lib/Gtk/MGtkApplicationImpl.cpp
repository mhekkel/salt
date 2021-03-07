//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <cstring>

#include "MUtils.hpp"

#include "MAcceleratorTable.hpp"
#include "MAlerts.hpp"
#include "MGtkApplicationImpl.hpp"
#include "MGtkWindowImpl.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "mrsrc.hpp"

using namespace std;

MGtkApplicationImpl::MGtkApplicationImpl()
{
	sInstance = this;
}

void MGtkApplicationImpl::Initialise()
{
	GList* iconList = nullptr;

	mrsrc::rsrc appIconResource("Icons/appicon.png");
	GInputStream* s = g_memory_input_stream_new_from_data(appIconResource.data(), appIconResource.size(), nullptr);
	THROW_IF_NIL(s);
	
	GError* error = nullptr;
	GdkPixbuf* icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
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
	gtk_window_set_default_icon_name ("salt-terminal");

	gdk_notify_startup_complete();
}

MGtkApplicationImpl::~MGtkApplicationImpl()
{
}

int MGtkApplicationImpl::RunEventLoop()
{
	g_timeout_add(50, &MGtkApplicationImpl::Timeout, nullptr);

	gtk_main();
	
	return 0;
}

void MGtkApplicationImpl::Quit()
{
	gtk_main_quit();
}

gboolean MGtkApplicationImpl::Timeout(gpointer inData)
{
	try
	{
		MGtkWindowImpl::RecycleWindows();
		
		gApp->Pulse();
	}
	catch (exception& e)
	{
		DisplayError(e);
	}

	return true;
}



