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

#include "MTypes.hpp"
#include "MP2PEvents.hpp"

class MWindow;
class MDevice;
class MView;
class MViewScroller;
class MScrollbar;
class MHandler;

typedef std::list<MView *> MViewList;

enum MCursor
{
	eNormalCursor,
	eIBeamCursor,
	eRightCursor,

	eBlankCursor,

	eCursorCount
};

enum MControlPacking
{
	ePackStart,
	ePackEnd
};

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

	MView(const std::string &inID, MRect inBounds);
	virtual ~MView();

	std::string GetID() const { return mID; }

	virtual MView *GetParent() const;
	virtual const MViewList &
	GetChildren() const { return mChildren; }

	virtual void SetParent(MView *inParent);
	virtual void AddChild(MView *inChild);
	virtual void RemoveChild(MView *inChild);
	virtual void AddedToWindow();
	virtual MWindow *
	GetWindow() const;
	virtual void SetViewScroller(MViewScroller *inScroller);
	virtual void GetBounds(MRect &outBounds) const;
	virtual void GetFrame(MRect &outFrame) const;
	virtual void SetFrame(const MRect &inFrame);
	virtual void MoveFrame(int32_t inXDelta, int32_t inYDelta);
	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);
	virtual void GetBindings(bool &outFollowLeft, bool &outFollowTop, bool &outFollowRight, bool &outFollowBottom) const;
	virtual void SetBindings(bool inFollowLeft, bool inFollowTop, bool inFollowRight, bool inFollowBottom);
	bool WidthResizable() const { return mBindLeft and mBindRight; }
	bool HeightResizable() const { return mBindTop and mBindBottom; }

	virtual void GetMargins(int32_t &outLeftMargin, int32_t &outTopMargin, int32_t &outRightMargin, int32_t &outBottomMargin) const;
	virtual void SetMargins(int32_t inLeftMargin, int32_t inTopMargin, int32_t inRightMargin, int32_t inBottomMargin);

	// used in automatic layout
	virtual void RecalculateLayout();
	virtual void ChildResized();
	virtual void GetScrollUnit(int32_t &outScrollUnitX, int32_t &outScrollUnitY) const;
	virtual void SetScrollUnit(int32_t inScrollUnitX, int32_t inScrollUnitY);
	virtual bool ActivateOnClick(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void TrackMouse(bool inTrackMove, bool inTrackExit);
	virtual void MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers);
	virtual void MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void MouseExit();
	virtual void MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void MouseWheel(int32_t inX, int32_t inY, int32_t inDeltaX, int32_t inDeltaY, uint32_t inModifiers);
	virtual void ShowContextMenu(int32_t inX, int32_t inY);
	virtual void RedrawAll(MRect inUpdate);
	// virtual void	Draw(MRect inUpdate);
	virtual void Draw();

	virtual void Activate();
	virtual void Deactivate();
	bool IsActive() const;

	virtual void Enable();
	virtual void Disable();
	bool IsEnabled() const;

	virtual void Show();
	virtual void Hide();
	bool IsVisible() const;

	virtual void Invalidate();
	virtual void Invalidate(MRect inRect);

	MEventOut<void()> eScrolled;

	virtual void ScrollBy(int32_t inDeltaX, int32_t inDeltaY);
	virtual void ScrollTo(int32_t inX, int32_t inY);
	virtual void GetScrollPosition(int32_t &outX, int32_t &outY) const;
	virtual void ScrollRect(MRect inRect, int32_t inX, int32_t inY);
	virtual void UpdateNow();
	virtual void AdjustCursor(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void SetCursor(MCursor inCursor);
	virtual void ObscureCursor();
	virtual void GetMouse(int32_t &outX, int32_t &outY, uint32_t &outModifiers) const;
	uint32_t GetModifiers() const;

	// called for printing
	virtual uint32_t CountPages(MDevice &inDevice);
	MView *FindSubView(int32_t inX, int32_t inY) const;
	virtual MView *FindSubViewByID(const std::string &inID) const;

	virtual MHandler *
	FindFocus();

	virtual void ConvertToParent(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromParent(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertToWindow(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromWindow(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertToScreen(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromScreen(int32_t &ioX, int32_t &ioY) const;

	// Used in X only
	virtual bool PastePrimaryBuffer(const std::string &inText);

  protected:
	void SuperActivate();
	virtual void ActivateSelf();
	void SuperDeactivate();
	virtual void DeactivateSelf();

	void SuperEnable();
	virtual void EnableSelf();
	void SuperDisable();
	virtual void DisableSelf();

	void SuperShow();
	virtual void ShowSelf();
	void SuperHide();
	virtual void HideSelf();

	std::string mID;
	MRect mBounds;
	MRect mFrame;
	int32_t mLeftMargin, mTopMargin, mRightMargin, mBottomMargin;
	bool mBindLeft, mBindTop, mBindRight, mBindBottom;
	MView *mParent;
	MViewScroller *mScroller;
	MViewList mChildren;
	bool mWillDraw;
	MTriState mActive;
	MTriState mVisible;
	MTriState mEnabled;
};

class MHBox : public MView
{
  public:
	MHBox(const std::string &inID, MRect inBounds, uint32_t inSpacing)
		: MView(inID, inBounds)
		, mSpacing(inSpacing)
	{
	}
	//
	//	virtual void	AddChild(// MView* inChild);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	virtual void SetSpacing(int32_t inSpacing) { mSpacing = inSpacing; }
	virtual void SetLeftMargin(int32_t inMargin) { mLeftMargin = inMargin; }
	virtual void SetRightMargin(int32_t inMargin) { mRightMargin = inMargin; }

  protected:
	virtual void RecalculateLayout();

	uint32_t mSpacing;
};

class MVBox : public MView
{
  public:
	MVBox(const std::string &inID, MRect inBounds, uint32_t inSpacing)
		: MView(inID, inBounds)
		, mSpacing(inSpacing)
	{
	}

	//	virtual void	AddChild(// MView* inChild);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	virtual void SetSpacing(int32_t inSpacing) { mSpacing = inSpacing; }
	virtual void SetTopMargin(int32_t inMargin) { mTopMargin = inMargin; }
	virtual void SetBottomMargin(int32_t inMargin) { mBottomMargin = inMargin; }

  protected:
	virtual void RecalculateLayout();

	uint32_t mSpacing;
};

class MTable : public MView
{
  public:
	MTable(const std::string &inID, MRect inBounds,
	       MView *inChildren[],
	       uint32_t inColumns, uint32_t inRows,
	       int32_t inHSpacing, int32_t inVSpacing);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	using MView::AddChild;
	virtual void AddChild(MView *inView, uint32_t inColumn, uint32_t inRow, int32_t inColumnSpan = 1, int32_t inRowSpan = 1);

  private:
	virtual void RecalculateLayout();

	uint32_t mColumns, mRows;
	int32_t mHSpacing, mVSpacing;
	std::vector<MView *>
		mGrid;
};

class MViewScroller : public MView
{
  public:
	MViewScroller(const std::string &inID, MView *inTarget,
	              bool inHScrollbar, bool inVScrollbar);

	virtual void AdjustScrollbars();

	MScrollbar *GetHScrollbar() const { return mHScrollbar; }
	MScrollbar *GetVScrollbar() const { return mVScrollbar; }

	virtual void MoveFrame(int32_t inXDelta, int32_t inYDelta);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	void SetTargetScrollUnit(int32_t inScrollUnitX, int32_t inScrollUnitY);

	void GetTargetScrollUnit(int32_t &outScrollUnitX, int32_t &outScrollUnitY) const;

	void GetTargetMinimalDimensions(int32_t &outMinWidth, int32_t &outMinHeight) const;

	virtual void MouseWheel(int32_t inX, int32_t inY, int32_t inDeltaX, int32_t inDeltaY, uint32_t inModifiers);

  protected:
	MView *mTarget;
	MScrollbar *mHScrollbar;
	MScrollbar *mVScrollbar;
	int32_t mScrollUnitX, mScrollUnitY;

	virtual void VScroll(MScrollMessage inScrollMsg);
	virtual void HScroll(MScrollMessage inScrollMsg);

	MEventIn<void(MScrollMessage)> eVScroll;
	MEventIn<void(MScrollMessage)> eHScroll;
};

// --------------------------------------------------------------------

class MPager : public MView
{
  public:
	MPager(const std::string &inID, MRect inBounds);

	void AddPage(MView *inPage);
	void SelectPage(uint32_t inPage);
	virtual void RecalculateLayout();
};
