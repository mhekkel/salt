//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MXcbLib.hpp"

#include <cstring>

#include "MUtils.hpp"

#include "MAcceleratorTable.hpp"
#include "MAlerts.hpp"
#include "MXcbApplicationImpl.hpp"
#include "MXcbWindowImpl.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MResources.hpp"

using namespace std;

MXcbApplicationImpl* MXcbApplicationImpl::sInstance;

MXcbApplicationImpl::MXcbApplicationImpl()
{
	sInstance = this;
	
	mConnection = xcb_connect(nullptr, nullptr);
	if (mConnection == nullptr)
		THROW(("Could not connect to X11 server"));
	
	mScreen = xcb_setup_roots_iterator(xcb_get_setup(mConnection)).data;
	if (mScreen == nullptr)
		THROW(("Could not get screen"));
}

void MXcbApplicationImpl::Initialise()
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

MXcbApplicationImpl::~MXcbApplicationImpl()
{
}

int MXcbApplicationImpl::RunEventLoop()
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
		
		ProcessEvent(e);

		/* Free the Generic Event */
		free(e);
	}
	
	return 0;
}

void MXcbApplicationImpl::ProcessEvent(xcb_generic_event_t* e)
{
	switch (e->response_type & ~0x80)
	{
		case XCB_KEY_PRESS:			KeyPressEvent((xcb_key_press_event_t*)e);					break;
		case XCB_KEY_RELEASE:		KeyReleaseEvent((xcb_key_release_event_t*)e);				break;
		case XCB_BUTTON_PRESS:		ButtonPressEvent((xcb_button_press_event_t*)e);				break;
		case XCB_BUTTON_RELEASE:	ButtonReleaseEvent((xcb_button_press_event_t*)e);			break;
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
}

void MXcbApplicationImpl::Quit()
{
//	gtk_main_quit();
}

void MXcbApplicationImpl::KeyPressEvent(xcb_key_press_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->KeyPressEvent(inEvent);
}

void MXcbApplicationImpl::KeyReleaseEvent(xcb_key_release_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->KeyReleaseEvent(inEvent);
}

void MXcbApplicationImpl::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->ButtonPressEvent(inEvent);
}

void MXcbApplicationImpl::ButtonReleaseEvent(xcb_button_press_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->ButtonReleaseEvent(inEvent);
}

void MXcbApplicationImpl::MotionNotifyEvent(xcb_motion_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->MotionNotifyEvent(inEvent);
}

void MXcbApplicationImpl::EnterNotifyEvent(xcb_enter_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->EnterNotifyEvent(inEvent);
}

void MXcbApplicationImpl::LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->LeaveNotifyEvent(inEvent);
}

void MXcbApplicationImpl::FocusInEvent(xcb_focus_in_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->FocusInEvent(inEvent);
}

void MXcbApplicationImpl::FocusOutEvent(xcb_focus_out_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->event);
	if (impl != nullptr)
		impl->FocusOutEvent(inEvent);
}

void MXcbApplicationImpl::KeymapNotifyEvent(xcb_keymap_notify_event_t* inEvent)
{
	PRINT(("KeymapNotifyEvent"));
}

void MXcbApplicationImpl::ExposeEvent(xcb_expose_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ExposeEvent(inEvent);
}

void MXcbApplicationImpl::GraphicsExposeEvent(xcb_graphics_exposure_event_t* inEvent)
{
	PRINT((__func__));
}

void MXcbApplicationImpl::NoExposureEvent(xcb_no_exposure_event_t* inEvent)
{
	PRINT((__func__));
}

void MXcbApplicationImpl::VisibilityNotifyEvent(xcb_visibility_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->VisibilityNotifyEvent(inEvent);
}

void MXcbApplicationImpl::CreateNotifyEvent(xcb_create_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->CreateNotifyEvent(inEvent);
}

void MXcbApplicationImpl::DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->DestroyNotifyEvent(inEvent);
}

void MXcbApplicationImpl::UnmapNotifyEvent(xcb_unmap_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->UnmapNotifyEvent(inEvent);
}

void MXcbApplicationImpl::MapNotifyEvent(xcb_map_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->MapNotifyEvent(inEvent);
}

void MXcbApplicationImpl::MapRequestEvent(xcb_map_request_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->MapRequestEvent(inEvent);
}

void MXcbApplicationImpl::ReparentNotitfyEvent(xcb_reparent_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ReparentNotitfyEvent(inEvent);
}

void MXcbApplicationImpl::ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ConfigureNotifyEvent(inEvent);
}

void MXcbApplicationImpl::ConfigureRequestEvent(xcb_configure_request_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ConfigureRequestEvent(inEvent);
}

void MXcbApplicationImpl::GravityNotifyEvent(xcb_gravity_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->GravityNotifyEvent(inEvent);
}

void MXcbApplicationImpl::ResizeRequestEvent(xcb_resize_request_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ResizeRequestEvent(inEvent);
}

void MXcbApplicationImpl::CirculateRequestEvent(xcb_circulate_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->CirculateRequestEvent(inEvent);
}

void MXcbApplicationImpl::PropertyNotifyEvent(xcb_property_notify_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->PropertyNotifyEvent(inEvent);
}

void MXcbApplicationImpl::SelectionClearEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT((__func__));
}

void MXcbApplicationImpl::SelectionRequestEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT((__func__));
}

void MXcbApplicationImpl::SelectionNotifyEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT((__func__));
}

void MXcbApplicationImpl::ColormapNotifyEvent(xcb_colormap_notify_event_t* inEvent)
{
	PRINT((__func__));
}

void MXcbApplicationImpl::ClientMessageEvent(xcb_client_message_event_t* inEvent)
{
	MXcbWinMixin* impl = MXcbWinMixin::FetchImpl(inEvent->window);
	if (impl != nullptr)
		impl->ClientMessageEvent(inEvent);
}

void MXcbApplicationImpl::MappingNotifyEvent(xcb_mapping_notify_event_t* inEvent)
{
	PRINT((__func__));
}

void MXcbApplicationImpl::GeGenericEvent(xcb_ge_generic_event_t* inEvent)
{
	PRINT((__func__));
}



