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

#include "MTerminalBuffer.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MUnicode.hpp"

#include <algorithm>

using namespace std;

static_assert(sizeof(MChar) == 8, "An MChar should be eight bytes");

// --------------------------------------------------------------------

MLine::MLine(uint32_t inSize, MXTermColor inForeColor, MXTermColor inBackColor)
	: mCharacters(new MChar[inSize])
	, mSize(inSize)
	, mSoftWrapped(false)
	, mDoubleWidth(false)
	, mDoubleHeight(false)
{
	std::for_each(mCharacters, mCharacters + mSize,
		[inForeColor, inBackColor](MChar &ch)
		{ ch = MChar(inForeColor, inBackColor); });
}

MLine::MLine(const MLine &rhs)
	: mCharacters(new MChar[rhs.mSize])
	, mSize(rhs.mSize)
	, mSoftWrapped(rhs.mSoftWrapped)
	, mDoubleWidth(rhs.mDoubleWidth)
	, mDoubleHeight(rhs.mDoubleHeight)
	, mDoubleHeightTop(rhs.mDoubleHeightTop)
{
	copy(rhs.mCharacters, rhs.mCharacters + mSize, mCharacters);
}

MLine::~MLine()
{
	delete[] mCharacters;
}

MLine &MLine::operator=(const MLine &rhs)
{
	if (this != &rhs)
	{
		if (rhs.mSize != mSize)
		{
			MChar *tmp = new MChar[rhs.mSize];
			delete[] mCharacters;
			mCharacters = tmp;
			mSize = rhs.mSize;
		}

		copy(rhs.mCharacters, rhs.mCharacters + mSize, mCharacters);

		mSoftWrapped = rhs.mSoftWrapped;
		mDoubleWidth = rhs.mDoubleWidth;
		mDoubleHeight = rhs.mDoubleHeight;
		mDoubleHeightTop = rhs.mDoubleHeightTop;
	}

	return *this;
}

void MLine::Delete(uint32_t inColumn, uint32_t inWidth, MXTermColor inForeColor, MXTermColor inBackColor)
{
	if (inWidth == 0 or inWidth > mSize)
		inWidth = mSize;

	assert(inColumn < mSize);
	for (uint32_t i = inColumn; i < inWidth - 1; ++i)
		mCharacters[i] = mCharacters[i + 1];
	mCharacters[inWidth - 1] = MChar(inForeColor, inBackColor);
}

void MLine::Insert(uint32_t inColumn, uint32_t inWidth)
{
	if (inWidth == 0 or inWidth > mSize)
		inWidth = mSize;

	for (uint32_t i = inWidth - 1; i > inColumn; --i)
		mCharacters[i] = mCharacters[i - 1];
	mCharacters[inColumn] = ' ';
}

template <class OutputIterator>
void MLine::CopyOut(OutputIterator iter) const
{
	MChar *begin = mCharacters;
	MChar *end = mCharacters + mSize;
	copy(begin, end, iter);
}

void MLine::swap(MLine &p)
{
	std::swap(mCharacters, p.mCharacters);
	std::swap(mSize, p.mSize);
	std::swap(mSoftWrapped, p.mSoftWrapped);
	std::swap(mDoubleWidth, p.mDoubleWidth);
	std::swap(mDoubleHeight, p.mDoubleHeight);
	std::swap(mDoubleHeightTop, p.mDoubleHeightTop);
}

// --------------------------------------------------------------------

MTerminalBuffer::MTerminalBuffer(uint32_t inWidth, uint32_t inHeight, bool inBuffer)
	: mLines(inHeight, MLine(inWidth, kXTermColorNone, kXTermColorNone))
	, mWidth(inWidth)
	, mDirty(false)
	, mBeginLine(0)
	, mBeginColumn(0)
	, mEndLine(0)
	, mEndColumn(0)
	, mBlockSelection(false)
	, mForeColor(kXTermColorNone)
	, mBackColor(kXTermColorNone)
{
	mBufferSize = inBuffer ? MPrefs::GetInteger("buffer-size", 5000) : 0;
}

