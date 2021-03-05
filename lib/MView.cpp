//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id$
	Copyright Drs M.L. Hekkelman
	Created 28-09-07 11:18:30
*/

#include "MLib.hpp"

#include <iostream>
#include <cassert>

#include "MWindow.hpp"
#include "MError.hpp"
#include "MUtils.hpp"
#include "MDevice.hpp"
#include "MControls.hpp"
#include "MHandler.hpp"

using namespace std;

MView::MView(
	const string&	inID,
	MRect			inBounds)
	: mID(inID)
	, mBounds(0, 0, inBounds.width, inBounds.height)
	, mFrame(inBounds)
	, mLeftMargin(0)
	, mTopMargin(0)
	, mRightMargin(0)
	, mBottomMargin(0)
	, mBindLeft(true)
	, mBindTop(true)
	, mBindRight(false)
	, mBindBottom(false)
	, mParent(nullptr)
	, mScroller(nullptr)
	, mWillDraw(false)
	, mActive(eTriStateOn)
	, mVisible(eTriStateLatent)
	, mEnabled(eTriStateOn)
{
}

MView::~MView()
{
	while (mChildren.size() > 0)
	{
		MView* v = mChildren.back();
		v->mParent = nullptr;
		mChildren.pop_back();
		delete v;
	}

	if (mParent != nullptr)
		mParent->RemoveChild(this);
}

MView* MView::GetParent() const
{
	return mParent;
}

void MView::SetParent(
	MView*			inParent)
{
	mParent = inParent;
}

void MView::AddChild(MView* inView)
{
	MRect frame(mFrame);

	mChildren.push_back(inView);
	inView->mParent = this;

	if (mEnabled == eTriStateOn)
		inView->SuperEnable();
	else
		inView->SuperDisable();

	if (mActive == eTriStateOn)
		inView->SuperActivate();
	else
		inView->SuperDeactivate();

	if (mVisible == eTriStateOn)
		inView->SuperShow();
	else
		inView->SuperHide();
	
	if (GetWindow() != nullptr)
		inView->AddedToWindow();

	RecalculateLayout();
	
	if (frame != mFrame and mParent != nullptr)
		mParent->ChildResized();
}

void MView::AddedToWindow()
{
	MHandler* thisHandler = dynamic_cast<MHandler*>(this);
	
	if (thisHandler != nullptr and thisHandler->GetSuper() == nullptr)
	{
		for (MView* parent = GetParent(); parent != nullptr; parent = parent->GetParent())
		{
			MHandler* super = dynamic_cast<MHandler*>(parent);
			if (super != nullptr)
			{
				thisHandler->SetSuper(super);
				break;
			}
		}
	}
	
	if (mVisible == eTriStateOn and not GetParent()->IsVisible())
		mVisible = eTriStateLatent;

	for (MView* child: mChildren)
		child->AddedToWindow();
}

void MView::RemoveChild(MView* inView)
{
	MViewList::iterator i = std::find(mChildren.begin(), mChildren.end(), inView);
	if (i != mChildren.end())
	{
		MView* child = *i;
		mChildren.erase(i);
		child->mParent = nullptr;
	}
}

MWindow* MView::GetWindow() const
{
	MWindow* result = nullptr;
	if (mParent != nullptr)
		result = mParent->GetWindow();
	return result;
}

void MView::SetViewScroller(
	MViewScroller*	inScroller)
{
	mScroller = inScroller;
}

void MView::GetBounds(
	MRect&			outBounds) const
{
	outBounds = mBounds;
}

void MView::GetFrame(
	MRect&			outFrame) const
{
	outFrame = mFrame;
}

void MView::SetFrame(
	const MRect&	inFrame)
{
	if (inFrame != mFrame)
	{
		mFrame = inFrame;

		if (mScroller != nullptr)
		{
			int32 minWidth, minHeight;
			mScroller->GetTargetMinimalDimensions(minWidth, minHeight);
		
			if (mFrame.width < minWidth)
				mFrame.width = minWidth;
		
			if (mFrame.height < minHeight)
				mFrame.height = minHeight;
		}

		mBounds.x = mLeftMargin;
		mBounds.y = mTopMargin;
		mBounds.width = mFrame.width - mLeftMargin - mRightMargin;
		mBounds.height = mFrame.height - mTopMargin - mBottomMargin;
		
		if (mScroller != nullptr)
			mScroller->AdjustScrollbars();
	}
}

void MView::GetBindings(
	bool&			outFollowLeft,
	bool&			outFollowTop,
	bool&			outFollowRight,
	bool&			outFollowBottom) const
{
	outFollowLeft = mBindLeft;
	outFollowTop = mBindTop;
	outFollowRight = mBindRight;
	outFollowBottom = mBindBottom;
}

void MView::SetBindings(
	bool			inFollowLeft,
	bool			inFollowTop,
	bool			inFollowRight,
	bool			inFollowBottom)
{
	mBindLeft = inFollowLeft;
	mBindTop = inFollowTop;
	mBindRight = inFollowRight;
	mBindBottom = inFollowBottom;
}

