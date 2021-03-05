//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MGTKAPPLICATION_IMPL_H
#define MGTKAPPLICATION_IMPL_H

#include "MApplicationImpl.hpp"

class MXcbApplicationImpl : public MApplicationImpl
{
  public:
	MXcbApplicationImpl();
	virtual ~MXcbApplicationImpl();

	static MXcbApplicationImpl& Instance()		{ return *sInstance; }

	xcb_connection_t* GetXCBConnection() const	{ return mConnection; }
	xcb_screen_t* GetXCBScreen() const			{ return mScreen; }

	void Initialise();
	virtual int RunEventLoop();
	virtual void Quit();

	void ProcessEvent(xcb_generic_event_t* inEvent);

  private:

	void KeyPressEvent(xcb_key_press_event_t* inEvent);
	void KeyReleaseEvent(xcb_key_release_event_t* inEvent);
	void ButtonPressEvent(xcb_button_press_event_t* inEvent);
	void ButtonReleaseEvent(xcb_button_press_event_t* inEvent);
	void MotionNotifyEvent(xcb_motion_notify_event_t* inEvent);
	void EnterNotifyEvent(xcb_enter_notify_event_t* inEvent);
	void LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent);
	void FocusInEvent(xcb_focus_in_event_t* inEvent);
	void FocusOutEvent(xcb_focus_out_event_t* inEvent);
	void KeymapNotifyEvent(xcb_keymap_notify_event_t* inEvent);
	void ExposeEvent(xcb_expose_event_t* inEvent);
	void GraphicsExposeEvent(xcb_graphics_exposure_event_t* inEvent);
	void NoExposureEvent(xcb_no_exposure_event_t* inEvent);
	void VisibilityNotifyEvent(xcb_visibility_notify_event_t* inEvent);
	void CreateNotifyEvent(xcb_create_notify_event_t* inEvent);
	void DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent);
	void UnmapNotifyEvent(xcb_unmap_notify_event_t* inEvent);
	void MapNotifyEvent(xcb_map_notify_event_t* inEvent);
	void MapRequestEvent(xcb_map_request_event_t* inEvent);
	void ReparentNotitfyEvent(xcb_reparent_notify_event_t* inEvent);
	void ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent);
	void ConfigureRequestEvent(xcb_configure_request_event_t* inEvent);
	void GravityNotifyEvent(xcb_gravity_notify_event_t* inEvent);
	void ResizeRequestEvent(xcb_resize_request_event_t* inEvent);
	void CirculateRequestEvent(xcb_circulate_notify_event_t* inEvent);
	void PropertyNotifyEvent(xcb_property_notify_event_t* inEvent);
	void SelectionClearEvent(xcb_selection_request_event_t* inEvent);
	void SelectionRequestEvent(xcb_selection_request_event_t* inEvent);
	void SelectionNotifyEvent(xcb_selection_request_event_t* inEvent);
	void ColormapNotifyEvent(xcb_colormap_notify_event_t* inEvent);
	void ClientMessageEvent(xcb_client_message_event_t* inEvent);
	void MappingNotifyEvent(xcb_mapping_notify_event_t* inEvent);
	void GeGenericEvent(xcb_ge_generic_event_t* inEvent);

//	static gboolean	Timeout(gpointer inData);
	static MXcbApplicationImpl* sInstance;
	xcb_connection_t*	mConnection;
	xcb_screen_t*		mScreen;
};

extern boost::filesystem::path gExecutablePath, gPrefixPath;

#endif