MTerminalBuffer::~MTerminalBuffer()
{
}

const MLine &MTerminalBuffer::GetLine(int32_t inLine) const
{
	if (inLine >= 0)
	{
		if (static_cast<uint32_t>(inLine) >= mLines.size())
			throw std::runtime_error("Out of range");
		return mLines[inLine];
	}
	else
	{
		inLine = -inLine - 1;
		if (static_cast<uint32_t>(inLine) >= mBuffer.size())
			throw std::runtime_error("Out of range");
		return mBuffer[inLine];
	}
}

void MTerminalBuffer::Resize(uint32_t inWidth, uint32_t inHeight, int32_t &ioAnchorLine)
{
	if (inWidth == mWidth)
	{
		// simple case, shift lines from mBuffer to/from mLines
		while (inHeight > mLines.size() and not mBuffer.empty())
		{
			mLines.insert(mLines.begin(), mBuffer.front());
			mBuffer.pop_front();
		}

		while (inHeight < mLines.size() and not mLines.empty())
		{
			mBuffer.push_front(mLines.front());
			mLines.erase(mLines.begin());
		}

		// this can happen if mBuffer is exhausted or was empty
		while (mLines.size() < inHeight)
			mLines.insert(mLines.begin(), MLine(mWidth, mForeColor, mBackColor));

		assert(mLines.size() == inHeight);
	}
	else
	{
		// for ioAnchorLine, find out what unwrapped line it is on
		int32_t anchor = 0, newAnchorLine = ioAnchorLine;
		for (int32_t i = 0; i < ioAnchorLine + static_cast<int32_t>(mBuffer.size()) and i < static_cast<int32_t>(mBuffer.size()); ++i)
		{
			if (not mBuffer[mBuffer.size() - i - 1].IsSoftWrapped())
				++anchor;
		}

		// first push all lines in the buffer
		for (MLine &line : mLines)
			mBuffer.push_front(line);

		deque<MLine> rewrapped;
		vector<MChar> chars;

		// then pull the lines out of the buffer, starting by the oldest
		do
		{
			if (not mBuffer.empty())
			{
				MLine line = mBuffer.back();
				mBuffer.pop_back();
				line.CopyOut(back_inserter(chars));

				// concatenate lines, if they were softwrapped
				if (line.IsSoftWrapped())
					continue;
			}

			// strip off trailing spaces of old line
			vector<MChar>::iterator e = chars.end();
			while (e != chars.begin() and *(e - 1) == ' ')
				--e;
			if (e != chars.end())
				chars.erase(e, chars.end());

			// if this is the first line and it is empty, ignore it
			if (chars.empty() and rewrapped.empty())
				continue;

			// create a new line
			rewrapped.push_front(MLine(inWidth, mForeColor, mBackColor));

			// store the new anchor position
			if (--anchor == 0)
				newAnchorLine = rewrapped.size();

			// copy over to new
			uint32_t offset = 0;
			while (offset < chars.size())
			{
				MLine &line(rewrapped.front());

				uint32_t n = inWidth;
				if (n + offset >= chars.size())
					n = chars.size() - offset;

				for (uint32_t i = 0; i < n; ++i)
					line[i] = chars[i + offset];

				offset += n;
				if (offset < chars.size())
				{
					line.SetSoftWrapped(true);
					rewrapped.push_front(MLine(inWidth, mForeColor, mBackColor));
				}
			}

			// rinse and repeat until end
			chars.clear();
		} while (not mBuffer.empty());

		mWidth = inWidth;
		mLines = vector<MLine>(inHeight, MLine(inWidth, mForeColor, mBackColor));
		swap(mBuffer, rewrapped);

		// fill the mLines array from the new buffer
		for (vector<MLine>::reverse_iterator line = mLines.rbegin(); line != mLines.rend(); ++line)
		{
			if (mBuffer.empty())
				break;
			*line = mBuffer.front();
			mBuffer.pop_front();
		}

		// finally, calculate new anchorline
		if (ioAnchorLine < 0)
			ioAnchorLine = newAnchorLine - static_cast<int32_t>(mBuffer.size());
	}

	mDirty = true;
}