void MView::MoveFrame(
	int32			inXDelta,
	int32			inYDelta)
{
	if (mWillDraw)
		Invalidate();

	mFrame.x += inXDelta;
	mFrame.y += inYDelta;

	for (MView* child: mChildren)
		child->MoveFrame(0, 0);

	if (mScroller != nullptr)
		mScroller->AdjustScrollbars();

	if (mWillDraw)
		Invalidate();
}

void MView::ResizeFrame(
	int32			inWidthDelta,
	int32			inHeightDelta)
{
	if (mWillDraw)
		Invalidate();

	mFrame.width += inWidthDelta;
	mFrame.height += inHeightDelta;

	if (mScroller == nullptr)
	{
		mBounds.width += inWidthDelta;
		mBounds.height += inHeightDelta;
	}
	else
	{
		int32 minWidth, minHeight;
		mScroller->GetTargetMinimalDimensions(minWidth, minHeight);
		
		if (mFrame.width < minWidth)
			mFrame.width = minWidth;
		
		if (mFrame.height < minHeight)
			mFrame.height = minHeight;
		
		mScroller->AdjustScrollbars();
	}

	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;
		
		int32 dx = 0, dy = 0, dw = 0, dh = 0;
		
		if (child->mBindRight)
		{
			if (child->mBindLeft)
				dw = inWidthDelta;
			else
				dx = inWidthDelta;
		}
		
		if (child->mBindBottom)
		{
			if (child->mBindTop)
				dh = inHeightDelta;
			else
				dy = inHeightDelta;
		}

		if (dx or dy)
			child->MoveFrame(dx, dy);
		child->ResizeFrame(dw, dh);
	}

	if (mWillDraw)
		Invalidate();
}

void MView::GetMargins(
	int32&	outLeftMargin,
	int32&	outTopMargin,
	int32&	outRightMargin,
	int32&	outBottomMargin) const
{
	outLeftMargin = mLeftMargin;
	outTopMargin = mTopMargin;
	outRightMargin = mRightMargin;
	outBottomMargin = mBottomMargin;
}

void MView::SetMargins(
	int32	inLeftMargin,
	int32	inTopMargin,
	int32	inRightMargin,
	int32	inBottomMargin)
{
	int32 dx = inLeftMargin - mLeftMargin;
	int32 dy = inTopMargin - mTopMargin;
	
	//int32 dw = (mLeftMargin + mRightMargin) - (inLeftMargin + inRightMargin);
	//int32 dh = (mTopMargin + mBottomMargin) - (inTopMargin + inBottomMargin);
	
	mLeftMargin = inLeftMargin;
	mTopMargin = inTopMargin;
	mRightMargin = inRightMargin;
	mBottomMargin = inBottomMargin;

	mBounds.x = mLeftMargin;
	mBounds.y = mTopMargin;
	
	mFrame.width = mBounds.width + mLeftMargin + mRightMargin;
	mFrame.height = mBounds.height + mTopMargin + mBottomMargin;
	
	for (MView* child: mChildren)
		child->MoveFrame(dx, dy);

	//for (MView* child: mChildren)
	//	child->ResizeFrame(dw, dh);
}

void MView::RecalculateLayout()
{
	MRect b = mBounds;

	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;

		MRect f;
		child->GetFrame(f);
		b |= f;
	}
	
	mFrame.width = b.width + mLeftMargin + mRightMargin;
	mFrame.height = b.height + mTopMargin + mBottomMargin;
}

void MView::ChildResized()
{
	MRect frame(mFrame);
	RecalculateLayout();
	if (frame != mFrame and mParent != nullptr)
		mParent->ChildResized();
}

void MView::Invalidate()
{
	Invalidate(mBounds);
}

void MView::Invalidate(
	MRect		inRect)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inRect.x, inRect.y);
		inRect &= mFrame;
		if (inRect)
			mParent->Invalidate(inRect);
	}
}

void MView::ScrollBy(int32 inDeltaX, int32 inDeltaY)
{
	int32 x, y;
	GetScrollPosition(x, y);
	
	MScrollbar* scrollbar = mScroller ? mScroller->GetHScrollbar() : nullptr;
	if (inDeltaX and scrollbar != nullptr)
	{
		if (x + inDeltaX < scrollbar->GetMinValue())
			inDeltaX = scrollbar->GetMinValue() - x;
		if (x + inDeltaX > scrollbar->GetMaxValue())
			inDeltaX = scrollbar->GetMaxValue() - x;
	}

	scrollbar = mScroller ? mScroller->GetVScrollbar() : nullptr;
	if (inDeltaY and scrollbar != nullptr)
	{
		if (y + inDeltaY < scrollbar->GetMinValue())
			inDeltaY = scrollbar->GetMinValue() - y;
		if (y + inDeltaY > scrollbar->GetMaxValue())
			inDeltaY = scrollbar->GetMaxValue() - y;
	}
	
	if (inDeltaX != 0 or inDeltaY != 0)
	{
		ScrollRect(mBounds, -inDeltaX, -inDeltaY);

		mBounds.x += inDeltaX;
		mBounds.y += inDeltaY;

		if (mScroller != nullptr)
			mScroller->AdjustScrollbars();

		eScrolled();
	}
}

