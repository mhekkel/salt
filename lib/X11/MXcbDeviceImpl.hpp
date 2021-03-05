//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <stack>

#include <pango/pangocairo.h>

#include "MDeviceImpl.hpp"

// --------------------------------------------------------------------

// --------------------------------------------------------------------
// base class for MDeviceImp
// provides only the basic Pango functionality
// This is needed in measuring text metrics and such

class MXcbDeviceImpl : public MDeviceImpl
{
  public:
							MXcbDeviceImpl();

							MXcbDeviceImpl(PangoLayout* inLayout);

	virtual					~MXcbDeviceImpl();

	virtual void			Save();
	virtual void			Restore();

	virtual void			SetOrigin(int32 inX, int32 inY);

	virtual void			SetFont(const std::string& inFont);

	virtual void			SetForeColor(MColor inColor);

	virtual MColor			GetForeColor() const;

	virtual void			SetBackColor(MColor inColor);

	virtual MColor			GetBackColor() const;
	
	virtual void			ClipRect(MRect inRect);

//	virtual void			ClipRegion(// MRegion inRegion);

	virtual void			EraseRect(MRect inRect);

	virtual void			FillRect(MRect inRect);

	virtual void			StrokeRect(MRect inRect, uint32 inLineWidth = 1);

	virtual void			FillEllipse(MRect inRect);

	virtual void			DrawImage(cairo_surface_t* inImage, float inX, float inY, float inShear);
	
	virtual void			CreateAndUsePattern(MColor inColor1, MColor inColor2);
	
	PangoFontMetrics*		GetMetrics();

	virtual float			GetAscent();
	
	virtual float			GetDescent();
	
	virtual float			GetLeading();

	virtual float			GetXWidth();
	
	virtual void			DrawString(const std::string& inText, float inX, float inY, uint32 inTruncateWidth = 0, MAlignment inAlign = eAlignNone);

	virtual uint32			GetStringWidth(const std::string& inText);

	// Text Layout options
	
	virtual void			SetText(const std::string& inText);
	
	virtual void			SetTabStops(float inTabWidth);
	
	virtual void			SetTextColors(uint32 inColorCount, uint32 inColorIndices[], uint32 inOffsets[], MColor inColors[]);
	virtual void			SetTextStyles(uint32 inStyleCount, uint32 inStyles[], uint32 inOffsets[]);
	virtual void			RenderTextBackground(float inX, float inY, uint32 inStart, uint32 inLength, MColor inColor);

	virtual void			SetTextSelection(uint32 inStart, uint32 inLength, MColor inSelectionColor);
	
	virtual void			IndexToPosition(uint32 inIndex, bool inTrailing, int32& outPosition);

	virtual bool			PositionToIndex(int32 inPosition, uint32& outIndex);
	
	virtual float			GetTextWidth();
	
	virtual void			RenderText(float inX, float inY);

	virtual void			DrawCaret(float inX, float inY, uint32 inOffset);
	
	virtual void			BreakLines(uint32 inWidth, std::vector<uint32>& outBreaks);

	virtual void			MakeTransparent(float inOpacity) {}

//	virtual GdkPixmap*		GetPixmap() const		{ return nullptr; }

	virtual void			SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor) {}

  protected:

	PangoItem*				Itemize(const char* inText, PangoAttrList* inAttrs);

	void					GetWhiteSpaceGlyphs(uint32& outSpace, uint32& outTab, uint32& outNL);

	PangoLayout*			mPangoLayout;
	PangoFontDescription*	mFont;
	PangoFontMetrics*		mMetrics;
	bool					mTextEndsWithNewLine;
	uint32					mSpaceGlyph, mTabGlyph, mNewLineGlyph;
};