void MTerminalBuffer::ScrollForward(uint32_t inFromLine, uint32_t inToLine,
	uint32_t inLeftMargin, uint32_t inRightMargin)
{
	if (inFromLine >= inToLine or inToLine >= mLines.size())
		return;

	// check the selection
	ClearSelection();

	if (inLeftMargin == 0 and inRightMargin == mWidth - 1)
	{
		mBuffer.push_front(mLines[inFromLine]);
		while (mBuffer.size() > mBufferSize)
			mBuffer.pop_back();

		for (uint32_t line = inFromLine; line < inToLine; ++line)
			mLines[line].swap(mLines[line + 1]);
	}
	else
	{
		for (uint32_t line = inFromLine; line < inToLine; ++line)
		{
			MLine &a = mLines[line];
			MLine &b = mLines[line + 1];

			for (uint32_t col = inLeftMargin; col <= inRightMargin; ++col)
				swap(a[col], b[col]);
		}
	}

	MLine &line = mLines[inToLine];
	for (uint32_t c = inLeftMargin; c <= inRightMargin; ++c)
		line[c] = MChar(mForeColor, mBackColor);
	line.SetSoftWrapped(false);

	mDirty = true;
}

void MTerminalBuffer::ScrollBackward(uint32_t inFromLine, uint32_t inToLine,
	uint32_t inLeftMargin, uint32_t inRightMargin)
{
	if (inFromLine >= inToLine or inToLine >= mLines.size())
		return;

	// check the selection
	ClearSelection();

	if (inLeftMargin == 0 and inRightMargin == mWidth - 1)
	{
		for (uint32_t line = inToLine; line > inFromLine; --line)
			swap(mLines[line], mLines[line - 1]);
	}
	else
	{
		for (uint32_t line = inToLine; line > inFromLine; --line)
		{
			MLine &a = mLines[line];
			MLine &b = mLines[line - 1];

			for (uint32_t col = inLeftMargin; col <= inRightMargin; ++col)
				swap(a[col], b[col]);
		}
	}

	MLine &line = mLines[inFromLine];
	for (uint32_t c = inLeftMargin; c <= inRightMargin; ++c)
		line[c] = MChar(mForeColor, mBackColor);
	line.SetSoftWrapped(false);

	mDirty = true;
}

void MTerminalBuffer::Clear()
{
	mBuffer.clear();
	EraseDisplay(0, 0, 2, false);
}

void MTerminalBuffer::SetCharacter(uint32_t inLine, uint32_t inColumn, unicode inChar, MStyle inStyle)
{
	if (inLine >= mLines.size())
		return;

	MLine &line(mLines[inLine]);
	line[inColumn] = MChar(inChar, inStyle);

	mDirty = true;
}

void MTerminalBuffer::ReverseFlag(uint32_t inFromLine, uint32_t inFromColumn,
	uint32_t inToLine, uint32_t inToColumn, MCharStyle inFlags)
{
	for (uint32_t li = inFromLine; li <= inToLine; ++li)
	{
		if (li >= mLines.size())
			break;

		MLine &line(mLines[li]);

		for (uint32_t ci = inFromColumn; ci <= inToColumn; ++ci)
		{
			if (ci >= mWidth)
				break;

			line[ci].ReverseFlag(inFlags);
		}
	}

	mDirty = true;
}