void MView::ScrollTo(int32 inX, int32 inY)
{
	ScrollBy(inX - mBounds.x, inY - mBounds.y);
}

void MView::GetScrollPosition(int32& outX, int32& outY) const
{
	outX = mBounds.x;
	outY = mBounds.y;
}

void MView::GetScrollUnit(
	int32&			outScrollUnitX,
	int32&			outScrollUnitY) const
{
	if (mScroller != nullptr)
		mScroller->GetTargetScrollUnit(outScrollUnitX, outScrollUnitY);
	else if (mParent != nullptr)
		mParent->GetScrollUnit(outScrollUnitX, outScrollUnitY);
	else
		outScrollUnitX = outScrollUnitY = 1;
}

void MView::SetScrollUnit(
	int32			inScrollUnitX,
	int32			inScrollUnitY)
{
	if (mScroller != nullptr)
		mScroller->SetTargetScrollUnit(inScrollUnitX, inScrollUnitY);
	else if (mParent != nullptr)
		mParent->SetScrollUnit(inScrollUnitX, inScrollUnitY);
}

void MView::ScrollRect(
	MRect			inRect,
	int32			inX,
	int32			inY)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inRect.x, inRect.y);
		inRect &= mFrame;
		if (inRect)
			mParent->ScrollRect(inRect, inX, inY);
	}
}

void MView::UpdateNow()
{
	if (mParent != nullptr)
		mParent->UpdateNow();
}

void MView::SetCursor(
	MCursor			inCursor)
{
	if (mParent != nullptr)
		mParent->SetCursor(inCursor);
}

void MView::AdjustCursor(
	int32			inX,
	int32			inY,
	uint32			inModifiers)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inX, inY);
		mParent->AdjustCursor(inX, inY, inModifiers);
	}
	else
		SetCursor(eNormalCursor);
}

void MView::ObscureCursor()
{
	if (mParent != nullptr)
		mParent->ObscureCursor();
}

bool MView::ActivateOnClick(int32 inX, int32 inY, uint32 inModifiers)
{
	return true;
}

void MView::TrackMouse(bool inTrackMove, bool inTrackExit)
{
}

void MView::MouseDown(
	int32			inX,
	int32			inY,
	uint32			inClickCount,
	uint32			inModifiers)
{
}

void MView::MouseMove(
	int32			inX,
	int32			inY,
	uint32			inModifiers)
{
}

void MView::MouseExit()
{
}

void MView::MouseUp(
	int32			inX,
	int32			inY,
	uint32			inModifiers)
{
}

void MView::MouseWheel(
	int32			inX,
	int32			inY,
	int32			inDeltaX,
	int32			inDeltaY,
	uint32			inModifiers)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inX, inY);
		mParent->MouseWheel(inX, inY, inDeltaX, inDeltaY, inModifiers);	
	}
}

void MView::ShowContextMenu(
	int32			inX,
	int32			inY)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inX, inY);
		mParent->ShowContextMenu(inX, inY);
	}
}

void MView::Show()
{
	if (mVisible != eTriStateOn)
	{
		if (mParent and mParent->IsVisible())
		{
			mVisible = eTriStateOn;
			Invalidate();
			ShowSelf();
		}
		else
			mVisible = eTriStateLatent;
	}

	if (mVisible == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperShow();
		
		if (mParent != nullptr)
			mParent->ChildResized();
	}
}

void MView::SuperShow()
{
	if (mVisible == eTriStateLatent)
	{
		mVisible = eTriStateOn;
		ShowSelf();
	}

	if (mVisible == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperShow();
	}
}

void MView::ShowSelf()
{
}

void MView::Hide()
{
	if (mVisible == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperHide();
	}

	if (mVisible != eTriStateOff)
	{
		Invalidate();

		bool wasVisible = (mVisible != eTriStateOff);
		mVisible = eTriStateOff;
		if (wasVisible)
		{
			HideSelf();
		
			if (mParent != nullptr)
				mParent->ChildResized();
		}
	}
}

void MView::SuperHide()
{
	if (mVisible == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperHide();

		mVisible = eTriStateLatent;
		HideSelf();
	}
}

void MView::HideSelf()
{
}

bool MView::IsVisible() const
{
	return mVisible == eTriStateOn;
}

void MView::Enable()
{
	if (mEnabled == eTriStateOff)
	{
		if (mParent != nullptr and mParent->mEnabled == eTriStateOn)
			mEnabled = eTriStateOn;
		else
			mEnabled = eTriStateLatent;
	}
	
	if (mEnabled == eTriStateOn)
	{
		EnableSelf();
		for (MView* child: mChildren)
			child->SuperEnable();
	}
}

void MView::SuperEnable()
{
	if (mEnabled == eTriStateLatent)
	{
		mEnabled = eTriStateOn;
		EnableSelf();
	}
	
	if (mEnabled == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperEnable();
	}
}

void MView::EnableSelf()
{
}

void MView::Disable()
{
	if (mEnabled == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperDisable();
	}

	// bool wasEnabled = (mEnabled != eTriStateOff);
	mEnabled = eTriStateOff;
	//if (wasEnabled)
		DisableSelf();
}

