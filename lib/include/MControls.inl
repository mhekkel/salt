//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

template<class IMPL>
MControl<IMPL>::MControl(const std::string& inID, MRect inBounds, IMPL* inImpl)
	: MControlBase(inID, inBounds)
	, mImpl(inImpl)
{
}

template<class IMPL>
MControl<IMPL>::~MControl()
{
	delete mImpl;
}

template<class IMPL>
bool MControl<IMPL>::IsFocus() const
{
	return mImpl->IsFocus();
}

template<class IMPL>
void MControl<IMPL>::SetFocus()
{
	mImpl->SetFocus();
}

template<class IMPL>
MControlImplBase* MControl<IMPL>::GetControlImplBase()
{
	return mImpl;
}

template<class IMPL>
void MControl<IMPL>::Draw()
{
	mImpl->Draw();
}

template<class IMPL>
void MControl<IMPL>::MoveFrame(int32_t inXDelta, int32_t inYDelta)
{
	MView::MoveFrame(inXDelta, inYDelta);
	mImpl->FrameMoved();
}

template<class IMPL>
void MControl<IMPL>::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	MView::ResizeFrame(inWidthDelta, inHeightDelta);
	mImpl->FrameResized();
}

template<class IMPL>
void MControl<IMPL>::SetMargins(int32_t inLeftMargin, int32_t inTopMargin, int32_t inRightMargin, int32_t inBottomMargin)

{
	MView::SetMargins(inLeftMargin, inTopMargin, inRightMargin, inBottomMargin);
	mImpl->MarginsChanged();
}

template<class IMPL>
void MControl<IMPL>::ActivateSelf()
{
	mImpl->ActivateSelf();
}

template<class IMPL>
void MControl<IMPL>::DeactivateSelf()
{
	mImpl->DeactivateSelf();
}

template<class IMPL>
void MControl<IMPL>::EnableSelf()
{
	mImpl->EnableSelf();
}

template<class IMPL>
void MControl<IMPL>::DisableSelf()
{
	mImpl->DisableSelf();
}

template<class IMPL>
void MControl<IMPL>::ShowSelf()
{
	mImpl->ShowSelf();
}

template<class IMPL>
void MControl<IMPL>::HideSelf()
{
	mImpl->HideSelf();
}

template<class IMPL>
void MControl<IMPL>::AddedToWindow()
{
	mImpl->AddedToWindow();

	MControlBase::AddedToWindow();
}

