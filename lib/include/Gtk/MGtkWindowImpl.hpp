//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <list>

#include "MGtkWidgetMixin.hpp"
#include "MMenu.hpp"
#include "MWindowImpl.hpp"

class MGtkWindowImpl : public MWindowImpl, public MGtkWidgetMixin
{
  public:
	MGtkWindowImpl(MWindowFlags inFlags,
		const std::string &inMenu, MWindow *inWindow);
	virtual ~MGtkWindowImpl();

	static void RecycleWindows();

	virtual void Create(MRect inBounds, const std::string &inTitle);

	// A window contains a VBox, and in this VBox you have to add the various elements.
	// (in the right order, please!)
	virtual void AddMenubarWidget(GtkWidget *inWidget);
	virtual void AddStatusbarWidget(MGtkWidgetMixin *inChild);
	virtual void Append(MGtkWidgetMixin *inChild, MControlPacking inPacking,
		bool inExpand, bool inFill, uint32_t inPadding);

	virtual void SetTitle(std::string inTitle);

	virtual void Show();
	virtual void Hide();

	virtual void SetTransientFor(MWindow *inWindow);

	virtual bool Visible() const;

	virtual void Select();
	virtual void Close();

	virtual void ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta);

	virtual void SetWindowPosition(MRect inBounds, bool inTransition);
	virtual void GetWindowPosition(MRect &outBounds) const;

	virtual void Invalidate(MRect inRect);
	virtual void Validate(MRect inRect);
	virtual void UpdateNow();

	virtual void ScrollRect(MRect inRect, int32_t inDeltaH, int32_t inDeltaV);

	virtual bool GetMouse(int32_t &outX, int32_t &outY, uint32_t &outModifiers);
	virtual bool WaitMouseMoved(int32_t inX, int32_t inY);

	virtual void SetCursor(MCursor inCursor);
	virtual void ObscureCursor();

	virtual void ConvertToScreen(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromScreen(int32_t &ioX, int32_t &ioY) const;

	virtual uint32_t GetModifiers() const;

	MWindow *GetWindow() const { return mWindow; }

	virtual MHandler *GetFocus();

  protected:
	virtual bool DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, const std::string &inText);

	virtual bool OnDestroy();
	virtual bool OnDelete(GdkEvent *inEvent);

	bool ChildFocus(GdkEventFocus *inEvent);
	MSlot<bool(GdkEventFocus *)> mChildFocus;

	bool OnMapEvent(GdkEvent *inEvent);
	MSlot<bool(GdkEvent *)> mMapEvent;

	virtual bool OnConfigureEvent(GdkEventConfigure *inEvent);

	//	void			Changed();
	//	MSlot<void()>	mChanged;

	virtual void DoForEach(GtkWidget *inWidget);
	static void DoForEachCallBack(GtkWidget *inWidget, gpointer inUserData);

	MMenuBar *mMenubar;
	GtkWidget *mMainVBox;
	MGtkWidgetMixin *mFocus;
	bool mConfigured;

	static std::list<MWindow *> sRecycle;
};
