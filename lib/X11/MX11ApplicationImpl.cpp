//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include <cstring>

#include "MUtils.hpp"

#include "MAcceleratorTable.hpp"
#include "MAlerts.hpp"
#include "MX11ApplicationImpl.hpp"
#include "MXcbWindowImpl.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MResources.hpp"

using namespace std;

MX11ApplicationImpl* MX11ApplicationImpl::sInstance;

MX11ApplicationImpl::MX11ApplicationImpl()
{
	sInstance = this;
	
	mConnection = xcb_connect(nullptr, nullptr);
	if (mConnection == nullptr)
		THROW(("Could not connect to X11 server"));
	
	mScreen = xcb_setup_roots_iterator(xcb_get_setup(mConnection)).data;
	if (mScreen == nullptr)
		THROW(("Could not get screen"));
}

void MX11ApplicationImpl::Initialise()
{
//	GList* iconList = nullptr;
//
//	mrsrc::rsrc appIconResource("Icons/appicon.png");
//	GInputStream* s = g_memory_input_stream_new_from_data(appIconResource.data(), appIconResource.size(), nullptr);
//	THROW_IF_NIL(s);
//	
//	GError* error = nullptr;
//	GdkPixbuf* icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
//	if (icon)
//		iconList = g_list_append(iconList, icon);
//	
//	if (error)
//		g_free(error);
//
//	mrsrc::rsrc smallAppIconResource("Icons/appicon.png");
//	s = g_memory_input_stream_new_from_data(smallAppIconResource.data(), smallAppIconResource.size(), nullptr);
//	THROW_IF_NIL(s);
//	
//	icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
//	if (icon)
//		iconList = g_list_append(iconList, icon);
//	
//	if (error)
//		g_free(error);
//
//	if (iconList)
//		gtk_window_set_default_icon_list(iconList);
//	
//	// now start up the normal executable
//	gtk_window_set_default_icon_name ("salt-terminal");
//
//	gdk_notify_startup_complete();
}

MX11ApplicationImpl::~MX11ApplicationImpl()
{
}

int MX11ApplicationImpl::RunEventLoop()
{
	/* We flush the request */
	xcb_flush (mConnection);

	while (not gApp->IsQuitting())
	{
		auto e = xcb_poll_for_event(mConnection);
		if (e == nullptr)
		{
			if (xcb_connection_has_error(mConnection))
			{
				cerr << "Connection has error, exiting" << endl;
				break;
			}
			
			gApp->Pulse();
			continue;
		}

//PRINT(("Event(%d, %d)", e->response_type, e->sequence));
		switch (e->response_type & ~0x80)
		{
			case XCB_KEY_PRESS:			KeyPressEvent((xcb_key_press_event_t*)e);					break;
			case XCB_KEY_RELEASE:		KeyReleaseEvent((xcb_key_release_event_t*)e);				break;
			case XCB_BUTTON_PRESS:		ButtonPressEvent((xcb_button_press_event_t*)e);				break;
			case XCB_BUTTON_RELEASE:	ButtonReleaseEvent((xcb_key_release_event_t*)e);			break;
			case XCB_MOTION_NOTIFY:		MotionNotifyEvent((xcb_motion_notify_event_t*)e);			break;
			case XCB_ENTER_NOTIFY:		EnterNotifyEvent((xcb_enter_notify_event_t*)e);				break;
			case XCB_LEAVE_NOTIFY:		LeaveNotifyEvent((xcb_leave_notify_event_t*)e);				break;
			case XCB_FOCUS_IN:			FocusInEvent((xcb_focus_in_event_t*)e);						break;
			case XCB_FOCUS_OUT:			FocusOutEvent((xcb_focus_out_event_t*)e);					break;
			case XCB_KEYMAP_NOTIFY:		KeymapNotifyEvent((xcb_keymap_notify_event_t*)e);			break;
			case XCB_EXPOSE:			ExposeEvent((xcb_expose_event_t*)e);						break;
			case XCB_GRAPHICS_EXPOSURE:	GraphicsExposeEvent((xcb_graphics_exposure_event_t*)e);		break;
			case XCB_NO_EXPOSURE:		NoExposureEvent((xcb_no_exposure_event_t*)e);				break;
			case XCB_VISIBILITY_NOTIFY:	VisibilityNotifyEvent((xcb_visibility_notify_event_t*)e);	break;
			case XCB_CREATE_NOTIFY:		CreateNotifyEvent((xcb_create_notify_event_t*)e);			break;
			case XCB_DESTROY_NOTIFY:	DestroyNotifyEvent((xcb_destroy_notify_event_t*)e);			break;
			case XCB_UNMAP_NOTIFY:		UnmapNotifyEvent((xcb_unmap_notify_event_t*)e);				break;
			case XCB_MAP_NOTIFY:		MapNotifyEvent((xcb_map_notify_event_t*)e);					break;
			case XCB_MAP_REQUEST:		MapRequestEvent((xcb_map_request_event_t*)e);				break;
			case XCB_REPARENT_NOTIFY:	ReparentNotitfyEvent((xcb_reparent_notify_event_t*)e);		break;
			case XCB_CONFIGURE_NOTIFY:	ConfigureNotifyEvent((xcb_configure_notify_event_t*)e);		break;
			case XCB_CONFIGURE_REQUEST:	ConfigureRequestEvent((xcb_configure_request_event_t*)e);	break;
			case XCB_GRAVITY_NOTIFY:	GravityNotifyEvent((xcb_gravity_notify_event_t*)e);			break;
			case XCB_RESIZE_REQUEST:	ResizeRequestEvent((xcb_resize_request_event_t*)e);			break;
			case XCB_CIRCULATE_REQUEST:	CirculateRequestEvent((xcb_circulate_notify_event_t*)e);	break;
			case XCB_PROPERTY_NOTIFY:	PropertyNotifyEvent((xcb_property_notify_event_t*)e);		break;
			case XCB_SELECTION_CLEAR:	SelectionClearEvent((xcb_selection_request_event_t*)e);		break;
			case XCB_SELECTION_REQUEST:	SelectionRequestEvent((xcb_selection_request_event_t*)e);	break;
			case XCB_SELECTION_NOTIFY:	SelectionNotifyEvent((xcb_selection_request_event_t*)e);	break;
			case XCB_COLORMAP_NOTIFY:	ColormapNotifyEvent((xcb_colormap_notify_event_t*)e);		break;
			case XCB_CLIENT_MESSAGE:	ClientMessageEvent((xcb_client_message_event_t*)e);			break;
			case XCB_MAPPING_NOTIFY:	MappingNotifyEvent((xcb_mapping_notify_event_t*)e);			break;
			case XCB_GE_GENERIC:		GeGenericEvent((xcb_ge_generic_event_t*)e);					break;
			default:
				PRINT(("Unknown event opcode: %d", e->response_type & ~0x80));
				break;
		}

		/* Free the Generic Event */
		free(e);
	}
	
//	uint32 snooper = gtk_key_snooper_install(&MX11ApplicationImpl::Snooper, nullptr);
//
//	g_timeout_add(50, &MX11ApplicationImpl::Timeout, nullptr);
//
//	gdk_threads_enter();
//	gtk_main();
//	gdk_threads_leave();
//
//	gtk_key_snooper_remove(snooper);

	return 0;
}