void MTerminalBuffer::ChangeFlags(uint32_t inFromLine, uint32_t inFromColumn,
	uint32_t inToLine, uint32_t inToColumn, uint32_t inMode)
{
	for (uint32_t li = inFromLine; li <= inToLine; ++li)
	{
		if (li >= mLines.size())
			break;

		MLine &line(mLines[li]);

		for (uint32_t ci = inFromColumn; ci <= inToColumn; ++ci)
		{
			if (ci >= mWidth)
				break;

			line[ci].ChangeFlags(inMode);
		}
	}

	mDirty = true;
}

void MTerminalBuffer::SetLineDoubleWidth(uint32_t inLine)
{
	if (inLine >= mLines.size())
		return;

	mLines[inLine].SetDoubleWidth();
	mDirty = true;
}

void MTerminalBuffer::SetLineDoubleHeight(uint32_t inLine, bool inTop)
{
	if (inLine >= mLines.size())
		return;

	mLines[inLine].SetDoubleHeight(inTop);
	mDirty = true;
}

void MTerminalBuffer::SetLineSingleWidth(uint32_t inLine)
{
	if (inLine >= mLines.size())
		return;

	mLines[inLine].SetSingleWidth();
	mDirty = true;
}

void MTerminalBuffer::EraseDisplay(uint32_t inLine, uint32_t inColumn, uint32_t inMode, bool inSelective)
{
	for (uint32_t l = 0; l < mLines.size(); ++l)
	{
		MLine &line(mLines[l]);
		line.SetSoftWrapped(false);
		line.SetSingleWidth();

		for (uint32_t c = 0; c < mWidth; ++c)
		{
			if (line[c] & kProtected or (inSelective and line[c] & kUnerasable))
				continue;

			switch (inMode)
			{
				case 0:
					if (l > inLine or (l == inLine and c >= inColumn))
						line[c] = MChar(mForeColor, mBackColor);
					break;
				case 1:
					if (l < inLine or (l == inLine and c <= inColumn))
						line[c] = MChar(mForeColor, mBackColor);
					break;
				case 2: line[c] = MChar(mForeColor, mBackColor); break;
			}
		}
	}

	mDirty = true;
}

void MTerminalBuffer::EraseLine(uint32_t inLine, uint32_t inColumn, uint32_t inMode, bool inSelective)
{
	if (inLine >= mLines.size())
		return;

	MLine &line(mLines[inLine]);
	line.SetSoftWrapped(false);
	//	line.SetSingleWidth();

	uint32_t cf = 0, ct = mWidth;
	switch (inMode)
	{
		case 0: cf = inColumn; break;
		case 1: ct = inColumn + 1; break;
	}

	for (uint32_t c = cf; c < ct; ++c)
	{
		if (line[c] & kProtected or (inSelective and line[c] & kUnerasable))
			continue;
		line[c] = MChar(mForeColor, mBackColor);
	}

	mDirty = true;
}

void MTerminalBuffer::EraseCharacter(uint32_t inLine, uint32_t inColumn, uint32_t inCount)
{
	if (inLine >= mLines.size())
		return;

	if (inCount >= mWidth - inColumn)
		inCount = mWidth - inColumn;

	MLine &line(mLines[inLine]);

	for (uint32_t c = inColumn; c < inColumn + inCount and c < mWidth; ++c)
	{
		if (line[c] & kProtected)
			continue;
		line[c] = MChar(mForeColor, mBackColor);
	}

	mDirty = true;
}

void MTerminalBuffer::DeleteCharacter(uint32_t inLine, uint32_t inColumn, uint32_t inWidth)
{
	if (inLine >= mLines.size())
		return;

	mLines[inLine].Delete(inColumn, inWidth, mForeColor, mBackColor);

	mDirty = true;
}

void MTerminalBuffer::InsertCharacter(uint32_t inLine, uint32_t inColumn, uint32_t inWidth)
{
	if (inLine >= mLines.size())
		return;

	mLines[inLine].Insert(inColumn, inWidth);

	mDirty = true;
}

