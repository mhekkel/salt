//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cmath>

#include "MDevice.hpp"

// --------------------------------------------------------------------
// base class for MGeometryImpl

struct MGeometryImpl
{
	MGeometryImpl() {}
	virtual ~MGeometryImpl() {}

	virtual void Begin(float inX, float inY, MGeometryBegin inBegin) = 0;
	virtual void LineTo(float inX, float inY) = 0;
	virtual void CurveTo(float inX1, float inY1, float inX2, float inY2, float inX3, float inY3) = 0;
	virtual void End(bool inClose) = 0;

	static MGeometryImpl *
	Create(MDevice &inDevice, MGeometryFillMode inMode);
};

// --------------------------------------------------------------------
// base class for MDeviceImpl

class MDeviceImpl
{
  public:
	MDeviceImpl() {}
	virtual ~MDeviceImpl() {}

	virtual void Save() {}
	virtual void Restore() {}

	virtual bool IsPrinting(int32_t &outPage) const { return false; }
	virtual MRect GetBounds() const { return { 0, 0, 100, 100 }; }
	virtual void SetOrigin(int32_t inX, int32_t inY) {}

	virtual void SetFont(const std::string &inFont) {}

	virtual void SetForeColor(MColor inColor) {}
	virtual MColor GetForeColor() const { return kBlack; }
	virtual void SetBackColor(MColor inColor) {}
	virtual MColor GetBackColor() const { return kWhite; }

	virtual void ClipRect(MRect inRect) {}
	//virtual void			ClipRegion(// MRegion inRegion)				{}

	virtual void EraseRect(MRect inRect) {}
	virtual void FillRect(MRect inRect) {}
	virtual void StrokeRect(MRect inRect, uint32_t inLineWidth = 1) {}
	virtual void StrokeLine(float inFromX, float inFromY, float inToX, float inToY, uint32_t inLineWidth) {}
	virtual void FillEllipse(MRect inRect) {}
	virtual void StrokeGeometry(MGeometryImpl &inGeometry, float inLineWidth) {}
	virtual void FillGeometry(MGeometryImpl &inGeometry) {}
	virtual void DrawBitmap(const MBitmap &inBitmap, float inX, float inY) {}

	// virtual void			CreateAndUsePattern(MColor inColor1, MColor inColor2, uint32_t inWidth, float inRotation)				{}
	//PangoFontMetrics*		GetMetrics();

	virtual float GetAscent() { return 10; }
	virtual float GetDescent() { return 2; }
	virtual float GetLeading() { return 0; }
	virtual int32_t GetLineHeight()
	{
		return static_cast<int32_t>(std::ceil(GetAscent() + GetDescent() + GetLeading()));
	}
	virtual float GetXWidth() { return 8; }

	virtual void DrawString(const std::string &inText, float inX, float inY, uint32_t inTruncateWidth = 0, MAlignment inAlign = eAlignNone) {}
	virtual void DrawString(const std::string &inText, MRect inBounds, MAlignment inAlign = eAlignNone) {}
	virtual uint32_t GetStringWidth(const std::string &inText) { return 0; }

	// Text Layout options
	virtual void SetText(const std::string &inText) {}
	virtual void SetTabStops(float inTabWidth) {}
	virtual void SetTextColors(uint32_t inColorCount, uint32_t inColorIndices[], uint32_t inOffsets[], MColor inColors[]) {}
	virtual void SetTextStyles(uint32_t inStyleCount, uint32_t inStyles[], uint32_t inOffsets[]) {}
	virtual void RenderTextBackground(float inX, float inY, uint32_t inStart, uint32_t inLength, MColor inColor) {}
	virtual void SetTextSelection(uint32_t inStart, uint32_t inLength, MColor inSelectionColor) {}
	virtual void SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor) {}
	virtual void SetReplaceUnknownCharacters(bool inReplaceUnknownCharacters) {}
	virtual void IndexToPosition(uint32_t inIndex, bool inTrailing, int32_t &outPosition) {}
	virtual bool PositionToIndex(int32_t inPosition, uint32_t &outIndex) { return false; }
	virtual float GetTextWidth() { return 0; }
	virtual void RenderText(float inX, float inY) {}
	virtual void DrawCaret(float inX, float inY, uint32_t inOffset) {}
	virtual void BreakLines(uint32_t inWidth, std::vector<uint32_t> &outBreaks) {}

	virtual void SetScale(float inScaleX, float inScaleY, float inCenterX, float inCenterY) {}
	virtual void MakeTransparent(float inOpacity) {}
	//virtual GdkPixmap*		GetPixmap() const							{ return nullptr; }

	virtual void DrawListItemBackground(MRect inBounds, MListItemState inState) {}

	static MDeviceImpl *Create();
	static MDeviceImpl *Create(MView *inView);
};