void MView::SuperDisable()
{
	if (mEnabled == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperDisable();

		mEnabled = eTriStateLatent;
		DisableSelf();
	}
}

void MView::DisableSelf()
{
}

bool MView::IsEnabled() const
{
	return (mEnabled == eTriStateOn) and IsVisible();
}

void MView::Activate()
{
	if (mActive == eTriStateOff)
	{
		if (mParent != nullptr and mParent->mActive == eTriStateOn)
		{
			mActive = eTriStateOn;
			ActivateSelf();
		}
		else
			mActive = eTriStateLatent;
	}

	if (mActive == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperActivate();
	}
}

void MView::SuperActivate()
{	
	if (mActive == eTriStateLatent)
	{
		mActive = eTriStateOn;
		ActivateSelf();
	}
	
	if (mActive == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperActivate();
	}
}

void MView::ActivateSelf()
{
}

void MView::Deactivate()
{
	if (mActive == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperDeactivate();
	}

	bool wasActive = (mActive == eTriStateOn);
	mActive = eTriStateOff;
	if (wasActive)
		DeactivateSelf();
}

void MView::SuperDeactivate()
{
	if (mActive == eTriStateOn)
	{
		for (MView* child: mChildren)
			child->SuperDeactivate();

		mActive = eTriStateLatent;
		DeactivateSelf();
	}
}

void MView::DeactivateSelf()
{
}

bool MView::IsActive() const
{
	return (mActive == eTriStateOn) and IsVisible();
}

void MView::GetMouse(
	int32&			outX,
	int32&			outY,
	uint32&			outModifiers) const
{
	if (mParent != nullptr)
	{
		mParent->GetMouse(outX, outY, outModifiers);
		ConvertFromParent(outX, outY);
	}
}

uint32 MView::GetModifiers() const
{
	uint32 result = 0;

	MWindow* w = GetWindow();
	if (w != nullptr)
		result = w->GetModifiers();
	
	return result;
}

uint32 MView::CountPages(
	MDevice&		inDevice)
{
	return 1;
}

MHandler* MView::FindFocus()
{
	MHandler* result = nullptr;
	
	for (MView* view: mChildren)
	{
		result = view->FindFocus();
		if (result != nullptr)
			break;
	}
	
	return result;
}

MView* MView::FindSubView(int32 inX, int32 inY) const
{
	const MView* result = this;

	ConvertFromParent(inX, inY);

	for (MView* view: mChildren)
	{
		if (view->IsVisible() and view->mFrame.ContainsPoint(inX, inY))
		{
			result = view->FindSubView(inX, inY);
			break;
		}
	}
	
	return const_cast<MView*>(result);
}

MView* MView::FindSubViewByID(const string& inID) const
{
	const MView* result = nullptr;
	
	if (mID == inID)
		result = this;
	else
	{
		for (MView* view: mChildren)
		{
			result = view->FindSubViewByID(inID);
			if (result != nullptr)
				break;
		}
	}
	
	return const_cast<MView*>(result);
}

void MView::ConvertToParent(int32& ioX, int32& ioY) const
{
	ioX += mFrame.x - mBounds.x + mLeftMargin;
	ioY += mFrame.y - mBounds.y + mTopMargin;
}

void MView::ConvertFromParent(int32& ioX, int32& ioY) const
{
	ioX -= mFrame.x - mBounds.x + mLeftMargin;
	ioY -= mFrame.y - mBounds.y + mTopMargin;
}

void MView::ConvertToWindow(int32& ioX, int32& ioY) const
{
	ConvertToParent(ioX, ioY);
	if (mParent != nullptr)
		mParent->ConvertToWindow(ioX, ioY);
}

void MView::ConvertFromWindow(int32& ioX, int32& ioY) const
{
	if (mParent != nullptr)
		mParent->ConvertFromWindow(ioX, ioY);
	ConvertFromParent(ioX, ioY);
}

void MView::ConvertToScreen(int32& ioX, int32& ioY) const
{
	ConvertToParent(ioX, ioY);
	if (mParent != nullptr)
		mParent->ConvertToScreen(ioX, ioY);
}

void MView::ConvertFromScreen(int32& ioX, int32& ioY) const
{
	if (mParent != nullptr)
		mParent->ConvertFromScreen(ioX, ioY);
	ConvertFromParent(ioX, ioY);
}

void MView::RedrawAll(
	MRect			inUpdate)
{
	assert(false);

	inUpdate &= mBounds;

	if (not IsVisible() or not inUpdate)
		return;

	// if (mWillDraw)
	// 	Draw(inUpdate);

	for (MView* child: mChildren)
	{
		MRect r = inUpdate;

		child->ConvertFromParent(r.x, r.y);
		child->RedrawAll(r);
	}
}

void MView::Draw(cairo_t* inCairo)
{
	// do nothing
}

bool MView::PastePrimaryBuffer(const string& inText)
{
	return false;
}

// --------------------------------------------------------------------

