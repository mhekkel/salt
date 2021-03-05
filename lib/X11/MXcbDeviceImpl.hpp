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

	virtual void			SetOrigin(int32_t inX, int32_t inY);

	virtual void			SetFont(const std::string& inFont);

	virtual void			SetForeColor(MColor inColor);

	virtual MColor			GetForeColor() const;

	virtual void			SetBackColor(MColor inColor);

	virtual MColor			GetBackColor() const;
	
	virtual void			ClipRect(MRect inRect);

//	virtual void			ClipRegion(// MRegion inRegion);

	virtual void			EraseRect(MRect inRect);

	virtual void			FillRect(MRect inRect);

	virtual void			StrokeRect(MRect inRect, uint32_t inLineWidth = 1);

	virtual void			FillEllipse(MRect inRect);

	virtual void			DrawImage(cairo_surface_t* inImage, float inX, float inY, float inShear);
	
	virtual void			CreateAndUsePattern(MColor inColor1, MColor inColor2);
	
	PangoFontMetrics*		GetMetrics();

	virtual float			GetAscent();
	
	virtual float			GetDescent();
	
	virtual float			GetLeading();

	virtual float			GetXWidth();
	
	virtual void			DrawString(const std::string& inText, float inX, float inY, uint32_t inTruncateWidth = 0, MAlignment inAlign = eAlignNone);

	virtual uint32_t			GetStringWidth(const std::string& inText);

	// Text Layout options
	
	virtual void			SetText(const std::string& inText);
	
	virtual void			SetTabStops(float inTabWidth);
	
	virtual void			SetTextColors(uint32_t inColorCount, uint32_t inColorIndices[], uint32_t inOffsets[], MColor inColors[]);
	virtual void			SetTextStyles(uint32_t inStyleCount, uint32_t inStyles[], uint32_t inOffsets[]);
	virtual void			RenderTextBackground(float inX, float inY, uint32_t inStart, uint32_t inLength, MColor inColor);

	virtual void			SetTextSelection(uint32_t inStart, uint32_t inLength, MColor inSelectionColor);
	
	virtual void			IndexToPosition(uint32_t inIndex, bool inTrailing, int32_t& outPosition);

	virtual bool			PositionToIndex(int32_t inPosition, uint32_t& outIndex);
	
	virtual float			GetTextWidth();
	
	virtual void			RenderText(float inX, float inY);

	virtual void			DrawCaret(float inX, float inY, uint32_t inOffset);
	
	virtual void			BreakLines(uint32_t inWidth, std::vector<uint32_t>& outBreaks);

	virtual void			MakeTransparent(float inOpacity) {}

//	virtual GdkPixmap*		GetPixmap() const		{ return nullptr; }

	virtual void			SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor) {}

  protected:

	PangoItem*				Itemize(const char* inText, PangoAttrList* inAttrs);

	void					GetWhiteSpaceGlyphs(uint32_t& outSpace, uint32_t& outTab, uint32_t& outNL);

	PangoLayout*			mPangoLayout;
	PangoFontDescription*	mFont;
	PangoFontMetrics*		mMetrics;
	bool					mTextEndsWithNewLine;
	uint32_t					mSpaceGlyph, mTabGlyph, mNewLineGlyph;
};
