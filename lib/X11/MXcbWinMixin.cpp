//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include "MXcbWinMixin.hpp"
#include "MX11ApplicationImpl.hpp"

#include "MError.hpp"

using namespace std;

list<MXcbWinMixin*> MXcbWinMixin::sWinMixins;
boost::mutex MXcbWinMixin::sMutex;

MXcbWinMixin::MXcbWinMixin()
	: MXcbWinMixin(
		MX11ApplicationImpl::Instance().GetXCBConnection(),
		MX11ApplicationImpl::Instance().GetXCBScreen()
	)
{
}

MXcbWinMixin::MXcbWinMixin(xcb_connection_t* inConnection,
	xcb_screen_t* inScreen)
	: mConnection(inConnection)
	, mScreen(inScreen)
	, mWindowID(xcb_generate_id(mConnection))
{
	boost::unique_lock<boost::mutex> lock(sMutex);
	sWinMixins.push_back(this);
}

MXcbWinMixin::~MXcbWinMixin()
{
	boost::unique_lock<boost::mutex> lock(sMutex);

	sWinMixins.erase(remove(sWinMixins.begin(), sWinMixins.end(), this), sWinMixins.end());
}

MXcbWinMixin* MXcbWinMixin::FetchImpl(uint32 inWindowID)
{
	boost::unique_lock<boost::mutex> lock(sMutex);

	MXcbWinMixin* result = nullptr;
	for (auto m : sWinMixins)
	{
		if (m->mWindowID == inWindowID)
		{
			result = m;
			break;
		}
	}
	return result;
}

void MXcbWinMixin::Show()
{
	xcb_map_window(mConnection, mWindowID);
}

void MXcbWinMixin::Hide()
{
	xcb_unmap_window(mConnection, mWindowID);
}

void MXcbWinMixin::KeyPressEvent(xcb_key_press_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::KeyReleaseEvent(xcb_key_release_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ButtonReleaseEvent(xcb_key_release_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::MotionNotifyEvent(xcb_motion_notify_event_t* inEvent)
{
//	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::EnterNotifyEvent(xcb_enter_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::FocusInEvent(xcb_focus_in_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::FocusOutEvent(xcb_focus_out_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::KeymapNotifyEvent(xcb_keymap_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ExposeEvent(xcb_expose_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::GraphicsExposeEvent(xcb_graphics_exposure_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::NoExposureEvent(xcb_no_exposure_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::VisibilityNotifyEvent(xcb_visibility_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::CreateNotifyEvent(xcb_create_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));

	boost::unique_lock<boost::mutex> lock(sMutex);

	sWinMixins.erase(remove(sWinMixins.begin(), sWinMixins.end(), this), sWinMixins.end());
	mWindowID = 0;
}

void MXcbWinMixin::UnmapNotifyEvent(xcb_unmap_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::MapNotifyEvent(xcb_map_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::MapRequestEvent(xcb_map_request_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ReparentNotitfyEvent(xcb_reparent_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ConfigureRequestEvent(xcb_configure_request_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::GravityNotifyEvent(xcb_gravity_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ResizeRequestEvent(xcb_resize_request_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::CirculateRequestEvent(xcb_circulate_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::PropertyNotifyEvent(xcb_property_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::SelectionClearEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::SelectionRequestEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::SelectionNotifyEvent(xcb_selection_request_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ColormapNotifyEvent(xcb_colormap_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::ClientMessageEvent(xcb_client_message_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::MappingNotifyEvent(xcb_mapping_notify_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

void MXcbWinMixin::GeGenericEvent(xcb_ge_generic_event_t* inEvent)
{
	PRINT(("%s(%d)", __func__, mWindowID));
}