void MHBox::RecalculateLayout()
{
	mBounds.x = mLeftMargin;
	mBounds.y = mTopMargin;
	mBounds.width = mBounds.height = 0;
	
	int32 x = mBounds.x, y = mBounds.y;
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;
		
		MRect f;
		child->GetFrame(f);
		if (mBounds.height < f.height)
			mBounds.height = f.height;
		
		if (f.x != x or f.y != y)
			child->MoveFrame(x - f.x, y - f.y);
		
		x += f.width + mSpacing;
	}
	
	mBounds.width = x - mSpacing - mLeftMargin;
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;
		
		MRect f;
		child->GetFrame(f);
		if (f.height != mBounds.height)
		{
			if (child->HeightResizable())
				child->ResizeFrame(0, mBounds.height - f.height);
			else if (f.y != y)
			{
				int32 dy = y - f.y;
				if (f.height < mBounds.height)
					dy += (mBounds.height - f.height) / 2;
				child->MoveFrame(0, dy);
			}
		}
	}
	
	mFrame.width = mBounds.width + mLeftMargin + mRightMargin;
	mFrame.height = mBounds.height + mTopMargin + mBottomMargin;
}

void MHBox::ResizeFrame(
	int32			inWidthDelta,
	int32			inHeightDelta)
{
	mFrame.width += inWidthDelta;
	mFrame.height += inHeightDelta;

	mBounds.width += inWidthDelta;
	mBounds.height += inHeightDelta;
	
	uint32 n = 0;	// count the resizing children
	uint32 cw = 0;	// and the width of the non-resizing children
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;

		if (child->WidthResizable())
			++n;
		else
		{
			MRect frame;
			child->GetFrame(frame);
			cw += frame.width;
		}
	}
	
	// now re-layout the children
	uint32 w = mFrame.width - (mLeftMargin + mRightMargin + (mChildren.size() - 1) * mSpacing);	// the available width
	uint32 rw = 0;
	if (w > cw)
		rw = w - cw;		// the width to divide
	int32 x = mLeftMargin;
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;

		// move the child
		MRect frame;
		child->GetFrame(frame);
		child->MoveFrame(x - frame.x, 0);

		int32 dw = frame.width, dy = 0, dh = 0;
		bool bl, bt, br, bb;
		child->GetBindings(bl, bt, br, bb);

		if (bt)
		{
			if (bb)
				dh = inHeightDelta;
			else
				dy = inHeightDelta;
		}
		
		if (dy)
			child->MoveFrame(0, dy);
		
		// resize the child if needed
		if (child->WidthResizable())
		{
			dw = rw / n;
			child->ResizeFrame(dw - frame.width, dh);
		}
		else
			child->ResizeFrame(0, dh);
		
		x += dw + mSpacing;
	}

	Invalidate();
}

void MVBox::RecalculateLayout()
{
	mBounds.x = mLeftMargin;
	mBounds.y = mTopMargin;
	mBounds.width = mBounds.height = 0;
	
	int32 x = mBounds.x, y = mBounds.y;
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;
		
		MRect f;
		child->GetFrame(f);
		if (mBounds.width < f.width)
			mBounds.width = f.width;
		
		if (f.x != x or f.y != y)
			child->MoveFrame(x - f.x, y - f.y);
		
		y += f.height + mSpacing;
	}
	
	mBounds.height = y - mSpacing - mTopMargin;
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;
		
		MRect f;
		child->GetFrame(f);
		if (f.width != mBounds.width)
			child->ResizeFrame(mBounds.width - f.width, 0);
	}
	
	mFrame.width = mBounds.width + mLeftMargin + mRightMargin;
	mFrame.height = mBounds.height + mTopMargin + mBottomMargin;
}

void MVBox::ResizeFrame(
	int32			inWidthDelta,
	int32			inHeightDelta)
{
	mFrame.width += inWidthDelta;
	mFrame.height += inHeightDelta;

	mBounds.width += inWidthDelta;
	mBounds.height += inHeightDelta;
	
	uint32 n = 0;	// count the resizing children
	uint32 ch = 0;	// and the height of the non-resizing children
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;

		if (child->HeightResizable())
			++n;
		else
		{
			MRect frame;
			child->GetFrame(frame);
			ch += frame.height;
		}
	}
	
	// now re-layout the children
	uint32 h = mFrame.height - (mTopMargin + mBottomMargin + (mChildren.size() - 1) * mSpacing);	// the available height
	uint32 rh = 0;
	if (h > ch)
		rh = h - ch;		// the height to divide
	int32 y = mTopMargin;
	
	for (MView* child: mChildren)
	{
		if (not child->IsVisible())
			continue;

		// move the child
		MRect frame;
		child->GetFrame(frame);
		child->MoveFrame(0, y - frame.y);

		int32 dh = frame.height, dx = 0, dw = 0;
		bool bl, bt, br, bb;
		child->GetBindings(bl, bt, br, bb);

		if (br)
		{
			if (bl)
				dw = inWidthDelta;
			else
				dx = inWidthDelta;
		}
		
		if (dx)
			child->MoveFrame(dx, 0);
		
		// resize the child if needed
		if (child->HeightResizable())
		{
			dh = rh / n;
			child->ResizeFrame(dw, dh - frame.height);
		}
		else
			child->ResizeFrame(dw, 0);
		
		y += dh + mSpacing;
	}

	Invalidate();
}

