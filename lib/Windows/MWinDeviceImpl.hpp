//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <stack>
#include <cassert>

#include "comptr.hpp"

#include "MDeviceImpl.hpp"

typedef ComPtr<ID2D1Factory>			ID2D1FactoryPtr;
typedef ComPtr<ID2D1RenderTarget>		ID2D1RenderTargetPtr;
typedef ComPtr<ID2D1Brush> 				ID2D1BrushPtr;
typedef ComPtr<ID2D1DrawingStateBlock>	ID2D1DrawingStateBlockPtr;
typedef ComPtr<ID2D1PathGeometry>		ID2D1PathGeometryPtr;
typedef ComPtr<ID2D1GeometrySink>		ID2D1GeometrySinkPtr;
typedef ComPtr<IDWriteFont>				IDWriteFontPtr;
typedef ComPtr<IDWriteTextLayout>		IDWriteTextLayoutPtr;
typedef ComPtr<IDWriteTextFormat>		IDWriteTextFormatPtr;
typedef ComPtr<IDWriteInlineObject>		IDWriteInlineObjectPtr;

class MWinDeviceImpl;

// --------------------------------------------------------------------

struct MWinGeometryImpl : public MGeometryImpl
{
					MWinGeometryImpl(MWinDeviceImpl& inDeviceImpl,
						MGeometryFillMode inMode);
	virtual			~MWinGeometryImpl();
	
	virtual void	Begin(float inX, float inY, MGeometryBegin inBegin);
	virtual void	LineTo(float inX, float inY);
	virtual void	CurveTo(float inX1, float inY1, float inX2, float inY2, float inX3, float inY3);
	virtual void	End(bool inClose);

	ID2D1PathGeometryPtr		mPath;
	ID2D1GeometrySinkPtr		mSink;
};

// --------------------------------------------------------------------

class MWinDeviceImpl : public MDeviceImpl
{
  public:
							MWinDeviceImpl();
							MWinDeviceImpl(MView* inView, MRect inBounds, bool inOffscreen);

	static ID2D1Factory*	GetD2D1Factory();
	static IDWriteFactory*	GetDWFactory();
	static std::wstring		GetLocale();

	virtual					~MWinDeviceImpl();
	virtual void			Save();
	virtual void			Restore();
	virtual MRect			GetBounds() const						{ return MRect(0, 0, 100, 100); }

	virtual void			SetFont(const std::string& inFont);
	virtual void			SetForeColor(MColor inColor);
	virtual MColor			GetForeColor() const;
	virtual void			SetBackColor(MColor inColor);
	virtual MColor			GetBackColor() const;
	
	virtual void			ClipRect(MRect inRect);
	//virtual void			ClipRegion(// MRegion inRegion);
	virtual void			EraseRect(MRect inRect);
	virtual void			FillRect(MRect inRect);
	virtual void			StrokeRect(MRect inRect, uint32 inLineWidth = 1);
	virtual void			StrokeLine(float inFromX, float inFromY, float inToX, float inToY, uint32 inLineWidth);
	virtual void			FillEllipse(MRect inRect);
	virtual void			StrokeGeometry(MGeometryImpl& inGeometry, float inLineWidth);
	virtual void			FillGeometry(MGeometryImpl& inGeometry);
	virtual void			DrawBitmap(const MBitmap& inBitmap, float inX, float inY);
	virtual void			CreateAndUsePattern(MColor inColor1, MColor inColor2, uint32 inWidth, float inRotation);
	
	virtual float			GetAscent();
	virtual float			GetDescent();
	virtual int32			GetLineHeight();
	virtual float			GetXWidth();
	virtual void			DrawString(const std::string& inText, float inX, float inY, uint32 inTruncateWidth = 0, MAlignment inAlign = eAlignNone);
	virtual void			DrawString(const std::string& inText, MRect inBounds, MAlignment inAlign);
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
	virtual void			SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor);
	virtual void			SetReplaceUnknownCharacters(bool inReplaceUnknownCharacters);
	virtual void			SetScale(float inScaleX, float inScaleY, float inCenterX, float inCenterY);
	virtual void			DrawListItemBackground(MRect inBounds, MListItemState inState);

  protected:

	void					CreateTextFormat();
	void					LookupFont(const std::wstring& inFamily);
	uint32					MapBack(uint32 inOffset);

	MView*					mView;
//	HDC						mDC;

	ID2D1RenderTargetPtr	mRenderTarget;
	ID2D1Layer*				mClipLayer;
	std::stack<MRect>		mClipping;
	IDWriteTextFormatPtr	mTextFormat;
	IDWriteTextLayoutPtr	mTextLayout;
	mutable ID2D1BrushPtr	mForeBrush;
	mutable ID2D1BrushPtr	mBackBrush;
	IDWriteInlineObjectPtr	mTrimmingSign;

	std::wstring			mFontFamily;
	float					mFontSize;
	IDWriteFontPtr			mFont;
	bool					mReplaceUnknownCharacters;
	bool					mDrawWhiteSpace;

	// converted text (from UTF8 to UTF16)
	std::wstring			mText;
	std::vector<uint16>		mTextIndex;		// from string to wstring
	MColor					mSelectionColor;
	uint32					mSelectionStart, mSelectionLength;
	MColor					mWhitespaceColor;

	float					mDpiScaleX, mDpiScaleY;

	struct State
	{
							State(MWinDeviceImpl& inImpl);

							~State();

		MWinDeviceImpl&		mImpl;
		ID2D1DrawingStateBlockPtr
							mStateBlock;
		ID2D1BrushPtr		mForeBrush;
		ID2D1BrushPtr		mBackBrush;

	  private:
							State(const State&);
		State&				operator=(const State&);
	};
	
	friend struct State;	

	std::stack<State*>		mState;
};
