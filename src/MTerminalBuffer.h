// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MTypes.h"

#include <deque>
#include <string>
#include <vector>
#include <cassert>

// --------------------------------------------------------------------
// Terminal buffer code. We keep a history of the screen content.
// Each character on screen is stored in a MChar structure containing
// the unicode for the character and a style which is a bitfield of styles.
//

enum MCharStyle
{
	kStyleNormal	= 0,
	kStyleBold		= 1 << 0,
	kStyleUnderline	= 1 << 1,
	kStyleBlink		= 1 << 2,
	kStyleInverse	= 1 << 3,
	kStyleInvisible	= 1 << 4,
	
	// not really a style...
	kUnerasable		= 1 << 5,
	kProtected		= 1 << 6
};

enum MXTermColor
{
	kXTermColorNone = 256,
	
	kXTermColorBlack = 0,
	kXTermColorRed,
	kXTermColorGreen,
	kXTermColorYellow,
	kXTermColorBlue,
	kXTermColorMagenta,
	kXTermColorCyan,
	kXTermColorWhite,

	kXTermColorBrightBlack,
	kXTermColorBrightRed,
	kXTermColorBrightGreen,
	kXTermColorBrightYellow,
	kXTermColorBrightBlue,
	kXTermColorBrightMagenta,
	kXTermColorBrightCyan,
	kXTermColorBrightWhite
};

class MStyle
{
	enum { kForeColorMask = 0x01ff0000, kBackColorMask = 0x0000ff80, kFgShift = 16, kBgShift = 7,
		kDefaultStyle = kXTermColorNone << kFgShift | kXTermColorNone << kBgShift };

  public:
	MStyle() : mData(kDefaultStyle)
	{
		BOOST_STATIC_ASSERT(sizeof(MStyle) == 4);
	}

	explicit MStyle(MCharStyle inStyle) : mData(inStyle)
	{
		SetForeColor(kXTermColorNone);
		SetBackColor(kXTermColorNone);
	}
//	explicit MStyle(uint32 inValue) : mData(inValue) {}
	
	MStyle(MXTermColor inForeColor, MXTermColor inBackColor) : mData(0)
	{
		SetForeColor(inForeColor);
		SetBackColor(inBackColor);
	}

	bool operator&(MCharStyle inStyle) const
	{
		return (mData & inStyle) != 0;
	}
	
	operator uint32() const
	{
		return mData;
	}
	
	void SetFlag(MCharStyle inStyle)
	{
		mData |= inStyle;
	}

	void ReverseFlag(MCharStyle inStyle)
	{
		mData ^= inStyle;
	}

	void ClearFlag(MCharStyle inStyle)
	{
		mData &= ~inStyle;
	}

	void ChangeFlags(uint32 inMode)
	{
		switch (inMode)
		{
			case 0: mData &= ~(kStyleBold|kStyleUnderline|kStyleInverse|kStyleBlink); break;
			case 1: mData |= kStyleBold; break;
			case 4: mData |= kStyleUnderline; break;
			case 5: mData |= kStyleBlink; break;
			case 7: mData |= kStyleInverse; break;
			case 21: mData &= ~kStyleBold; break;
			case 24: mData &= ~kStyleUnderline; break;
			case 25: mData &= ~kStyleBlink; break;
			case 27: mData &= ~kStyleInverse; break;
		}
	}

	MXTermColor GetForeColor() const
	{
		return (MXTermColor)((mData & kForeColorMask) >> kFgShift);
	}

	void SetForeColor(MXTermColor inColor)
	{
		mData &= ~kForeColorMask;
		mData |= ((uint32)inColor << kFgShift) & kForeColorMask;
	}
  	
	MXTermColor GetBackColor() const
	{
		return (MXTermColor)((mData & kBackColorMask) >> kBgShift);
	}

	void SetBackColor(MXTermColor inColor)
	{
		mData &= ~kBackColorMask;
		mData |= ((uint32)inColor << kBgShift) & kBackColorMask;
	}
  	
  private:
	uint32	mData;
};

// MChar is a container for both a unicode and the style associated with
// this character in the buffer. Keeping them together makes coding easier.
// The unicode needs 17 bits, the style 15. We'll do the bit shifting
// ourselves since compilers tend to do a poor job here.

// Unicode is limited to 1ffff, Everything in Plane 2 and up is not supported.

