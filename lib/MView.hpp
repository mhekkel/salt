//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id$
	Copyright Maarten L. Hekkelman
	Created 28-09-07 11:15:18
*/

#pragma once

#include <list>
#include <vector>

#include <cairo/cairo.h>

#include "MP2PEvents.hpp"

class MWindow;
class MDevice;
class MView;
class MViewScroller;
class MScrollbar;
class MHandler;

typedef std::list<MView*> MViewList;

enum MCursor
{
	eNormalCursor,
	eIBeamCursor,
	eRightCursor,
	
	eBlankCursor,
	
	eCursorCount
};

enum MControlPacking { ePackStart, ePackEnd };

/**
 * MView's have bounds and frames. The frame is the outer rectangle
 * in parent coordinates that encloses all of the MView, including
 * its margins. The bounds are in coordinates relative to the top/
 * left of the frame. When setting the frame, the bounds are
 * calculated based on the frame's dimensions and the margins.
 *
 * MViewScroller scrolls a MView by making the bounds's dimension
 * the same as the visible area inside the scroller. The bounds
 * are then moved inside the frame.
*/

class MView
{
  public:
	  friend class MViewScroller;

					MView(const std::string& inID, MRect inBounds);
	virtual			~MView();

	std::string		GetID() const						{ return mID; }

	virtual MView*	GetParent() const;
	virtual const MViewList&
					GetChildren() const					{ return mChildren; }

	virtual void	SetParent(MView* inParent);
	virtual void	AddChild(MView* inChild);
	virtual void	RemoveChild(MView* inChild);
	virtual void	AddedToWindow();
	virtual MWindow*
					GetWindow() const;
	virtual void	SetViewScroller(MViewScroller* inScroller);
	virtual void	GetBounds(MRect& outBounds) const;
	virtual void	GetFrame(MRect& outFrame) const;
	virtual void	SetFrame(const MRect& inFrame);
	virtual void	MoveFrame(int32 inXDelta, int32 inYDelta);
	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);
	virtual void	GetBindings(bool& outFollowLeft, bool& outFollowTop, bool& outFollowRight, bool& outFollowBottom) const;
	virtual void	SetBindings(bool inFollowLeft, bool inFollowTop, bool inFollowRight, bool inFollowBottom);
	bool			WidthResizable() const				{ return mBindLeft and mBindRight; }
	bool			HeightResizable() const				{ return mBindTop and mBindBottom; }

	virtual void	GetMargins(int32& outLeftMargin, int32& outTopMargin, int32& outRightMargin, int32& outBottomMargin) const;
	virtual void	SetMargins(int32 inLeftMargin, int32 inTopMargin, int32 inRightMargin, int32 inBottomMargin);

	// used in automatic layout
	virtual void	RecalculateLayout();
	virtual void	ChildResized();
	virtual void	GetScrollUnit(int32& outScrollUnitX, int32& outScrollUnitY) const;
	virtual void	SetScrollUnit(int32 inScrollUnitX, int32 inScrollUnitY);
	virtual bool	ActivateOnClick(int32 inX, int32 inY, uint32 inModifiers);
	virtual void	TrackMouse(bool inTrackMove, bool inTrackExit);
	virtual void	MouseDown(int32 inX, int32 inY, uint32 inClickCount, uint32 inModifiers);
	virtual void	MouseMove(int32 inX, int32 inY, uint32 inModifiers);
	virtual void	MouseExit();
	virtual void	MouseUp(int32 inX, int32 inY, uint32 inModifiers);
	virtual void	MouseWheel(int32 inX, int32 inY, int32 inDeltaX, int32 inDeltaY, uint32 inModifiers);
	virtual void	ShowContextMenu(int32 inX, int32 inY);
	virtual void	RedrawAll(MRect inUpdate);
	// virtual void	Draw(MRect inUpdate);
	virtual void	Draw(cairo_t* inCairo);

	virtual void	Activate();
	virtual void	Deactivate();
	bool			IsActive() const;

	virtual void	Enable();
	virtual void	Disable();
	bool			IsEnabled() const;

	virtual void	Show();
	virtual void	Hide();
	bool			IsVisible() const;

	virtual void	Invalidate();
	virtual void	Invalidate(MRect inRect);

	MEventOut<void()>	eScrolled;

	virtual void	ScrollBy(int32 inDeltaX, int32 inDeltaY);
	virtual void	ScrollTo(int32 inX, int32 inY);
	virtual void	GetScrollPosition(int32& outX, int32& outY) const;
	virtual void	ScrollRect(MRect inRect, int32 inX, int32 inY);
	virtual void	UpdateNow();
	virtual void	AdjustCursor(int32 inX, int32 inY, uint32 inModifiers);
	virtual void	SetCursor(MCursor inCursor);
	virtual void	ObscureCursor();
	virtual void	GetMouse(int32& outX, int32& outY, uint32& outModifiers) const;
	uint32			GetModifiers() const;

	// called for printing
	virtual uint32	CountPages(MDevice& inDevice);
	MView*			FindSubView(int32 inX, int32 inY) const;
	virtual MView*	FindSubViewByID(const std::string& inID) const;
	
	virtual MHandler*
					FindFocus();

	virtual void	ConvertToParent(int32& ioX, int32& ioY) const;
	virtual void	ConvertFromParent(int32& ioX, int32& ioY) const;
	virtual void	ConvertToWindow(int32& ioX, int32& ioY) const;
	virtual void	ConvertFromWindow(int32& ioX, int32& ioY) const;
	virtual void	ConvertToScreen(int32& ioX, int32& ioY) const;
	virtual void	ConvertFromScreen(int32& ioX, int32& ioY) const;
	
	// Used in X only
	virtual bool	PastePrimaryBuffer(const std::string& inText);

  protected:

	void			SuperActivate();
	virtual void	ActivateSelf();
	void			SuperDeactivate();
	virtual void	DeactivateSelf();

	void			SuperEnable();
	virtual void	EnableSelf();
	void			SuperDisable();
	virtual void	DisableSelf();

	void			SuperShow();
	virtual void	ShowSelf();
	void			SuperHide();
	virtual void	HideSelf();

	std::string		mID;
	MRect			mBounds;
	MRect			mFrame;
	int32			mLeftMargin, mTopMargin, mRightMargin, mBottomMargin;
	bool			mBindLeft, mBindTop, mBindRight, mBindBottom;
	MView*			mParent;
	MViewScroller*	mScroller;
	MViewList		mChildren;
	bool			mWillDraw;
	MTriState		mActive;
	MTriState		mVisible;
	MTriState		mEnabled;
};

