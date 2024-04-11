/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MTypes.hpp"

#include <cassert>
#include <deque>
#include <string>
#include <vector>

// --------------------------------------------------------------------
// Terminal buffer code. We keep a history of the screen content.
// Each character on screen is stored in a MChar structure containing
// the unicode for the character and a style which is a bitfield of styles.
//

enum MCharStyle
{
	kStyleNormal = 0,
	kStyleBold = 1 << 0,
	kStyleUnderline = 1 << 1,
	kStyleBlink = 1 << 2,
	kStyleInverse = 1 << 3,
	kStyleInvisible = 1 << 4,

	// not really a style...
	kUnerasable = 1 << 5,
	kProtected = 1 << 6
};

enum MXTermColor
{
	kXTermColorNone = 256,

	kXTermColorRegularBack,
	kXTermColorRegularText,

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
	enum
	{
		kForeColorMask = 0x01ff0000,
		kBackColorMask = 0x0000ff80,
		kFgShift = 16,
		kBgShift = 7,
		kDefaultStyle = kXTermColorNone << kFgShift | kXTermColorNone << kBgShift
	};

  public:
	MStyle()
		: mData(kDefaultStyle)
	{
		static_assert(sizeof(MStyle) == 4, "style should be four bytes");
	}

	explicit MStyle(MCharStyle inStyle)
		: mData(inStyle)
	{
		SetForeColor(kXTermColorNone);
		SetBackColor(kXTermColorNone);
	}
	//	explicit MStyle(uint32_t inValue) : mData(inValue) {}

	MStyle(MXTermColor inForeColor, MXTermColor inBackColor)
		: mData(0)
	{
		SetForeColor(inForeColor);
		SetBackColor(inBackColor);
	}

	bool operator&(MCharStyle inStyle) const
	{
		return (mData & inStyle) != 0;
	}

	operator uint32_t() const
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

	void ChangeFlags(uint32_t inMode)
	{
		switch (inMode)
		{
			case 0: mData &= ~(kStyleBold | kStyleUnderline | kStyleInverse | kStyleBlink); break;
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
		mData |= ((uint32_t)inColor << kFgShift) & kForeColorMask;
	}

	MXTermColor GetBackColor() const
	{
		return (MXTermColor)((mData & kBackColorMask) >> kBgShift);
	}

	void SetBackColor(MXTermColor inColor)
	{
		mData &= ~kBackColorMask;
		mData |= ((uint32_t)inColor << kBgShift) & kBackColorMask;
	}

  private:
	uint32_t mData;
};

// MChar is a container for both a unicode and the style associated with
// this character in the buffer. Keeping them together makes coding easier.
// Unicode is limited to 1ffff, Everything in Plane 2 and up is not supported.

class MChar
{
  public:
	MChar() = default;

	MChar(MXTermColor inForeColor, MXTermColor inBackColor) noexcept
		: mUnicode(' ')
		, mStyle(inForeColor, inBackColor)
	{
	}

	MChar(unicode inChar, MStyle inStyle, int inHyperLink = 0) noexcept
		: mUnicode(inChar)
		, mStyle(inStyle)
		, mHyperLink(inHyperLink)
	{
	}

	MChar(const MChar &inChar) noexcept
		: mUnicode(inChar.mUnicode)
		, mStyle(inChar.mStyle)
		, mHyperLink(inChar.mHyperLink)
	{
	}

	MChar &operator=(const MChar &inChar) noexcept
	{
		mUnicode = inChar.mUnicode;
		mStyle = inChar.mStyle;
		mHyperLink = inChar.mHyperLink;
		return *this;
	}

	MChar &operator=(unicode inChar) noexcept
	{
		mUnicode = inChar;
		return *this;
	}

	MChar &operator=(char inChar) noexcept
	{
		mUnicode = inChar;
		return *this;
	}

	MChar &operator=(MStyle inStyle) noexcept
	{
		mStyle = inStyle;
		return *this;
	}

	bool operator==(char rhs) const { return mUnicode == static_cast<char32_t>(rhs); }
	bool operator==(unicode rhs) const { return mUnicode == rhs; }
	bool operator==(MStyle rhs) const { return mStyle == rhs; }

	bool operator!=(char rhs) const { return mUnicode != static_cast<char32_t>(rhs); }
	bool operator!=(unicode rhs) const { return mUnicode != rhs; }
	bool operator!=(MStyle rhs) const { return mStyle != rhs; }

	bool operator&(MCharStyle inStyle) const { return (mStyle & inStyle) != 0; }