void MTerminalBuffer::WrapLine(uint32_t inLine)
{
	if (inLine < mLines.size())
		mLines[inLine].SetSoftWrapped(true);
}

void MTerminalBuffer::FillWithE()
{
	for (uint32_t l = 0; l < mLines.size(); ++l)
	{
		MLine &line(mLines[l]);
		for (uint32_t column = 0; column < mWidth; ++column)
			line[column] = MChar('E', MStyle(mForeColor, mBackColor));
	}

	mDirty = true;
}

bool MTerminalBuffer::IsSelectionEmpty() const
{
	return mBeginLine == mEndLine and mBeginColumn == mEndColumn;
}

bool MTerminalBuffer::IsSelectionBlock() const
{
	return mBlockSelection;
}

void MTerminalBuffer::GetSelectionBegin(int32_t &outLine, int32_t &outColumn) const
{
	outLine = mBeginLine;
	outColumn = mBeginColumn;
}

void MTerminalBuffer::GetSelectionEnd(int32_t &outLine, int32_t &outColumn) const
{
	outLine = mEndLine;
	outColumn = mEndColumn;
}

void MTerminalBuffer::GetSelection(int32_t &outBeginLine, int32_t &outBeginColumn,
	int32_t &outEndLine, int32_t &outEndColumn, bool &outIsBlock) const
{
	outBeginLine = mBeginLine;
	outBeginColumn = mBeginColumn;
	outEndLine = mEndLine;
	outEndColumn = mEndColumn;
	outIsBlock = mBlockSelection;
}

void MTerminalBuffer::SetSelection(int32_t inBeginLine, int32_t inBeginColumn,
	int32_t inEndLine, int32_t inEndColumn, bool inBlock)
{
	assert(inBeginLine < inEndLine or (inBeginLine == inEndLine and inBeginColumn <= inEndColumn));

	mBeginLine = inBeginLine;
	mBeginColumn = inBeginColumn;
	mEndLine = inEndLine;
	mEndColumn = inEndColumn;
	mBlockSelection = inBlock;
}

void MTerminalBuffer::SelectAll()
{
	mBeginLine = -BufferedLines();
	mBeginColumn = 0;
	mEndLine = mLines.size() - 1;
	mEndColumn = mWidth;
	mBlockSelection = false;
}

void MTerminalBuffer::ClearSelection()
{
	mBeginLine = mBeginColumn = mEndLine = mEndColumn = 0;
	mBlockSelection = false;
}

namespace
{

// a custom wordbreak class routine
// since we expect slightly different behaviour for word selection in terminals
// and we don't have CR/LF/TAB in the terminal buffer.

enum TerminalWordBreakClass
{
	eTWB_Sep,
	eTWB_Let,
	eTWB_Com,
	eTWB_Hira,
	eTWB_Kata,
	eTWB_Han,
	eTWB_Other
};

TerminalWordBreakClass GetTerminalWordBreakClass(unicode inUnicode)
{
	TerminalWordBreakClass result;

	switch (GetProperty(inUnicode))
	{
		case kLETTER:
		case kNUMBER:
			if (inUnicode >= 0x003040 and inUnicode <= 0x00309f)
				result = eTWB_Hira;
			else if (inUnicode >= 0x0030a0 and inUnicode <= 0x0030ff)
				result = eTWB_Kata;
			else if (inUnicode >= 0x004e00 and inUnicode <= 0x009fff)
				result = eTWB_Han;
			else if (inUnicode >= 0x003400 and inUnicode <= 0x004DFF)
				result = eTWB_Han;
			else if (inUnicode >= 0x00F900 and inUnicode <= 0x00FAFF)
				result = eTWB_Han;
			else
				result = eTWB_Let;
			break;

		case kCOMBININGMARK:
			result = eTWB_Com;
			break;

		case kSEPARATOR:
			result = eTWB_Sep;
			break;

		case kPUNCTUATION:
			switch (inUnicode) // special case
			{
				case '.':
				case '-':
				case '/':
					result = eTWB_Com;
					break;

				default:
					result = eTWB_Other;
					break;
			}
			break;

		default:
			result = eTWB_Other;
			break;
	}

	return result;
}

} // namespace

