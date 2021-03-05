//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <list>

#include <boost/thread/mutex.hpp>

class MXcbWinMixin
{
  public:
	MXcbWinMixin();
	MXcbWinMixin(xcb_connection_t* inConnection,
		xcb_screen_t* inScreen);

	virtual ~MXcbWinMixin();

	static MXcbWinMixin* FetchImpl(uint32 inWindowID);
	
	uint32 GetWindowID() const			{ return mWindowID; }

	void Show();
	void Hide();

	virtual void KeyPressEvent(xcb_key_press_event_t* inEvent);
	virtual void KeyReleaseEvent(xcb_key_release_event_t* inEvent);
	virtual void ButtonPressEvent(xcb_button_press_event_t* inEvent);
	virtual void ButtonReleaseEvent(xcb_key_release_event_t* inEvent);
	virtual void MotionNotifyEvent(xcb_motion_notify_event_t* inEvent);
	virtual void EnterNotifyEvent(xcb_enter_notify_event_t* inEvent);
	virtual void LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent);
	virtual void FocusInEvent(xcb_focus_in_event_t* inEvent);
	virtual void FocusOutEvent(xcb_focus_out_event_t* inEvent);
	virtual void KeymapNotifyEvent(xcb_keymap_notify_event_t* inEvent);
	virtual void ExposeEvent(xcb_expose_event_t* inEvent);
	virtual void GraphicsExposeEvent(xcb_graphics_exposure_event_t* inEvent);
	virtual void NoExposureEvent(xcb_no_exposure_event_t* inEvent);
	virtual void VisibilityNotifyEvent(xcb_visibility_notify_event_t* inEvent);
	virtual void CreateNotifyEvent(xcb_create_notify_event_t* inEvent);
	virtual void DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent);
	virtual void UnmapNotifyEvent(xcb_unmap_notify_event_t* inEvent);
	virtual void MapNotifyEvent(xcb_map_notify_event_t* inEvent);
	virtual void MapRequestEvent(xcb_map_request_event_t* inEvent);
	virtual void ReparentNotitfyEvent(xcb_reparent_notify_event_t* inEvent);
	virtual void ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent);
	virtual void ConfigureRequestEvent(xcb_configure_request_event_t* inEvent);
	virtual void GravityNotifyEvent(xcb_gravity_notify_event_t* inEvent);
	virtual void ResizeRequestEvent(xcb_resize_request_event_t* inEvent);
	virtual void CirculateRequestEvent(xcb_circulate_notify_event_t* inEvent);
	virtual void PropertyNotifyEvent(xcb_property_notify_event_t* inEvent);
	virtual void SelectionClearEvent(xcb_selection_request_event_t* inEvent);
	virtual void SelectionRequestEvent(xcb_selection_request_event_t* inEvent);
	virtual void SelectionNotifyEvent(xcb_selection_request_event_t* inEvent);
	virtual void ColormapNotifyEvent(xcb_colormap_notify_event_t* inEvent);
	virtual void ClientMessageEvent(xcb_client_message_event_t* inEvent);
	virtual void MappingNotifyEvent(xcb_mapping_notify_event_t* inEvent);
	virtual void GeGenericEvent(xcb_ge_generic_event_t* inEvent);

  protected:
	xcb_connection_t* mConnection;
	xcb_screen_t* mScreen;
	uint32 mWindowID;

  private:
	static boost::mutex				sMutex;
	static std::list<MXcbWinMixin*>	sWinMixins;
};
