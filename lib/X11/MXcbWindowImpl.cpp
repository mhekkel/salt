//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include "MError.hpp"

#include "MX11ApplicationImpl.hpp"
#include "MXcbWindowImpl.hpp"

using namespace std;

MXcbWindowImpl::MXcbWindowImpl(MWindowFlags inFlags, const string& inMenu,
		MWindow* inWindow, xcb_connection_t* inConnection, xcb_screen_t* inScreen)
	: MWindowImpl(inFlags, inWindow)
	, MXcbWinMixin(inConnection, inScreen)
	, mMenubar(nullptr)
{
	if (not inMenu.empty())
		mMenubar = MWindowImpl::CreateMenu(inMenu);
}

MXcbWindowImpl::~MXcbWindowImpl()
{
}

void MXcbWindowImpl::Create(MRect inBounds, const string& inTitle)
{
	uint32 mask = 0;
	vector<uint32> values;
	
	mask |= XCB_CW_BACK_PIXEL;
	values.push_back(mScreen->white_pixel);
	
	mask |= XCB_CW_EVENT_MASK;
	values.push_back(
		XCB_EVENT_MASK_EXPOSURE              | XCB_EVENT_MASK_BUTTON_PRESS         |
		XCB_EVENT_MASK_BUTTON_RELEASE        | XCB_EVENT_MASK_POINTER_MOTION       |
		XCB_EVENT_MASK_ENTER_WINDOW          | XCB_EVENT_MASK_LEAVE_WINDOW         |
		XCB_EVENT_MASK_KEY_PRESS             | XCB_EVENT_MASK_KEY_RELEASE          |
		XCB_EVENT_MASK_VISIBILITY_CHANGE     | XCB_EVENT_MASK_STRUCTURE_NOTIFY     |
//		XCB_EVENT_MASK_RESIZE_REDIRECT       | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY  |
//		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE         |
		XCB_EVENT_MASK_PROPERTY_CHANGE       //| XCB_EVENT_MASK_COLOR_MAP_CHANGE     |
//		XCB_EVENT_MASK_OWNER_GRAB_BUTTON
	);

	auto cookie0 = xcb_create_window_checked(mConnection,
                     XCB_COPY_FROM_PARENT,            /* depth               */
                     mWindowID,                       /* window Id           */
                     mScreen->root,                   /* parent window       */
                     inBounds.x, inBounds.y,          /* x, y                */
                     inBounds.width, inBounds.height, /* width, height       */
                     10,                              /* border_width        */
                     XCB_WINDOW_CLASS_INPUT_OUTPUT,   /* class               */
                     mScreen->root_visual,            /* visual              */
                     mask, &values[0]);               /* masks */

	xcb_generic_error_t *error;
	if ((error = xcb_request_check(mConnection, cookie0))) {
		free(error);
		throw runtime_error("could not create window");
	}

	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(mConnection, 1, 12, "WM_PROTOCOLS");
	mWMProtocolsAtomReply = xcb_intern_atom_reply(mConnection, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(mConnection, 0, 16, "WM_DELETE_WINDOW");
	mDeleteWindowAtomReply = xcb_intern_atom_reply(mConnection, cookie2, 0);

	xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindowID, mWMProtocolsAtomReply->atom,
		4, 32, 1, &mDeleteWindowAtomReply->atom);

	/* Map the window on the screen */
	xcb_map_window(mConnection, mWindowID);
}

void MXcbWindowImpl::SetTitle(string inTitle)
{
	xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindowID,
		XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, inTitle.length(), inTitle.c_str());

	xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindowID,
		XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8, inTitle.length(), inTitle.c_str());
}

void MXcbWindowImpl::Show()
{
}

void MXcbWindowImpl::Hide()
{
}

void MXcbWindowImpl::Select()
{
}

void MXcbWindowImpl::Close()
{
	xcb_void_cookie_t cookie = xcb_destroy_window_checked(mConnection, mWindowID);

	xcb_generic_error_t *error;
	if ((error = xcb_request_check(mConnection, cookie))) {
		fprintf(stderr, "Could not close the window\n");
		free(error);
		return;
	}
}

MHandler* MXcbWindowImpl::GetFocus()
{
	return nullptr;
}

void MXcbWindowImpl::ResizeWindow(int32 inWidthDelta, int32 inHeightDelta)
{
}

void MXcbWindowImpl::SetWindowPosition(MRect inBounds, bool inTransition)
{
}

void MXcbWindowImpl::GetWindowPosition(MRect& outBounds) const
{
}

void MXcbWindowImpl::Invalidate(MRect inRect)
{
}

void MXcbWindowImpl::Validate(MRect inRect)
{
}

void MXcbWindowImpl::UpdateNow()
{
}

void MXcbWindowImpl::ScrollRect(MRect inRect, int32 inDeltaH, int32 inDeltaV)
{
}

bool MXcbWindowImpl::GetMouse(int32& outX, int32& outY, uint32& outModifiers)
{
	return false;
}

uint32 MXcbWindowImpl::GetModifiers() const
{
	return 0;
}

bool MXcbWindowImpl::WaitMouseMoved(int32 inX, int32 inY)
{
	return false;
}

void MXcbWindowImpl::SetCursor(MCursor inCursor)
{
}

void MXcbWindowImpl::ObscureCursor()
{
}

void MXcbWindowImpl::ConvertToScreen(int32& ioX, int32& ioY) const
{
}

void MXcbWindowImpl::ConvertFromScreen(int32& ioX, int32& ioY) const
{
}

void MXcbWindowImpl::ClientMessageEvent(xcb_client_message_event_t* inEvent)
{
	if (inEvent->data.data32[0] == mDeleteWindowAtomReply->atom)
	{
		if (mWindow->AllowClose(false))
			mWindow->Close();
	}
	else
		MXcbWinMixin::ClientMessageEvent(inEvent);
}

void MXcbWindowImpl::DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent)
{
	MWindow* w = mWindow;
	mWindow = nullptr;
	delete w;
	
	MXcbWinMixin::DestroyNotifyEvent(inEvent);
}

void MXcbWindowImpl::MapNotifyEvent(xcb_map_notify_event_t* inEvent)
{
	mWindow->Mapped();
}

void MXcbWindowImpl::UnmapNotifyEvent(xcb_unmap_notify_event_t* inEvent)
{
	mWindow->Unmapped();
}

void MXcbWindowImpl::ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent)
{
	MRect frame;
	mWindow->GetFrame(frame);
	
	if (inEvent->width != frame.width or inEvent->height != frame.height)
		mWindow->MView::ResizeFrame(inEvent->width - frame.width, inEvent->height - frame.height);

#if DEBUG
	mWindow->GetFrame(frame);
	PRINT(("New frame is (%d,%d,%d,%d)", frame.x, frame.y, frame.width, frame.height));
#endif
}

// --------------------------------------------------------------------

MWindowImpl* MWindowImpl::Create(const string& inTitle, MRect inBounds,
	MWindowFlags inFlags, const string& inMenu, MWindow* inWindow)
{
	MXcbWindowImpl* result = new MXcbWindowImpl(inFlags, inMenu, inWindow,
		MX11ApplicationImpl::Instance().GetXCBConnection(),
		MX11ApplicationImpl::Instance().GetXCBScreen());
	result->Create(inBounds, inTitle);
	return result;
}

void MWindow::GetMainScreenBounds(MRect& outRect)
{
	auto screen = MX11ApplicationImpl::Instance().GetXCBScreen();
	
	outRect = MRect(0, 0, screen->width_in_pixels, screen->height_in_pixels);
}
