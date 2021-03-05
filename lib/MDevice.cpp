//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <cmath>
#include <cstring>

#include "MDevice.hpp"
#include "MView.hpp"
#include "MWindow.hpp"
#include "MError.hpp"
#include "MUnicode.hpp"

#include "MDeviceImpl.hpp"

using namespace std;

// -------------------------------------------------------------------

MGeometry::MGeometry(MDevice& inDevice, MGeometryFillMode inMode)
	: mImpl(MGeometryImpl::Create(inDevice, inMode))
{
}

MGeometry::~MGeometry()
{
	delete mImpl;
}

void MGeometry::Begin(float inX, float inY, MGeometryBegin inBegin)
{
	mImpl->Begin(inX, inY, inBegin);
}

void MGeometry::LineTo(float inX, float inY)
{
	mImpl->LineTo(inX, inY);
}

void MGeometry::CurveTo(float inX1, float inY1, float inX2, float inY2, float inX3, float inY3)
{
	mImpl->CurveTo(inX1, inY1, inX2, inY2, inX3, inY3);
}

void MGeometry::End(bool inClose)
{
	mImpl->End(inClose);
}

// -------------------------------------------------------------------

MBitmap::MBitmap()
	: mData(nullptr), mWidth(0), mHeight(0), mStride(0), mUseAlpha(false)
{
}

MBitmap::MBitmap(MBitmap&& inBitmap)
{
	mData = inBitmap.mData;		inBitmap.mData = nullptr;
	mWidth = inBitmap.mWidth;	inBitmap.mWidth = 0;
	mHeight = inBitmap.mHeight;	inBitmap.mHeight = 0;
	mStride = inBitmap.mStride;	inBitmap.mStride = 0;
	mUseAlpha = inBitmap.mUseAlpha;
								inBitmap.mUseAlpha = false;
}

MBitmap& MBitmap::operator=(MBitmap&& inBitmap)
{
	mData = inBitmap.mData;		inBitmap.mData = nullptr;
	mWidth = inBitmap.mWidth;	inBitmap.mWidth = 0;
	mHeight = inBitmap.mHeight;	inBitmap.mHeight = 0;
	mStride = inBitmap.mStride;	inBitmap.mStride = 0;
	mUseAlpha = inBitmap.mUseAlpha;
								inBitmap.mUseAlpha = false;
	return *this;
}

MBitmap::MBitmap(uint32_t inWidth, uint32_t inHeight, bool inUseAlpha)
	: mData(nullptr)
	, mWidth(inWidth)
	, mHeight(inHeight)
	, mStride(inWidth * sizeof(uint32_t))
	, mUseAlpha(inUseAlpha)
{
	mData = new uint32_t[inWidth * inHeight];
}

MBitmap::MBitmap(const MBitmap& inSource, MRect inCopyRect)
	: mData(nullptr)
	, mWidth(inCopyRect.width)
	, mHeight(inCopyRect.height)
	, mStride(mWidth * sizeof(uint32_t))
	, mUseAlpha(inSource.mUseAlpha)
{
	mData = new uint32_t[mWidth * mHeight];

	if (inCopyRect.x + inCopyRect.width <= static_cast<int32_t>(inSource.mWidth) and
		inCopyRect.y + inCopyRect.height <= static_cast<int32_t>(inSource.mHeight))
	{
		for (uint32_t y = 0; y < mHeight; ++y)
		{
			uint32_t sy = y + inCopyRect.y;
			for (uint32_t x = 0; x < mWidth; ++x)
			{
				uint32_t sx = x + inCopyRect.x;
				mData[y * mWidth + x] = inSource.mData[sy * inSource.mWidth + sx];
			}
		}
	}
}

MBitmap::~MBitmap()
{
	delete[] mData;
}

// -------------------------------------------------------------------

MDevice::MDevice()
	: mImpl(MDeviceImpl::Create())
{
}

// MDevice::MDevice(
// 	MView*		inView,
// 	MRect		inRect,
// 	bool		inCreateOffscreen)
// 	: mImpl(MDeviceImpl::Create(inView, inRect, inCreateOffscreen))
// {
// }

MDevice::MDevice(MView* inView, cairo_t* inCairo)
	: mImpl(MDeviceImpl::Create(inView, inCairo))
{
}

MDevice::~MDevice()
{
	delete mImpl;
}

void MDevice::Save()
{
	mImpl->Save();
}

void MDevice::Restore()
{
	mImpl->Restore();
}

bool MDevice::IsPrinting() const
{
	int32_t page;
	return mImpl->IsPrinting(page);
}

int32_t MDevice::GetPageNr() const
{
	int32_t page;
	if (not mImpl->IsPrinting(page))
		page = -1;
	return page;
}

MRect MDevice::GetBounds() const
{
	return mImpl->GetBounds();
}

void MDevice::SetFont(
	const string&	inFont)
{
	mImpl->SetFont(inFont);
}

void MDevice::SetForeColor(
	MColor		inColor)
{
	mImpl->SetForeColor(inColor);
}

MColor MDevice::GetForeColor() const
{
	return mImpl->GetForeColor();
}

void MDevice::SetBackColor(
	MColor		inColor)
{
	mImpl->SetBackColor(inColor);
}

MColor MDevice::GetBackColor() const
{
	return mImpl->GetBackColor();
}

void MDevice::ClipRect(
	MRect		inRect)
{
	mImpl->ClipRect(inRect);
}

//void MDevice::ClipRegion(
//	MRegion		inRegion)
//{
//	mImpl->ClipRegion(inRegion);
//}

void MDevice::EraseRect(
	MRect		inRect)
{
	mImpl->EraseRect(inRect);
}

void MDevice::FillRect(
	MRect		inRect)
{
	mImpl->FillRect(inRect);
}