void MX11ApplicationImpl::Quit()
{
//	gtk_main_quit();
}

void MX11ApplicationImpl::KeyPressEvent(xcb_key_press_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->KeyPressEvent(inEvent);
}

void MX11ApplicationImpl::KeyReleaseEvent(xcb_key_release_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->KeyReleaseEvent(inEvent);
}

void MX11ApplicationImpl::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->ButtonPressEvent(inEvent);
}

void MX11ApplicationImpl::ButtonReleaseEvent(xcb_key_release_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->ButtonReleaseEvent(inEvent);
}

void MX11ApplicationImpl::MotionNotifyEvent(xcb_motion_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->MotionNotifyEvent(inEvent);
}

void MX11ApplicationImpl::EnterNotifyEvent(xcb_enter_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->EnterNotifyEvent(inEvent);
}

void MX11ApplicationImpl::LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->LeaveNotifyEvent(inEvent);
}

void MX11ApplicationImpl::FocusInEvent(xcb_focus_in_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->FocusInEvent(inEvent);
}

void MX11ApplicationImpl::FocusOutEvent(xcb_focus_out_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->FocusOutEvent(inEvent);
}

void MX11ApplicationImpl::KeymapNotifyEvent(xcb_keymap_notify_event_t* inEvent)
{
	PRINT(("KeymapNotifyEvent"));
}

void MX11ApplicationImpl::ExposeEvent(xcb_expose_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ExposeEvent(inEvent);
}

void MX11ApplicationImpl::GraphicsExposeEvent(xcb_graphics_exposure_event_t* inEvent)
{
	PRINT((__func__));
}

void MX11ApplicationImpl::NoExposureEvent(xcb_no_exposure_event_t* inEvent)
{
	PRINT((__func__));
}

void MX11ApplicationImpl::VisibilityNotifyEvent(xcb_visibility_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->VisibilityNotifyEvent(inEvent);
}

void MX11ApplicationImpl::CreateNotifyEvent(xcb_create_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->CreateNotifyEvent(inEvent);
}

void MX11ApplicationImpl::DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->DestroyNotifyEvent(inEvent);
}

void MX11ApplicationImpl::UnmapNotifyEvent(xcb_unmap_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->UnmapNotifyEvent(inEvent);
}

void MX11ApplicationImpl::MapNotifyEvent(xcb_map_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->MapNotifyEvent(inEvent);
}

void MX11ApplicationImpl::MapRequestEvent(xcb_map_request_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->MapRequestEvent(inEvent);
}

void MX11ApplicationImpl::ReparentNotitfyEvent(xcb_reparent_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ReparentNotitfyEvent(inEvent);
}

void MX11ApplicationImpl::ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ConfigureNotifyEvent(inEvent);
}

void MX11ApplicationImpl::ConfigureRequestEvent(xcb_configure_request_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ConfigureRequestEvent(inEvent);
}

void MX11ApplicationImpl::GravityNotifyEvent(xcb_gravity_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->GravityNotifyEvent(inEvent);
}

void MX11ApplicationImpl::ResizeRequestEvent(xcb_resize_request_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ResizeRequestEvent(inEvent);
}

void MX11ApplicationImpl::CirculateRequestEvent(xcb_circulate_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->CirculateRequestEvent(inEvent);
}

void MX11ApplicationImpl::PropertyNotifyEvent(xcb_property_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->PropertyNotifyEvent(inEvent);
}

void MX11ApplicationImpl::SelectionClearEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT((__func__));
}

void MX11ApplicationImpl::SelectionRequestEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT((__func__));
}

void MX11ApplicationImpl::SelectionNotifyEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT((__func__));
}

void MX11ApplicationImpl::ColormapNotifyEvent(xcb_colormap_notify_event_t* inEvent)
{
	PRINT((__func__));
}

void MX11ApplicationImpl::ClientMessageEvent(xcb_client_message_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ClientMessageEvent(inEvent);
}

void MX11ApplicationImpl::MappingNotifyEvent(xcb_mapping_notify_event_t* inEvent)
{
	PRINT((__func__));
}

void MX11ApplicationImpl::GeGenericEvent(xcb_ge_generic_event_t* inEvent)
{
	PRINT((__func__));
}