	void operator|=(char32_t inStyle) { mStyle.SetFlag((MCharStyle)inStyle); }
	void operator&=(char32_t inStyle) { mStyle.ClearFlag((MCharStyle)inStyle); }

	void ReverseFlag(MCharStyle inStyle) { mStyle.ReverseFlag(inStyle); }
	void ChangeFlags(char32_t inMode) { mStyle.ChangeFlags(inMode); }

	operator unicode() const { return mUnicode; }
	operator MStyle() const { return mStyle; }

	void SetHyperLink(int inLinkNr)
	{
		mHyperLink = inLinkNr;
	}

	int GetHyperLink() const
	{
		return mHyperLink;
	}

  private:
	char32_t mUnicode = ' ';
	MStyle mStyle{};
	int mHyperLink = 0;
};

static_assert(sizeof(MChar) == 12, "MChar should be 12 bytes");

// --------------------------------------------------------------------
// Characters are store in lines

class MLine
{
  public:
	MLine(uint32_t inSize, MXTermColor inForeColor, MXTermColor inBackColor);

	MLine(const MLine &rhs);

	MLine(MLine &&rhs) noexcept
	{
		swap(*this, rhs);
	}

	~MLine();

	MLine &operator=(MLine rhs) noexcept
	{
		swap(*this, rhs);
		return *this;
	}

	void Delete(uint32_t inColumn, uint32_t inWidth, MXTermColor inForeColor, MXTermColor inBackColor);
	void Insert(uint32_t inColumn, uint32_t inWidth);

	MChar &operator[](uint32_t inColumn)
	{
		assert(inColumn < mSize);
		return mCharacters[inColumn];
	}
	MChar operator[](uint32_t inColumn) const
	{
		assert(inColumn < mSize);
		return mCharacters[inColumn];
	}

	friend void swap(MLine &lhs, MLine &rhs) noexcept;

	bool IsSoftWrapped() const { return mSoftWrapped; }
	void SetSoftWrapped(bool inSoftWrapped) { mSoftWrapped = inSoftWrapped; }

	bool IsDoubleWidth() const { return mDoubleWidth; }
	bool IsDoubleHeight() const { return mDoubleHeight; }
	bool IsDoubleHeightTop() const { return mDoubleHeightTop; }

	void SetDoubleWidth()
	{
		mDoubleWidth = true;
		mDoubleHeight = false;
	}
	void SetDoubleHeight(bool inTop)
	{
		mDoubleHeight = true;
		mDoubleHeightTop = inTop;
	}
	void SetSingleWidth() { mDoubleHeight = mDoubleWidth = false; }

	template <class OutputIterator>
	void CopyOut(OutputIterator iter) const;

	struct iterator
	{
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = MChar;
		using pointer = value_type *;
		using reference = value_type &;
		using difference_type = std::ptrdiff_t;

		iterator(MChar *inPtr)
			: mPtr(inPtr)
		{
		}

		reference operator*() { return *mPtr; }
		pointer operator->() const { return mPtr; }

		iterator &operator++()
		{
			++mPtr;
			return *this;
		}

		iterator operator++(int)
		{
			iterator iter(*this);
			operator++();
			return iter;
		}

		iterator &operator--()
		{
			--mPtr;
			return *this;
		}

		iterator operator--(int)
		{
			iterator iter(*this);
			operator--();
			return iter;
		}

		friend auto operator<=>(const iterator &lhs, const iterator &rhs) = default;

		MChar *mPtr;
	};

	iterator begin() { return iterator(mCharacters); }
	iterator end() { return iterator(mCharacters + mSize); }

  private:
	MChar *mCharacters = nullptr;
	uint32_t mSize = 0;
	bool mSoftWrapped = false;
	bool mDoubleWidth = false, mDoubleHeight = false, mDoubleHeightTop = false;
};

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
	MTerminalBuffer(uint32_t inWidth, uint32_t inHeight, bool inBuffer);
	virtual ~MTerminalBuffer();

	void SetBufferSize(uint32_t inBufferSize) { mBufferSize = inBufferSize; }

	const MLine &GetLine(int32_t inLine) const;

	// anchor line is recalculated in Resize to help to
	// adjust scrollbar.
	void Resize(uint32_t inWidth, uint32_t inHeight, int32_t &ioAnchorLine);

	void SetCharacter(uint32_t inLine, uint32_t inColumn, unicode inChar,
		MStyle inStyle = MStyle(), int inHyperLink = 0);