void MTerminalBuffer::FindWord(int32_t inLine, int32_t inColumn,
	int32_t &outLine1, int32_t &outColumn1, int32_t &outLine2, int32_t &outColumn2)
{
	// sensible defaults:
	outLine1 = outLine2 = inLine;
	outColumn1 = outColumn2 = inColumn;

	// begin by finding an unwrapped line, search backward until we've hit a newline
	for (;;)
	{
		if (inLine > 0)
		{
			if (not mLines[inLine - 1].IsSoftWrapped())
				break;
			--inLine;
			inColumn += mWidth;
			continue;
		}

		if (-inLine + 1 < static_cast<int32_t>(mBuffer.size()))
		{
			if (not mBuffer[-inLine + 1].IsSoftWrapped())
				break;
			--inLine;
			inColumn += mWidth;
		}

		break;
	}

	// collect the unicode string for this line
	vector<MChar> s;
	int32_t lineNr = inLine;
	for (;;)
	{
		const MLine &line = GetLine(lineNr);
		++lineNr;

		line.CopyOut(back_inserter(s));
		if (not line.IsSoftWrapped() or lineNr >= static_cast<int32_t>(mLines.size()))
			break;
	}

	// strip off trailing white space
	while (not s.empty() and (wchar_t) s.back() == ' ')
		s.pop_back();

	if (inColumn > static_cast<int32_t>(s.size()))
		inColumn = static_cast<int32_t>(s.size());

	const int8_t
		kNextWordBreakStateTable[5][7] = {
			//	Sep	Let	Com	Hir	Kat	Han	Other		State
			{ 0, 1, 1, 2, 3, 4, 0 },      //	0
			{ -1, 1, 1, -1, -1, -1, -1 }, //	1
			{ -1, -1, 2, 2, -1, -1, -1 }, //	2
			{ -1, -1, 3, 2, 3, -1, -1 },  //	3
			{ -1, -1, 4, 2, -1, 4, -1 },  //	4
		},
		kPrevWordBreakStateTable[6][7] = {
			//	Sep	Let	Com	Hir	Kat	Han	Other		State
			{ 0, 1, 2, 3, 4, 5, 0 },      //	0
			{ -1, 2, 1, 3, 4, 5, -1 },    //	1
			{ -1, 2, 2, -1, -1, -1, -1 }, //	2
			{ -1, -1, 3, 3, 4, 5, -1 },   //	3
			{ -1, -1, 4, -1, 4, -1, -1 }, //	4
			{ -1, -1, 5, -1, -1, 5, -1 }, //	5
		};

	// now find the word position, start by going right
	int32_t nextColumn, prevColumn, column = inColumn - 1;
	int8_t state = 0;
	while (state >= 0)
	{
		++column;
		nextColumn = column;
		if (nextColumn >= static_cast<int32_t>(s.size()))
			break;
		TerminalWordBreakClass cl = GetTerminalWordBreakClass(s[column]);
		state = kNextWordBreakStateTable[uint8_t(state)][cl];
	}

	// then go back
	column = nextColumn;
	state = 0;
	while (state >= 0)
	{
		prevColumn = column;
		if (column == 0)
			break;
		--column;
		TerminalWordBreakClass cl = GetTerminalWordBreakClass(s[column]);
		state = kPrevWordBreakStateTable[uint8_t(state)][cl];
	}

	// check if we did find anything
	if (prevColumn < inColumn and nextColumn > inColumn)
	{
		outColumn1 = prevColumn;
		outColumn2 = nextColumn;
		outLine1 = outLine2 = inLine; // we now have to correct for the wrapping

		while (outColumn1 > static_cast<int32_t>(mWidth))
		{
			outColumn1 -= mWidth;
			++outLine1;
		}

		while (outColumn2 > static_cast<int32_t>(mWidth))
		{
			outColumn2 -= mWidth;
			++outLine2;
		}
	}
}