class MHBox : public MView
{
  public:
					MHBox(const std::string& inID, MRect inBounds, uint32 inSpacing)
						: MView(inID, inBounds)
						, mSpacing(inSpacing) {}
//
//	virtual void	AddChild(// MView* inChild);
	
	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);

	virtual void	SetSpacing(int32 inSpacing)				{ mSpacing = inSpacing; }
	virtual void	SetLeftMargin(int32 inMargin)			{ mLeftMargin = inMargin; }
	virtual void	SetRightMargin(int32 inMargin)			{ mRightMargin = inMargin; }

  protected:

	virtual void	RecalculateLayout();

	uint32			mSpacing;
};

class MVBox : public MView
{
  public:
					MVBox(const std::string& inID, MRect inBounds, uint32 inSpacing)
						: MView(inID, inBounds)
						, mSpacing(inSpacing) {}

//	virtual void	AddChild(// MView* inChild);

	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);

	virtual void	SetSpacing(int32 inSpacing)				{ mSpacing = inSpacing; }
	virtual void	SetTopMargin(int32 inMargin)			{ mTopMargin = inMargin; }
	virtual void	SetBottomMargin(int32 inMargin)			{ mBottomMargin = inMargin; }

  protected:

	virtual void	RecalculateLayout();

	uint32			mSpacing;
};

class MTable : public MView
{
  public:
					MTable(const std::string& inID, MRect inBounds,
						MView* inChildren[],
						uint32 inColumns, uint32 inRows,
						int32 inHSpacing, int32 inVSpacing);

	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);

	using MView::AddChild;
	virtual void	AddChild(MView* inView, uint32 inColumn, uint32 inRow, int32 inColumnSpan = 1, int32 inRowSpan = 1);

  private:

	virtual void	RecalculateLayout();

	uint32			mColumns, mRows;
	int32			mHSpacing, mVSpacing;
	std::vector<MView*>
					mGrid;
};

class MViewScroller : public MView
{
public:
					MViewScroller(const std::string& inID, MView* inTarget,
						bool inHScrollbar, bool inVScrollbar);

	virtual void	AdjustScrollbars();

	MScrollbar*		GetHScrollbar() const					{ return mHScrollbar; }
	MScrollbar*		GetVScrollbar() const					{ return mVScrollbar; }

	virtual void	MoveFrame(int32 inXDelta, int32 inYDelta);

	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);

	void			SetTargetScrollUnit(int32 inScrollUnitX, int32 inScrollUnitY);

	void			GetTargetScrollUnit(int32& outScrollUnitX, int32& outScrollUnitY) const;

	void			GetTargetMinimalDimensions(int32& outMinWidth, int32& outMinHeight) const;

	virtual void	MouseWheel(int32 inX, int32 inY, int32 inDeltaX, int32 inDeltaY, uint32 inModifiers);

protected:

	MView*			mTarget;
	MScrollbar*		mHScrollbar;
	MScrollbar*		mVScrollbar;
	int32			mScrollUnitX, mScrollUnitY;

	virtual void	VScroll(MScrollMessage inScrollMsg);
	virtual void	HScroll(MScrollMessage inScrollMsg);

	MEventIn<void(MScrollMessage)> eVScroll;
	MEventIn<void(MScrollMessage)> eHScroll;
};

// --------------------------------------------------------------------

class MPager : public MView
{
  public:
					MPager(const std::string& inID, MRect inBounds);

	void			AddPage(MView* inPage);
	void			SelectPage(uint32 inPage);
	virtual void	RecalculateLayout();
};
