//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MGTKWINDOWIMPL_H
#define MGTKWINDOWIMPL_H

#include <list>

#include "MMenu.hpp"
#include "MWindowImpl.hpp"
#include "MGtkWidgetMixin.hpp"

class MGtkWindowImpl : public MWindowImpl, public MGtkWidgetMixin
{
  public:
					MGtkWindowImpl(MWindowFlags inFlags,
						const std::string& inMenu, MWindow* inWindow);
	virtual			~MGtkWindowImpl();

	static void		RecycleWindows();

	virtual void	Create(MRect inBounds, const std::string& inTitle);
	
	// A window contains a VBox, and in this VBox you have to add the various elements.
	// (in the right order, please!)
	virtual void	AddMenubarWidget(GtkWidget* inWidget);
	virtual void	AddStatusbarWidget(MGtkWidgetMixin* inChild);
	virtual void	Append(MGtkWidgetMixin* inChild, MControlPacking inPacking,
						bool inExpand, bool inFill, uint32 inPadding);

	virtual void	SetTitle(std::string inTitle);

	virtual void	Show();
	virtual void	Hide();

	virtual bool	Visible() const;

	virtual void	Select();
	virtual void	Close();

	virtual void	ResizeWindow(int32 inWidthDelta, int32 inHeightDelta);

	virtual void	SetWindowPosition(MRect inBounds, bool inTransition);
	virtual void	GetWindowPosition(MRect& outBounds) const;
	
	virtual void	Invalidate(MRect inRect);
	virtual void	Validate(MRect inRect);
	virtual void	UpdateNow();

	virtual void	ScrollRect(MRect inRect, int32 inDeltaH, int32 inDeltaV);
	
	virtual bool	GetMouse(int32& outX, int32& outY, uint32& outModifiers);
	virtual bool	WaitMouseMoved(int32 inX, int32 inY);

	virtual void	SetCursor(MCursor inCursor);
	virtual void	ObscureCursor();

	virtual void	ConvertToScreen(int32& ioX, int32& ioY) const;
	virtual void	ConvertFromScreen(int32& ioX, int32& ioY) const;

	virtual uint32	GetModifiers() const;

	MWindow*		GetWindow() const						{ return mWindow; }
	
	virtual MHandler* GetFocus();

  protected:

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, const std::string& inText);

	virtual bool	OnDestroy();
	virtual bool	OnDelete(GdkEvent* inEvent);

	bool			ChildFocus(GdkEventFocus* inEvent);
	MSlot<bool(GdkEventFocus*)> mChildFocus;
	
	bool			OnMapEvent(GdkEvent* inEvent);
	MSlot<bool(GdkEvent*)> mMapEvent;

	virtual bool	OnConfigureEvent(GdkEventConfigure* inEvent);

//	void			Changed();
//	MSlot<void()>	mChanged;

	virtual void	DoForEach(GtkWidget* inWidget);
	static void		DoForEachCallBack(GtkWidget* inWidget, gpointer inUserData);

	MMenuBar*		mMenubar;
	GtkWidget*		mMainVBox;
	MGtkWidgetMixin*mFocus;
	bool			mConfigured;
	
	static std::list<MWindow*>	sRecycle;
};

#endif