class MChar
{
  public:

//	enum
//	{
////		kCharMask	= 0x001fffff,
//		kCharMask	= 0x0001ffff,
////		kStyleMask	= 0xffe00000
//		kStyleMask	= 0xfffe0000
//	};
//
				MChar()									: mUnicode(' ') {}
				MChar(MXTermColor inForeColor, MXTermColor inBackColor)
					: mUnicode(' '), mStyle(inForeColor, inBackColor) {}
				MChar(unicode inChar, MStyle inStyle)	: mUnicode(inChar), mStyle(inStyle) {}
				MChar(const MChar& inChar)				: mUnicode(inChar.mUnicode), mStyle(inChar.mStyle) {}

	MChar&		operator=(const MChar& inChar)			{ mUnicode = inChar.mUnicode; mStyle = inChar.mStyle; return *this; }
	MChar&		operator=(unicode inChar)				{ mUnicode = inChar; return *this; }
	MChar&		operator=(char inChar)					{ mUnicode = inChar; return *this; }
	MChar&		operator=(MStyle inStyle)				{ mStyle = inStyle; return *this; }

	bool		operator==(char rhs) const				{ return mUnicode == static_cast<uint32>(rhs); }
	bool		operator==(unicode rhs) const			{ return mUnicode == rhs; }
	bool		operator==(MStyle rhs) const			{ return mStyle == rhs; }

	bool		operator!=(char rhs) const				{ return mUnicode != static_cast<uint32>(rhs); }
	bool		operator!=(unicode rhs) const			{ return mUnicode != rhs; }
	bool		operator!=(MStyle rhs) const			{ return mStyle != rhs; }

	bool		operator&(MCharStyle inStyle) const		{ return (mStyle & inStyle) != 0; }
	
	void		operator|=(uint32 inStyle)				{ mStyle.SetFlag((MCharStyle)inStyle); }
	void		operator&=(uint32 inStyle)				{ mStyle.ClearFlag((MCharStyle)inStyle); }

	void		ReverseFlag(MCharStyle inStyle)			{ mStyle.ReverseFlag(inStyle); }
	void		ChangeFlags(uint32 inMode)				{ mStyle.ChangeFlags(inMode); }
	
				operator unicode () const				{ return mUnicode; }
				operator MStyle () const				{ return mStyle; }

  private:
	uint32		mUnicode;
	MStyle		mStyle;
};

// --------------------------------------------------------------------
// Characters are store in lines

class MLine
{
  public:
					MLine(uint32 inSize, MXTermColor inForeColor, MXTermColor inBackColor);
					MLine(const MLine& rhs);
					~MLine();

	MLine&			operator=(const MLine& rhs);
	
	void			Delete(uint32 inColumn, uint32 inWidth, MXTermColor inForeColor, MXTermColor inBackColor);
	void			Insert(uint32 inColumn, uint32 inWidth);
	
	MChar&			operator[](uint32 inColumn)			{ assert(inColumn < mSize); return mCharacters[inColumn]; }
	MChar			operator[](uint32 inColumn) const	{ assert(inColumn < mSize); return mCharacters[inColumn]; }

	void			swap(MLine& rhs);

	bool			IsSoftWrapped() const				{ return mSoftWrapped; }
	void			SetSoftWrapped(bool inSoftWrapped)	{ mSoftWrapped = inSoftWrapped; }
	
	bool			IsDoubleWidth() const				{ return mDoubleWidth; }
	bool			IsDoubleHeight() const				{ return mDoubleHeight; }
	bool			IsDoubleHeightTop() const			{ return mDoubleHeightTop; }

	void			SetDoubleWidth()					{ mDoubleWidth = true; mDoubleHeight = false; }
	void			SetDoubleHeight(bool inTop)			{ mDoubleHeight = true; mDoubleHeightTop = inTop; }
	void			SetSingleWidth()					{ mDoubleHeight = mDoubleWidth = false; }

	template<class OutputIterator>
	void			CopyOut(OutputIterator iter) const;

  private:
	MChar*			mCharacters;
	uint32			mSize;
	bool			mSoftWrapped;
	bool			mDoubleWidth, mDoubleHeight, mDoubleHeightTop;
};

namespace std
{
	template<>
	inline
	void swap(MLine& a, MLine& b)
	{
		a.swap(b);
	}
}

// --------------------------------------------------------------------
// And all the lines together for a buffer. We store the lines in a
// deque object. We push new lines to the front of this list, and
// when the buffer overflows we pop off remaining lines from the back.
// This means we have to know the height of the terminal since the
// terminal addresses the lines starting from 0 to height - 1 for on
// screen lines, and -1 .. -inf for buffered lines.

class MTerminalBuffer
{
  public:
					MTerminalBuffer(uint32 inWidth, uint32 inHeight, bool inBuffer);
	virtual			~MTerminalBuffer();