MTable::MTable(const std::string& inID, MRect inBounds, MView* inChildren[],
	uint32 inColumns, uint32 inRows, int32 inHSpacing, int32 inVSpacing)
	: MView(inID, inBounds)
	, mColumns(inColumns)
	, mRows(inRows)
	, mHSpacing(inHSpacing)
	, mVSpacing(inVSpacing)
	, mGrid(mRows * mColumns)
{
	vector<int32> widths(mColumns);
	vector<int32> heights(mRows);
	
	for (uint32 r = 0; r < mRows; ++r)
	{
		for (uint32 c = 0; c < mColumns; ++c)
		{
			int ix = r * mColumns + c;
			
			//if (inChildren[ix] == nullptr or not inChildren[ix]->IsVisible())
			//	continue;

			mGrid[ix] = inChildren[ix];
			MView::AddChild(mGrid[ix]);
			
			MRect bounds;
			mGrid[ix]->GetBounds(bounds);
			
			if (widths[c] < bounds.width)
				widths[c] = bounds.width;
			
			if (heights[r] < bounds.height)
				heights[r] = bounds.height;
		}
	}
	
	int32 x, y = 0;
	
	for (uint32 r = 0; r < mRows; ++r)
	{
		x = 0;
	
		for (uint32 c = 0; c < mColumns; ++c)
		{
			int ix = r * mColumns + c;
	
			if (inChildren[ix] == nullptr or not inChildren[ix]->IsVisible())
				continue;
	
			MRect frame;
			mGrid[ix]->GetFrame(frame);
			
			// compensate (vertical center in row)
			int32 dy = y - frame.y;
			if (frame.height < heights[r])
				dy += (heights[r] - frame.height) / 2;
			
			mGrid[ix]->MoveFrame(x - frame.x, dy);
			mGrid[ix]->ResizeFrame(
				widths[c] - frame.width, heights[r] - frame.height);
			
			x += widths[c] + mHSpacing;
		}
		
		y += heights[r] + mVSpacing;
	}
	
	mBounds.width = mFrame.width = x - mHSpacing;
	mBounds.height = mFrame.height = y - mVSpacing;
}

void MTable::RecalculateLayout()
{
	mBounds.x = mLeftMargin;
	mBounds.y = mTopMargin;
	mBounds.width = mBounds.height = 0;

	int32 y = mBounds.y;

	// resize rows
	for (uint32 rx = 0; rx < mRows; ++rx)
	{
		int32 height = 0;
		
		for (uint32 cx = 0; cx < mColumns; ++cx)
		{
			MView* v = mGrid[rx * mColumns + cx];
			if (v == nullptr or not v->IsVisible())
				continue;
			
			MRect f;
			v->GetFrame(f);
			
			if (height < f.y + f.height)
				height = f.y + f.height;
		}

		for (uint32 cx = 0; cx < mColumns; ++cx)
		{
			MView* v = mGrid[rx * mColumns + cx];
			if (v == nullptr or not v->IsVisible() or v->HeightResizable() == false)
				continue;
			
			MRect f;
			v->GetFrame(f);
			
			if (f.y != y)
				v->MoveFrame(0, y - f.y);
			
			if (f.y + f.height < height)
				v->ResizeFrame(0, height - f.y - f.height);
		}
		
		if (height != 0)
		{
			mBounds.height = y + height - mTopMargin;
			y += height + mVSpacing;
		}
	}
	
	int32 x = mBounds.x;
	
	for (uint32 cx = 0; cx < mColumns; ++cx)
	{
		int32 width = 0;
		for (uint32 rx = 0; rx < mRows; ++rx)
		{
			MView* child = mGrid[rx * mColumns + cx];
			
			if (child == nullptr or not child->IsVisible())
				continue;
			
			MRect f;
			child->GetFrame(f);
			if (f.x != x)
				child->MoveFrame(x - f.x, 0);
			
			if (width < f.width)
				width = f.width;
		}

		if (width != 0)
		{
			mBounds.width = x + width;
			x += width + mHSpacing;
		}
	}
	
	mFrame.width = mBounds.width + mLeftMargin + mRightMargin;
	mFrame.height = mBounds.height + mTopMargin + mBottomMargin;
}

