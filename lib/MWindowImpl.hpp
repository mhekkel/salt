//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MWindow.hpp"

class MMenuBar;

class MWindowImpl
{
  public:
	static MWindowImpl*	Create(const std::string& inTitle, MRect inBounds,
							MWindowFlags inFlags, const std::string& inMenu,
							MWindow* inWindow);

	static MWindowImpl*	CreateDialog(const std::string& inResource, MWindow* inWindow);

	virtual				~MWindowImpl() {}

	MWindowFlags		GetFlags() const								{ return mFlags; }

	virtual void		SetTransparency(float inAlpha) {}

	virtual void		Finish() {}

	virtual void		SetTitle(std::string inTitle) = 0;
	//virtual std::string	GetTitle() const = 0;

	virtual void		Show() = 0;
	virtual void		Hide() = 0;

	virtual bool		ShowModal() { return false; }

	virtual bool		Visible() const	= 0;

	virtual void		Select() = 0;
	virtual void		Close() = 0;

	//virtual void		ActivateSelf();
	//virtual void		DeactivateSelf();
	//virtual void		BeFocus();
	//virtual void		SubFocusChanged();
	
//	virtual void		SetFocus(MView* inFocus) {}
	virtual MHandler*	GetFocus() = 0;
	
	virtual void		ResizeWindow(int32 inWidthDelta, int32 inHeightDelta) = 0;

	virtual void		SetWindowPosition(MRect inBounds, bool inTransition) = 0;
	virtual void		GetWindowPosition(MRect& outBounds) const = 0;
	
	virtual void		Invalidate(MRect inRect) = 0;
	virtual void		Validate(MRect inRect) = 0;
	virtual void		UpdateNow() = 0;

	virtual void		ScrollRect(MRect inRect, int32 inDeltaH, int32 inDeltaV) = 0;
	
	virtual bool		GetMouse(int32& outX, int32& outY, uint32& outModifiers) = 0;
	virtual uint32		GetModifiers() const = 0;
	virtual bool		WaitMouseMoved(int32 inX, int32 inY) = 0;

	virtual void		SetCursor(MCursor inCursor) = 0;
	virtual void		ObscureCursor() = 0;

	virtual void		ConvertToScreen(int32& ioX, int32& ioY) const = 0;
	virtual void		ConvertFromScreen(int32& ioX, int32& ioY) const = 0;

  protected:
						MWindowImpl(MWindowFlags inFlags, MWindow* inWindow)
							: mWindow(inWindow), mFlags(inFlags) {}

	MMenuBar*			CreateMenu(const std::string& inMenu);
	  
	MWindow*			mWindow;
	MWindowFlags		mFlags;
};
