//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <vector>

#include <cairo.h>

#include "MTypes.hpp"
#include "MColor.hpp"

#undef DrawText

class MView;
class MTextLayout;
class MDevice;

enum MAlignment {
	eAlignNone,
	eAlignLeft,
	eAlignCenter,
	eAlignRight	
};

enum MListItemState {
	eLIS_Disabled,
	eLIS_Hot,
	eLIS_HotSelected,
	eLIS_Normal,
	eLIS_Selected,
	eLIS_SelectedNotFocus
};

enum MGeometryFillMode {
	eGeometryFillModeWinding,
	eGeometryFillModeAlternate
};

enum MGeometryBegin {
	eGeometryBeginFilled,
	eGeometryBeginHollow
};


class MGeometry
{
  public:
				MGeometry(MDevice& inDevice, MGeometryFillMode inMode = eGeometryFillModeAlternate);
				~MGeometry();
	
	void		Begin(float inX, float inY, MGeometryBegin inBegin);
	void		LineTo(float inX, float inY);
	void		CurveTo(float inX1, float inY1, float inX2, float inY2, float inX3, float inY3);
	void		End(bool inClose);
  private:
				MGeometry(const MGeometry&);
	MGeometry&	operator=(const MGeometry&);
	friend class MDevice;
	struct MGeometryImpl* mImpl;
};

class MBitmap
{
  public:
				MBitmap();
				MBitmap(MBitmap&& inBitmap);
	MBitmap&	operator=(MBitmap&& inBitmap);
				MBitmap(uint32 inWidth, uint32 inHeight, bool inUseAlpha = false);
				MBitmap(const void* inPNG, uint32 inLength);
				MBitmap(const MBitmap& inSource, MRect inCopyRect);
	virtual		~MBitmap();
	
	uint32*		Data() const				{ return mData; }
	uint32		Stride() const				{ return mStride; }
	bool		UseAlpha() const			{ return mUseAlpha; }
	
	uint32		Width() const				{ return mWidth; }
	uint32		Height() const				{ return mHeight; }

  private:
				MBitmap(const MBitmap&);
	MBitmap&	operator=(const MBitmap&);
	uint32*		mData;
	uint32		mWidth, mHeight, mStride;
	bool		mUseAlpha;
};

class MDevice
{
  public:
  				// create a dummy device, used for measuring text widths
				MDevice();
				// a regular device, used for drawing in a view
				// if inCreateOffscreen is true, drawing is done
				// in a GdkPixmap instead.
				// MDevice(MView* inView, MRect inRect, bool inCreateOffscreen = false);
				MDevice(MView* inView, cairo_t* inCairo);
				
				~MDevice();
	void		Save();
	
	void		Restore();
	bool		IsPrinting() const;
	
	int32		GetPageNr() const;
	MRect		GetBounds() const;
	
	static void	ListFonts(bool inFixedWidthOnly, std::vector<std::string>& outFonts);
	void		SetFont(const std::string& inFont);
	void		SetForeColor(MColor inColor);
	MColor		GetForeColor() const;
	void		SetBackColor(MColor inColor);
	MColor		GetBackColor() const;
	
	void		ClipRect(MRect inRect);
	
	//void		ClipRegion(MRegion inRegion);
	void		EraseRect(MRect inRect);
	void		FillRect(MRect inRect);
	void		StrokeRect(MRect inRect, uint32 inLineWidth = 1);
	void		StrokeLine(float inFromX, float inFromY, float inToX, float inToY, uint32 inLineWidth = 1);
//	void		StrokeBezier(float inCntrlPtX[4], float inCntrlPtY[4]);

	void		FillEllipse(MRect inRect);
	void		StrokeGeometry(MGeometry& inGeometry, float inLineWidth = 1.f);
	void		FillGeometry(MGeometry& inGeometry);
	void		DrawBitmap(const MBitmap& inBitmap, float inX, float inY);
	
	// void		CreateAndUsePattern(MColor inColor1, MColor inColor2, uint32 inWidth = 4, float inRotation = 45.f);
	
	float		GetAscent() const;
	float		GetDescent() const;
	float		GetLeading() const;
	int32		GetLineHeight() const;
	float		GetXWidth() const;
	void		DrawString(const std::string& inText, float inX, float inY, uint32 inTruncateWidth = 0, MAlignment inAlign = eAlignNone);
	void		DrawString(const std::string& inText, MRect inBounds, MAlignment inAlign = eAlignNone);
	// Text Layout options
	void		SetText(const std::string& inText);
	
	float		GetTextWidth() const;
	void		SetTabStops(float inTabWidth);
	
	void		SetTextColors(uint32 inColorCount, uint32 inColorIndices[], uint32 inOffsets[], MColor inColors[]);
	enum MTextStyle {
		eTextStyleNormal	= 0,
		eTextStyleItalic	= 1 << 0,
		eTextStyleBold		= 1 << 1,
		eTextStyleUnderline	= 1 << 2
	};
	
	void		SetTextStyles(uint32 inStyleCount, uint32 inStyles[], uint32 inOffsets[]);
	
	void		RenderTextBackground(float inX, float inY, uint32 inStart, uint32 inLength, MColor inColor);
	void		SetTextSelection(uint32 inStart, uint32 inLength, MColor inSelectionColor);
	
	void		IndexToPosition(uint32 inIndex, bool inTrailing, int32& outPosition);
//	bool		PositionToIndex(// int32 inPosition, // uint32& outIndex, // bool& outTrailing);

	bool		PositionToIndex(int32 inPosition, uint32& outIndex);
	
	void		RenderText(float inX, float inY);
	void		DrawCaret(float inX, float inY, uint32 inOffset);
	
	void		BreakLines(uint32 inWidth, std::vector<uint32>& outBreaks);
	void		MakeTransparent(float inOpacity);
	//GdkPixmap*	GetPixmap() const;
	void		SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor);
	// replace unknown characters uses a trick to force monospaced
	// fonts to render correctly if there's a character missing from
	// the font.
	void		SetReplaceUnknownCharacters(bool inReplaceUnknownCharacters);
	
	void		SetScale(float inScaleX, float inScaleY, float inCenterX, float inCenterY);
	
	static void	GetSysSelectionColor(MColor& outColor);
						
	// Theme support
	
	void		DrawListItemBackground(MRect inBounds, MListItemState inState);
	class MDeviceImpl*
				GetImpl()									{ return mImpl; }

  private:

				MDevice(const MDevice&);
	MDevice&	operator=(const MDevice&);
	class MDeviceImpl*	mImpl;
};

class MDeviceContextSaver
{
  public:
				MDeviceContextSaver(MDevice& inDevice)
					: mDevice(inDevice)
				{
					mDevice.Save();
				}
				
				~MDeviceContextSaver()
				{
					mDevice.Restore();
				}

  private:
	MDevice&	mDevice;
};