void MTable::ResizeFrame(
	int32			inWidthDelta,
	int32			inHeightDelta)
{
	mFrame.width += inWidthDelta;
	mFrame.height += inHeightDelta;

	mBounds.width += inWidthDelta;
	mBounds.height += inHeightDelta;
	
	// resize rows
	for (uint32 rx = 0; rx < mRows; ++rx)
	{
		int32 height = 0;
		
		for (uint32 cx = 0; cx < mColumns; ++cx)
		{
			MView* v = mGrid[rx * mColumns + cx];
			if (v == nullptr or not v->IsVisible())
				continue;
			
			MRect f;
			v->GetFrame(f);
			
			if (height < f.y + f.height)
				height = f.y + f.height;
		}

		for (uint32 cx = 0; cx < mColumns; ++cx)
		{
			MView* v = mGrid[rx * mColumns + cx];
			if (v == nullptr or not v->IsVisible() or v->HeightResizable() == false)
				continue;
			
			MRect f;
			v->GetFrame(f);
			
			if (f.y + f.height < height)
				v->ResizeFrame(0, height - f.y - f.height);
		}
	}

	// resize columns
	// first count the number of resizing columns
	
	if (inWidthDelta != 0)
	{
		uint32 n = 0;
		vector<bool> resizable(mColumns);

		for (uint32 cx = 0; cx < mColumns; ++cx)
		{
			for (uint32 rx = 0; rx < mRows; ++rx)
			{
				MView* child = mGrid[rx * mColumns + cx];

				if (child != nullptr and child->WidthResizable() and child->IsVisible())
				{
					resizable[cx] = true;
					++n;
					break;
				}
			}
		}
		
		if (n > 0)
		{
			for (uint32 cx = 0; cx < mColumns; ++cx)
			{
				int32 delta = inWidthDelta / n;
				int32 dx = 0;
				
				for (uint32 rx = 0; rx < mRows; ++rx)
				{
					MView* child = mGrid[rx * mColumns + cx];
					
					if (child == nullptr or not child->IsVisible())
						continue;
					
					child->MoveFrame(dx, 0);
					if (resizable[cx])
						child->ResizeFrame(delta, 0);
				}

				if (resizable[cx])
					dx += delta;
			}
		}
	}
}

void MTable::AddChild(
	MView*			inView,
	uint32			inColumn,
	uint32			inRow,
	int32			inColumnSpan,
	int32			inRowSpan)
{
	MView::AddChild(inView);
	
	if (inColumn > mColumns or inRow > mRows)
		THROW(("table index out of bounds"));
	
	mGrid[inRow * mColumns + inColumn] = inView;
	
	ResizeFrame(0, 0);
}

// --------------------------------------------------------------------

MViewScroller::MViewScroller(const string& inID,
		MView* inTarget, bool inHScrollbar, bool inVScrollbar)
	: MView(inID, MRect(0, 0, 0, 0))
	, mTarget(inTarget)
	, mHScrollbar(nullptr)
	, mVScrollbar(nullptr)
	, mScrollUnitX(1)
	, mScrollUnitY(1)
	, eVScroll(this, &MViewScroller::VScroll)
	, eHScroll(this, &MViewScroller::HScroll)
{
	MRect frame;
	mTarget->GetFrame(frame);	// our frame will be the frame of the target
	SetFrame(frame);
	
	MRect bounds(frame);
	bounds.x = bounds.y = 0;
	mBounds = bounds;

	if (inVScrollbar)
	{
		MRect r(mBounds.x + mBounds.width - kScrollbarWidth, mBounds.y,
			kScrollbarWidth, mBounds.height);
		if (inHScrollbar)
			r.height -= kScrollbarWidth;

//		mVScrollbar = new MScrollbar(GetID() + "-vscrollbar", r);
//		mVScrollbar->SetBindings(false, true, true, true);
//		mVScrollbar->SetValue(0);
//		AddChild(mVScrollbar);
//		AddRoute(mVScrollbar->eScroll, eVScroll);

		bounds.width -= kScrollbarWidth;
	}

	if (inHScrollbar)
	{
		MRect r(mBounds.x, mBounds.y + mBounds.height - kScrollbarWidth,
			mBounds.width, kScrollbarWidth);
		
		if (inVScrollbar)
			r.width -= kScrollbarWidth;

//		mHScrollbar = new MScrollbar(GetID() + "-hscrollbar", r);
//		AddChild(mHScrollbar);
//		mHScrollbar->SetBindings(true, false, true, true);
//		mHScrollbar->SetValue(0);
//		AddChild(mHScrollbar);
//		AddRoute(mHScrollbar->eScroll, eHScroll);

		bounds.height -= kScrollbarWidth;
	}

	mTarget->mBounds = bounds;

	mTarget->SetViewScroller(this);
	mTarget->GetBindings(mBindLeft, mBindTop, mBindRight, mBindBottom);
	mTarget->SetBindings(true, true, true, true);

	AddChild(mTarget);
}

void MViewScroller::MoveFrame(
	int32			inXDelta,
	int32			inYDelta)
{
	MView::MoveFrame(inXDelta, inYDelta);
	AdjustScrollbars();
}

void MViewScroller::ResizeFrame(
	int32			inWidthDelta,
	int32			inHeightDelta)
{
	mFrame.width += inWidthDelta;
	mFrame.height += inHeightDelta;

	mBounds.width += inWidthDelta;
	mBounds.height += inHeightDelta;
	
	int32 dx = 0, dy = 0;
	
	if (mHScrollbar != nullptr)
	{
		mHScrollbar->MoveFrame(0, inHeightDelta);
		mHScrollbar->ResizeFrame(inWidthDelta, 0);
	}
	else
		dx = inWidthDelta;
	
	if (mVScrollbar != nullptr)
	{
		mVScrollbar->MoveFrame(inWidthDelta, 0);
		mVScrollbar->ResizeFrame(0, inHeightDelta);
	}
	else
		dy = inHeightDelta;
	
	mTarget->mBounds.width += inWidthDelta;
	mTarget->mBounds.height += inHeightDelta;
	mTarget->ResizeFrame(dx, dy);
	
	mTarget->Invalidate();

	AdjustScrollbars();
}