void MDevice::StrokeRect(
	MRect		inRect,
	uint32_t		inLineWidth)
{
	mImpl->StrokeRect(inRect, inLineWidth);
}

void MDevice::StrokeLine(
	float				inFromX,
	float				inFromY,
	float				inToX,
	float				inToY,
	uint32_t				inLineWidth)
{
	mImpl->StrokeLine(inFromX, inFromY, inToX, inToY, inLineWidth);	
}

void MDevice::StrokeGeometry(
	MGeometry&			inGeometry,
	float				inLineWidth)
{
	mImpl->StrokeGeometry(*inGeometry.mImpl, inLineWidth);
}

void MDevice::FillGeometry(
	MGeometry&			inGeometry)
{
	mImpl->FillGeometry(*inGeometry.mImpl);
}

void MDevice::FillEllipse(
	MRect		inRect)
{
	mImpl->FillEllipse(inRect);
}

void MDevice::DrawBitmap(
	const MBitmap&	inBitmap,
	float			inX,
	float			inY)
{
	mImpl->DrawBitmap(inBitmap, inX, inY);
}

// void MDevice::CreateAndUsePattern(
// 	MColor		inColor1,
// 	MColor		inColor2,
// 	uint32_t		inWidth,
// 	float		inRotation)
// {
// 	mImpl->CreateAndUsePattern(inColor1, inColor2, inWidth, inRotation);
// }

float MDevice::GetAscent() const
{
	return mImpl->GetAscent();
}

float MDevice::GetDescent() const
{
	return mImpl->GetDescent();
}

float MDevice::GetLeading() const
{
	return mImpl->GetLeading();
}

int32_t MDevice::GetLineHeight() const
{
	return mImpl->GetLineHeight();
}

float MDevice::GetXWidth() const
{
	return mImpl->GetXWidth();
}

void MDevice::DrawString(
	const string&	inText,
	float 			inX,
	float 			inY,
	uint32_t			inTruncateWidth,
	MAlignment		inAlign)
{
	mImpl->DrawString(inText, inX, inY, inTruncateWidth, inAlign);
}

void MDevice::DrawString(
	const string&	inText,
	MRect 			inBounds,
	MAlignment		inAlign)
{
	mImpl->DrawString(inText, inBounds, inAlign);
}

void MDevice::SetText(
	const string&	inText)
{
	mImpl->SetText(inText);
}

void MDevice::SetTabStops(
	float			inTabWidth)
{
	mImpl->SetTabStops(inTabWidth);
}

void MDevice::SetTextColors(
	uint32_t				inColorCount,
	uint32_t				inColorIndices[],
	uint32_t				inOffsets[],
	MColor				inColors[])
{
	mImpl->SetTextColors(inColorCount, inColorIndices, inOffsets, inColors);
}

void MDevice::SetTextStyles(
	uint32_t				inStyleCount,
	uint32_t				inStyles[],
	uint32_t				inOffsets[])
{
	mImpl->SetTextStyles(inStyleCount, inStyles, inOffsets);
}

void MDevice::RenderTextBackground(
	float				inX,
	float				inY,
	uint32_t				inStart,
	uint32_t				inLength,
	MColor				inColor)
{
	mImpl->RenderTextBackground(inX, inY, inStart, inLength, inColor);
}

void MDevice::SetTextSelection(
	uint32_t			inStart,
	uint32_t			inLength,
	MColor			inSelectionColor)
{
	mImpl->SetTextSelection(inStart, inLength, inSelectionColor);
}

void MDevice::IndexToPosition(
	uint32_t			inIndex,
	bool			inTrailing,
	int32_t&			outPosition)
{
	mImpl->IndexToPosition(inIndex, inTrailing, outPosition);
}

bool MDevice::PositionToIndex(
	int32_t			inPosition,
	uint32_t&			outIndex)
{
	return mImpl->PositionToIndex(inPosition, outIndex);
}

void MDevice::RenderText(
	float			inX,
	float			inY)
{
	mImpl->RenderText(inX, inY);
}

float MDevice::GetTextWidth() const
{
	return mImpl->GetTextWidth();
}

void MDevice::BreakLines(
	uint32_t				inWidth,
	vector<uint32_t>&		outBreaks)
{
	mImpl->BreakLines(inWidth, outBreaks);
}

void MDevice::DrawCaret(
	float				inX,
	float				inY,
	uint32_t				inOffset)
{
	mImpl->DrawCaret(inX, inY, inOffset);
}

void MDevice::MakeTransparent(
	float				inOpacity)
{
	mImpl->MakeTransparent(inOpacity);
}

//GdkPixmap* MDevice::GetPixmap() const
//{
//	return mImpl->GetPixmap();
//}
//
void MDevice::SetDrawWhiteSpace(
	bool				inDrawWhiteSpace,
	MColor				inWhiteSpaceColor)
{
	mImpl->SetDrawWhiteSpace(inDrawWhiteSpace, inWhiteSpaceColor);
}

void MDevice::SetReplaceUnknownCharacters(
	bool				inReplaceUnknownCharacters)
{
	mImpl->SetReplaceUnknownCharacters(inReplaceUnknownCharacters);
}

void MDevice::SetScale(
	float				inScaleX,
	float				inScaleY,
	float				inCenterX,
	float				inCenterY)
{
	mImpl->SetScale(inScaleX, inScaleY, inCenterX, inCenterY);
}

void MDevice::DrawListItemBackground(
	MRect				inBounds,
	MListItemState		inState)
{
	mImpl->DrawListItemBackground(inBounds, inState);
}

//void MDevice::DrawImage(
//	cairo_surface_t*	inImage,
//	float				inX,
//	float				inY,
//	float				inShear)
//{
//	mImpl->DrawImage(inImage, inX, inY, inShear);
//}