string MTerminalBuffer::GetSelectedText() const
{
	string result;

	for (int32_t l = mBeginLine; l <= mEndLine; ++l)
	{
		const MLine &line(GetLine(l));

		int32_t c1 = 1, c2 = 0;
		if (mBlockSelection)
		{
			c1 = mBeginColumn;
			c2 = mEndColumn;
			if (c1 > c2)
				swap(c1, c2);
		}
		else
		{
			if (l == mBeginLine or mBlockSelection)
				c1 = mBeginColumn;
			else
				c1 = 0;
			if (l == mEndLine or mBlockSelection)
				c2 = mEndColumn;
			else
				c2 = mWidth;
		}

		if (not mBlockSelection and (l == mEndLine or not line.IsSoftWrapped()))
		{
			while (c2 > c1 and line[c2 - 1] == ' ')
				--c2;
		}

		auto iter = back_inserter(result);

		for (int32_t c = c1; c < c2; ++c)
			MEncodingTraits<kEncodingUTF8>::WriteUnicode(iter, (unicode)line[c]);

		if (mBlockSelection or (l != mEndLine and not line.IsSoftWrapped()))
			result += '\n';
	}

	return result;
}

unicode MTerminalBuffer::GetChar(uint32_t inOffset, bool inToLower) const
{
	int32_t line = inOffset / mWidth;
	int32_t column = inOffset % mWidth;

	assert(line >= 0 and line < static_cast<int32_t>(mBuffer.size() + mLines.size()));

	unicode result;
	if (line >= static_cast<int32_t>(mBuffer.size()))
		result = mLines[line - mBuffer.size()][column];
	else
		result = mBuffer[mBuffer.size() - line - 1][column];

	if (inToLower)
		result = ToLower(result);

	return result;
}

// --------------------------------------------------------------------
// Find is implemented using the Rabin-Karp algorithm
// Implement forward and backward searching in separate routines to
// keep code readable.

bool MTerminalBuffer::FindNext(int32_t &ioLine, int32_t &ioColumn, const string &inWhat,
	bool inIgnoreCase, bool inWrapAround)
{
	// q is a rather large prime, d is the alphabet size (of Unicode)
	const int64_t q = 33554393, d = 1114112;

	// convert the search string to unicode's
	vector<unicode> what;
	for (string::const_iterator w = inWhat.begin(); w != inWhat.end();)
	{
		unicode ch;
		uint32_t l;
		MEncodingTraits<kEncodingUTF8>::ReadUnicode(w, l, ch);
		w += l;

		if (inIgnoreCase)
			ch = ToLower(ch);

		what.push_back(ch);
	}

	// M is the length of the search string
	int32_t M = what.size();

	// We're looking forward. N is the length of the remaining characters in the buffer
	int32_t lineCount = static_cast<int32_t>(mBuffer.size() + mLines.size());
	int32_t line = static_cast<int32_t>(mBuffer.size()) + ioLine;
	int32_t N = (lineCount - line) * mWidth - ioColumn;
	int32_t O = lineCount * mWidth - N; // offset from start for ioLine/ioColumn

	// sanity check
	if (M >= N)
		return false;

	int64_t dM = 1, h1 = 0, h2 = 0;
	int32_t i;

	for (i = 1; i < M; ++i)
		dM = (d * dM) % q;

	for (i = 0; i < M; ++i)
	{
		h1 = (h1 * d + what[i]) % q;
		h2 = (h2 * d + GetChar(O + i, inIgnoreCase)) % q;
	}

	for (i = 0; i < N - M; ++i)
	{
		if (h1 == h2) // hashes match, make sure the characters are the same too (collision?)
		{
			bool found = true;
			for (int j = 0; found and j < M; ++j)
				found = what[j] == GetChar(O + i + j, inIgnoreCase);
			if (found)
				break;
		}

		h2 = (h2 + d * q - GetChar(O + i, inIgnoreCase) * dM) % q;
		h2 = (h2 * d + GetChar(O + i + M, inIgnoreCase)) % q;
	}

	bool result = false;
	if (i < N - M)
	{
		ioLine = (O + i) / mWidth - static_cast<int32_t>(mBuffer.size());
		ioColumn = (O + i) % mWidth;
		result = true;
	}
	else if (inWrapAround and (ioLine > -static_cast<int32_t>(mBuffer.size()) or ioColumn > 0))
	{
		int32_t line = -static_cast<int32_t>(mBuffer.size()), column = 0;
		result = FindNext(line, column, inWhat, inIgnoreCase, false);
		if (result and (line > ioLine or (line == ioLine and column + static_cast<int32_t>(what.size()) >= ioColumn)))
			result = false;
		else
		{
			ioLine = line;
			ioColumn = column;
		}
	}

	return result;
}