	template <typename Handler>
	void ForeachInRectangle(int32_t inFromLine, int32_t inFromColumn,
		int32_t inToLine, int32_t inToColumn, Handler &&inHandler)
	{
		for (int32_t li = inFromLine; li <= inToLine; ++li)
		{
			if (li >= static_cast<int32_t>(mLines.size()))
				break;

			MLine &line(mLines[li]);

			for (int32_t ci = inFromColumn; ci <= inToColumn; ++ci)
			{
				if (ci >= static_cast<int32_t>(mWidth))
					break;

				inHandler(line[ci], li, ci);
			}
		}
	}

	void ReverseFlag(uint32_t inFromLine, uint32_t inFromColumn,
		uint32_t inToLine, uint32_t inToColumn, MCharStyle inFlags);
	void ChangeFlags(uint32_t inFromLine, uint32_t inFromColumn,
		uint32_t inToLine, uint32_t inToColumn, uint32_t inMode);

	void ScrollForward(uint32_t inFromLine, uint32_t inToLine,
		uint32_t inLeftMargin, uint32_t inRightMargin);
	void ScrollBackward(uint32_t inFromLine, uint32_t inToLine,
		uint32_t inLeftMargin, uint32_t inRightMargin);

	void Clear();

	void EraseDisplay(uint32_t inLine, uint32_t inColumn, uint32_t inMode, bool inSelective);
	void EraseLine(uint32_t inLine, uint32_t inColumn, uint32_t inMode, bool inSelective);
	void EraseCharacter(uint32_t inLine, uint32_t inColumn, uint32_t inCount);
	void InsertCharacter(uint32_t inLine, uint32_t inColumn, uint32_t inWidth = 0);
	void DeleteCharacter(uint32_t inLine, uint32_t inColumn, uint32_t inWidth = 0);

	void WrapLine(uint32_t inLine);

	void SetLineDoubleWidth(uint32_t inLine);
	void SetLineDoubleHeight(uint32_t inLine, bool inTop);
	void SetLineSingleWidth(uint32_t inLine);

	void FillWithE(); // for DECALN

	void SetDirty(bool inDirty) { mDirty = inDirty; }
	bool IsDirty() const { return mDirty; }

	bool IsSelectionEmpty() const;
	bool IsSelectionBlock() const;

	void GetSelectionBegin(int32_t &outLine, int32_t &outColumn) const;
	void GetSelectionEnd(int32_t &outLine, int32_t &outColumn) const;
	void GetSelection(int32_t &outBeginLine, int32_t &outBeginColumn,
		int32_t &outEndLine, int32_t &outEndColumn, bool &outIsBlock) const;

	void SetSelection(int32_t inBeginLine, int32_t inBeginColumn,
		int32_t inEndLine, int32_t inEndColumn, bool inBlock = false);
	void SelectAll();
	void ClearSelection();

	void SetColors(MXTermColor inForeColor, MXTermColor inBackColor)
	{
		mForeColor = inForeColor;
		mBackColor = inBackColor;
	}

	void FindWord(int32_t inLine, int32_t inColumn, int32_t &outBeginLine, int32_t &outBeginColumn,
		int32_t &outEndLine, int32_t &outEndColumn);

	std::string GetSelectedText() const;

	int32_t BufferedLines() const { return static_cast<int32_t>(mBuffer.size()); }

	bool FindNext(int32_t &ioLine, int32_t &ioColumn, const std::string &inWhat,
		bool inIgnoreCase, bool inWrapAround);
	bool FindPrevious(int32_t &ioLine, int32_t &ioColumn, const std::string &inWhat,
		bool inIgnoreCase, bool inWrapAround);

	int AddHyperLink(const std::string &inURI, const std::string &inID);
	int GetHoveredLink(int32_t inLine, int32_t inColumn) const;
	std::string GetHyperLink(int inNr) const;

  private:
	unicode GetChar(uint32_t inOffset, bool inToLower) const;

	void GarbageCollectHyperlinks();

	std::deque<MLine> mBuffer;
	uint32_t mBufferSize;
	std::vector<MLine> mLines;
	uint32_t mWidth;
	bool mDirty;
	int32_t mBeginLine, mBeginColumn, mEndLine, mEndColumn;
	bool mBlockSelection;
	MXTermColor mForeColor, mBackColor;

	// On screen hyperlinks
	int mNextHyperLinkNr = 1;

	struct MHyperLink
	{
		int nr;
		std::string mID, mURI;
	};

	std::vector<MHyperLink> mHyperLinks;
};
