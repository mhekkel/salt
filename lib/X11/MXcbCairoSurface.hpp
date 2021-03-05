//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MXcbWinMixin.hpp"

class MView;

class MXcbCairoSurface : public MXcbWinMixin
{
  public:
	MXcbCairoSurface(MView* inView);
	~MXcbCairoSurface();
	
	virtual void AddedToWindow();
	virtual void FrameResized();
	
	virtual void CreateWidget();
	virtual void DrawWidget();

	virtual void Invalidate();
	
//	virtual void MouseEnter();
//	virtual void MouseMove();
//	virtual void MouseLeave();
//	virtual void MouseDown();

	void GetParentAndBounds(MXcbWinMixin*& outParent, MRect& outBounds);

  protected:

	virtual void ButtonPressEvent(xcb_button_press_event_t* inEvent);
	virtual void ButtonReleaseEvent(xcb_key_release_event_t* inEvent);
	virtual void MotionNotifyEvent(xcb_motion_notify_event_t* inEvent);
	virtual void EnterNotifyEvent(xcb_enter_notify_event_t* inEvent);
	virtual void LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent);
	virtual void ExposeEvent(xcb_expose_event_t* inEvent);
	virtual void ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent);

	void SetColor(MColor inColor);

	void RoundedRectanglePath(MRect inBounds, uint32 inRadius);
	void FillRoundedRectangle(MRect inBounds, uint32 inRadius);
	void StrokeRoundedRectangle(MRect inBounds, uint32 inRadius);
	
	void SetFont(const std::string& inFont);
	enum MFontWeight { fwNormal, fwBold };
	enum MFontSlant { fsNormal, fsItalic, fsOblique };
	
	void SetFont(const std::string& inFont, double inSize,
		MFontSlant inSlant = fsNormal, MFontWeight inWeight = fwNormal);
	
	void DrawString(MRect inBounds, const std::string& inText);

	MView*				mView;
	cairo_surface_t*	mSurface;
	cairo_t*			mCairo;
	bool				mMouseIsIn;
};