void MViewScroller::AdjustScrollbars()
{
	int32 dx = 0, dy = 0;
	
	MRect targetFrame;
	mTarget->GetFrame(targetFrame);

	MRect targetBounds;
	mTarget->GetBounds(targetBounds);

	if (mHScrollbar != nullptr)
	{
		mHScrollbar->SetAdjustmentValues(0, targetFrame.width, mScrollUnitX,
			targetBounds.width, targetBounds.x);
		dx = mHScrollbar->GetValue() - targetBounds.x;
	}

	if (mVScrollbar != nullptr)
	{
		mVScrollbar->SetAdjustmentValues(0, targetFrame.height, mScrollUnitY,
			targetBounds.height, targetBounds.y);
		dy = mVScrollbar->GetValue() - targetBounds.y;
	}
	
	if (dx != 0 or dy != 0)
		mTarget->ScrollBy(dx, dy);
}

void MViewScroller::SetTargetScrollUnit(
	int32			inScrollUnitX,
	int32			inScrollUnitY)
{
	assert(inScrollUnitY > 0 and inScrollUnitX > 0);
//	if (inScrollUnitX < 1 or inScrollUnitY < 1)
//		THROW(("Scroll unit should be larger than one"));
	mScrollUnitX = inScrollUnitX;
	mScrollUnitY = inScrollUnitY;
}

void MViewScroller::GetTargetScrollUnit(
	int32&			outScrollUnitX,
	int32&			outScrollUnitY) const
{
	outScrollUnitX = mScrollUnitX;
	outScrollUnitY = mScrollUnitY;
}

void MViewScroller::GetTargetMinimalDimensions(
	int32&			outMinWidth,
	int32&			outMinHeight) const
{
	outMinWidth = mBounds.width;
	if (mVScrollbar)
		outMinWidth -= kScrollbarWidth;
	outMinHeight = mBounds.height;
	if (mHScrollbar)
		outMinHeight -= kScrollbarWidth;
}

void MViewScroller::VScroll(MScrollMessage inScrollMsg)
{
	MRect bounds;
	mTarget->GetBounds(bounds);

	int32 dy = 0;

	switch (inScrollMsg)
	{
		case kScrollLineUp:
			dy = -mScrollUnitY;
			break;

		case kScrollLineDown:
			dy = mScrollUnitY;
			break;

		case kScrollPageUp:
			dy = -bounds.height;
			break;

		case kScrollPageDown:
			dy = bounds.height;
			break;

		case kScrollToThumb:
			dy = mVScrollbar->GetTrackValue() - bounds.y;
			break;
		
		default:
			PRINT(("Unhandled scroll message %d", inScrollMsg));
			break;
	}

	if (dy != 0)
		mTarget->ScrollBy(0, dy);
}

void MViewScroller::HScroll(MScrollMessage inScrollMsg)
{
	MRect bounds;
	mTarget->GetBounds(bounds);

	int32 dx = 0;

	switch (inScrollMsg)
	{
		case kScrollLineUp:
			dx = -mScrollUnitX;
			break;

		case kScrollLineDown:
			dx = mScrollUnitX;
			break;

		case kScrollPageUp:
			dx = -bounds.height;
			break;

		case kScrollPageDown:
			dx = bounds.height;
			break;

		case kScrollToThumb:
			dx = mHScrollbar->GetTrackValue() - bounds.x;
			break;
		
		default:
			PRINT(("Unhandled scroll message %d", inScrollMsg));
			break;
	}

	if (dx != 0)
		mTarget->ScrollBy(dx, 0);
}

void MViewScroller::MouseWheel(
	int32			inX,
	int32			inY,
	int32			inDeltaX,
	int32			inDeltaY,
	uint32			inModifiers)
{
	const int32 kWheelAcceleration = 3;
	int32 dx = 0, dy = 0;

	if (mHScrollbar != nullptr)
		dx = -(inDeltaX * mScrollUnitX * kWheelAcceleration);
			
	if (mVScrollbar != nullptr)
		dy = -(inDeltaY * mScrollUnitY * kWheelAcceleration);

	if (dx != 0 or dy != 0)
		mTarget->ScrollBy(dx, dy);
}

// --------------------------------------------------------------------

MPager::MPager(const string& inID, MRect inBounds)
	: MView(inID, inBounds)
{
}

void MPager::AddPage(MView* inPage)
{
	AddChild(inPage);
}

void MPager::SelectPage(uint32 inPage)
{
	if (inPage < mChildren.size())
	{
		for (MView* page: mChildren)
		{
			if (inPage-- == 0)
			{
				page->Show();
				page->Enable();
			}
			else
			{
				page->Hide();
				page->Disable();
			}
		}
	}
}

void MPager::RecalculateLayout()
{
	MRect b = mBounds;

	for (MView* child: mChildren)
	{
		MRect f;
		child->GetFrame(f);
		b |= f;
	}
	
	mBounds.x = mLeftMargin;
	mBounds.y = mTopMargin;
	mBounds.width = b.width;
	mBounds.height = b.height;

	mFrame.width = b.width + mLeftMargin + mRightMargin;
	mFrame.height = b.height + mTopMargin + mBottomMargin;
}