bool MTerminalBuffer::FindPrevious(int32_t &ioLine, int32_t &ioColumn, const string &inWhat,
	bool inIgnoreCase, bool inWrapAround)
{
	// q is a rather large prime, d is the alphabet size (of Unicode)
	const int64_t q = 33554393, d = 1114112;

	// convert the search string to unicode's
	vector<unicode> what;
	for (string::const_iterator w = inWhat.begin(); w != inWhat.end();)
	{
		unicode ch;
		uint32_t l;
		MEncodingTraits<kEncodingUTF8>::ReadUnicode(w, l, ch);
		w += l;

		if (inIgnoreCase)
			ch = ToLower(ch);

		what.push_back(ch);
	}
	reverse(what.begin(), what.end());

	// M is the length of the search string
	int32_t M = what.size();

	// We're looking backward now. N is the length of the characters in the buffer
	// up until the point where we start.
	int32_t lineCount = static_cast<int32_t>(mBuffer.size() + mLines.size());
	int32_t line = static_cast<int32_t>(mBuffer.size()) + ioLine;
	int32_t N = line * mWidth + ioColumn;

	// sanity check
	if (M >= N)
		return false;

	int64_t dM = 1, h1 = 0, h2 = 0;
	int32_t i;

	for (i = 1; i < M; ++i)
		dM = (d * dM) % q;

	for (i = 0; i < M; ++i)
	{
		h1 = (h1 * d + what[i]) % q;
		h2 = (h2 * d + GetChar(N - i, inIgnoreCase)) % q;
	}

	for (i = 0; i < N - M; ++i)
	{
		if (h1 == h2) // hashes match, make sure the characters are the same too (collision?)
		{
			bool found = true;
			for (int j = 0; found and j < M; ++j)
				found = what[j] == GetChar(N - (i + j), inIgnoreCase);
			if (found)
				break;
		}

		h2 = (h2 + d * q - GetChar(N - i, inIgnoreCase) * dM) % q;
		h2 = (h2 * d + GetChar(N - (i + M), inIgnoreCase)) % q;
	}

	bool result = false;
	if (i < N - M)
	{
		ioLine = (N - i - M + 1) / mWidth - static_cast<int32_t>(mBuffer.size());
		ioColumn = (N - i - M + 1) % mWidth;
		result = true;
	}
	else if (inWrapAround and (ioLine < lineCount or ioColumn < static_cast<int32_t>(mWidth)))
	{
		int32_t line = lineCount - mBuffer.size() - 1, column = mWidth - 1;
		result = FindPrevious(line, column, inWhat, inIgnoreCase, false);
		if (result and (line < ioLine or (line == ioLine and column + static_cast<int32_t>(what.size()) <= ioColumn)))
			result = false;
		else
		{
			ioLine = line;
			ioColumn = column;
		}
	}

	return result;
}
