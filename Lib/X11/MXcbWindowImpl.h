//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MWindowImpl.h"
#include "MXcbWinMixin.h"

class MXcbWindowImpl : public MWindowImpl, public MXcbWinMixin
{
  public:
	MXcbWindowImpl(MWindowFlags inFlags, const std::string& inMenu,
		MWindow* inWindow, xcb_connection_t* inConnection,
		xcb_screen_t* inScreen);

	~MXcbWindowImpl();

	void Create(MRect inBounds, const std::string& inTitle);

	virtual void SetTitle(std::string inTitle);

	virtual void Show();
	virtual void Hide();

//	virtual bool ShowModal() { return false; }

	virtual bool Visible() const		{ return mVisible; }

	virtual void Select();
	virtual void Close();

	virtual MHandler* GetFocus();
	
	virtual void ResizeWindow(int32 inWidthDelta, int32 inHeightDelta);

	virtual void SetWindowPosition(MRect inBounds, bool inTransition);
	virtual void GetWindowPosition(MRect& outBounds) const;
	
	virtual void Invalidate(MRect inRect);
	virtual void Validate(MRect inRect);
	virtual void UpdateNow();

	virtual void ScrollRect(MRect inRect, int32 inDeltaH, int32 inDeltaV);
	
	virtual bool GetMouse(int32& outX, int32& outY, uint32& outModifiers);
	virtual uint GetModifiers() const;
	virtual bool WaitMouseMoved(int32 inX, int32 inY);

	virtual void SetCursor(MCursor inCursor);
	virtual void ObscureCursor();

	virtual void ConvertToScreen(int32& ioX, int32& ioY) const;
	virtual void ConvertFromScreen(int32& ioX, int32& ioY) const;

	virtual void ClientMessageEvent(xcb_client_message_event_t* inEvent);
	virtual void DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent);
	virtual void MapNotifyEvent(xcb_map_notify_event_t* inEvent);
	virtual void UnmapNotifyEvent(xcb_unmap_notify_event_t* inEvent);
	virtual void ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent);

  protected:
	MMenuBar* mMenubar;

  private:
	xcb_intern_atom_reply_t*	mWMProtocolsAtomReply;
	xcb_intern_atom_reply_t*	mDeleteWindowAtomReply;
	bool						mVisible;
};