	void			SetBufferSize(uint32 inBufferSize)				{ mBufferSize = inBufferSize; }
	
	const MLine&	GetLine(int32 inLine) const;

					// anchor line is recalculated in Resize to help to 
					// adjust scrollbar.
	void			Resize(uint32 inWidth, uint32 inHeight, int32& ioAnchorLine);

	void			SetCharacter(uint32 inLine, uint32 inColumn, unicode inChar, MStyle inStyle = MStyle());

	template<typename Handler>
	void			ForeachInRectangle(int32 inFromLine, int32 inFromColumn,
						int32 inToLine, int32 inToColumn, Handler&& inHandler)
	{
		for (int32 li = inFromLine; li <= inToLine; ++li)
		{
			if (li >= static_cast<int32>(mLines.size()))
				break;
			
			MLine& line(mLines[li]);
			
			for (int32 ci = inFromColumn; ci <= inToColumn; ++ci)
			{
				if (ci >= static_cast<int32>(mWidth))
					break;
				
				inHandler(line[ci], li, ci);
			}
		}
	}
	
	void			ReverseFlag(uint32 inFromLine, uint32 inFromColumn,
						uint32 inToLine, uint32 inToColumn, MCharStyle inFlags);
	void			ChangeFlags(uint32 inFromLine, uint32 inFromColumn,
						uint32 inToLine, uint32 inToColumn, uint32 inMode);

	void			ScrollForward(uint32 inFromLine, uint32 inToLine,
						uint32 inLeftMargin, uint32 inRightMargin);
	void			ScrollBackward(uint32 inFromLine, uint32 inToLine,
						uint32 inLeftMargin, uint32 inRightMargin);

	void			Clear();

	void			EraseDisplay(uint32 inLine, uint32 inColumn, uint32 inMode, bool inSelective);
	void			EraseLine(uint32 inLine, uint32 inColumn, uint32 inMode, bool inSelective);
	void			EraseCharacter(uint32 inLine, uint32 inColumn, uint32 inCount);
	void			InsertCharacter(uint32 inLine, uint32 inColumn, uint32 inWidth = 0);
	void			DeleteCharacter(uint32 inLine, uint32 inColumn, uint32 inWidth = 0);
	
	void			WrapLine(uint32 inLine);
	
	void			SetLineDoubleWidth(uint32 inLine);
	void			SetLineDoubleHeight(uint32 inLine, bool inTop);
	void			SetLineSingleWidth(uint32 inLine);

	void			FillWithE();		// for DECALN
	
	void			SetDirty(bool inDirty)			{ mDirty = inDirty; }
	bool			IsDirty() const					{ return mDirty; }
	
	bool			IsSelectionEmpty() const;
	bool			IsSelectionBlock() const;

	void			GetSelectionBegin(int32& outLine, int32& outColumn) const;
	void			GetSelectionEnd(int32& outLine, int32& outColumn) const;
	void			GetSelection(int32& outBeginLine, int32& outBeginColumn,
						int32& outEndLine, int32& outEndColumn, bool& outIsBlock) const;

	void			SetSelection(int32 inBeginLine, int32 inBeginColumn,
						int32 inEndLine, int32 inEndColumn, bool inBlock = false);
	void			SelectAll();
	void			ClearSelection();

	void			SetColors(MXTermColor inForeColor, MXTermColor inBackColor)
	{
		mForeColor = inForeColor;
		mBackColor = inBackColor;
	}

	void			FindWord(int32 inLine, int32 inColumn, int32& outBeginLine, int32& outBeginColumn,
						int32& outEndLine, int32& outEndColumn);

	std::string		GetSelectedText() const;
	
	int32			BufferedLines() const			{ return static_cast<int32>(mBuffer.size()); }
	
	bool			FindNext(int32& ioLine, int32& ioColumn, const std::string& inWhat,
						bool inIgnoreCase, bool inWrapAround);
	bool			FindPrevious(int32& ioLine, int32& ioColumn, const std::string& inWhat,
						bool inIgnoreCase, bool inWrapAround);

  private:

	unicode			GetChar(uint32 inOffset, bool inToLower) const;

	std::deque<MLine>	mBuffer;
	uint32				mBufferSize;
	std::vector<MLine>	mLines;
	uint32				mWidth;
	bool				mDirty;
	int32				mBeginLine, mBeginColumn, mEndLine, mEndColumn;
	bool				mBlockSelection;
	MXTermColor			mForeColor, mBackColor;
};
