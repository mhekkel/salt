//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// MTypes.h

#pragma once

#include <cstdint>

typedef uint32_t unicode;

struct MRect
{
	int32_t x, y;
	int32_t width, height;

	MRect();

	MRect(const MRect &inRHS);

	MRect(int32_t inX, int32_t inY, int32_t inWidth, int32_t inHeight);

	MRect(float inX, float inY, float inWidth, float inHeight);

	bool Intersects(const MRect &inRHS) const;

	bool ContainsPoint(int32_t inX, int32_t inY) const;

	void PinPoint(int32_t &ioX, int32_t &ioY) const;

	void InsetBy(int32_t inDeltaX, int32_t inDeltaY);

	bool empty() const;
	operator bool() const;

	// Intersection
	MRect operator&(const MRect &inRegion);
	MRect &operator&=(const MRect &inRegion);

	// Union
	MRect operator|(const MRect &inRegion);
	MRect &operator|=(const MRect &inRegion);

	bool operator==(const MRect &inRect) const;
	bool operator!=(const MRect &inRect) const;
};

class MRegion
{
public:
	MRegion();
	MRegion(const MRect &inRect);
	MRegion(const MRegion &inRegion);
	~MRegion();

	MRegion &operator=(const MRegion &inRegion);

	// Intersection
	MRegion operator&(const MRegion &inRegion) const;
	MRegion operator&(const MRect &inRect) const;

	MRegion &operator&=(const MRegion &inRegion);
	MRegion &operator&=(const MRect &inRect);

	// Union
	MRegion operator|(const MRegion &inRegion) const;
	MRegion operator|(const MRect &inRect) const;

	MRegion &operator|=(const MRegion &inRegion);
	MRegion &operator|=(const MRect &inRect);

	// test for empty region
	operator bool() const;

	void OffsetBy(int32_t inX, int32_t inY);
	bool ContainsPoint(int32_t inX, int32_t inY) const;
	MRect GetBounds() const;

private:
	struct MRegionImpl *mImpl;
};

enum MTriState
{
	eTriStateOn,
	eTriStateOff,
	eTriStateLatent
};

enum MDirection
{
	kDirectionForward = 1,
	kDirectionBackward = -1
};

enum MScrollMessage
{
	kScrollNone,
	kScrollToStart,
	kScrollToEnd,
	kScrollToCaret,
	kScrollToSelection,
	kScrollToThumb,
	kScrollCenterSelection,
	kScrollLineUp,
	kScrollLineDown,
	kScrollPageUp,
	kScrollPageDown,
	kScrollForKiss,
	kScrollReturnAfterKiss,
	kScrollForDiff,
	kScrollToPC
};

enum
{ // modifier keys
	kCmdKey = 1 << 0,
	kShiftKey = 1 << 1,
	kOptionKey = 1 << 2,
	kControlKey = 1 << 3,
	kAlphaLock = 1 << 4,
	kNumPad = 1 << 5,
	kRightShiftKey = 1 << 6,
	kRightOptionKey = 1 << 7,
	kRightControlKey = 1 << 8
};

enum MKeyCode
{ // key codes
	kNullKeyCode = 0,
	kHomeKeyCode = 1,
	kCancelKeyCode = 3,
	kEndKeyCode = 4,
	kInsertKeyCode = 5,
	kBellKeyCode = 7,
	kBackspaceKeyCode = 8,
	kTabKeyCode = 9,
	kLineFeedKeyCode = 10,
	kVerticalTabKeyCode = 11,
	kPageUpKeyCode = 11,
	kFormFeedKeyCode = 12,
	kPageDownKeyCode = 12,
	kReturnKeyCode = 13,
	kFunctionKeyKeyCode = 16,
	kPauseKeyCode = 19,
	kEscapeKeyCode = 27,
	kClearKeyCode = 27,
	kLeftArrowKeyCode = 28,
	kRightArrowKeyCode = 29,
	kUpArrowKeyCode = 30,
	kDownArrowKeyCode = 31,
	kSpaceKeyCode = 32,
	kDeleteKeyCode = 127,
	kDivideKeyCode = 111,
	kMultiplyKeyCode = 106,
	kSubtractKeyCode = 109,
	kNumlockKeyCode = 0x0090,
	kF1KeyCode = 0x0101,
	kF2KeyCode = 0x0102,
	kF3KeyCode = 0x0103,
	kF4KeyCode = 0x0104,
	kF5KeyCode = 0x0105,
	kF6KeyCode = 0x0106,
	kF7KeyCode = 0x0107,
	kF8KeyCode = 0x0108,
	kF9KeyCode = 0x0109,
	kF10KeyCode = 0x010a,
	kF11KeyCode = 0x010b,
	kF12KeyCode = 0x010c,
	kF13KeyCode = 0x010d,
	kF14KeyCode = 0x010e,
	kF15KeyCode = 0x010f,
	kF16KeyCode = 0x0110,
	kF17KeyCode = 0x0111,
	kF18KeyCode = 0x0112,
	kF19KeyCode = 0x0113,
	kF20KeyCode = 0x0114,

	// my own pseudo key codes
	kEnterKeyCode = 0x0201,
};

extern const char kHexChars[];

template <class T>
class value_changer
{
public:
	value_changer(T &inVariable, const T inTempValue);
	~value_changer();

private:
	T &mVariable;
	T mValue;
};

template <class T>
inline value_changer<T>::value_changer(T &inVariable, const T inTempValue)
	: mVariable(inVariable), mValue(inVariable)
{
	mVariable = inTempValue;
}

template <class T>
inline value_changer<T>::~value_changer()
{
	mVariable = mValue;
}

// --------------------------------------------------------------------
// inlines

inline MRect::MRect()
	: x(0), y(0), width(0), height(0) {}

inline MRect::MRect(const MRect &inRHS)
	: x(inRHS.x), y(inRHS.y), width(inRHS.width), height(inRHS.height)
{
}

//inline
//MRect::MRect(// const GdkRectangle& inRHS)
//	: x(inRHS.x)
//	, y(inRHS.y)
//	, width(inRHS.width)
//	, height(inRHS.height)
//{
//}

inline MRect::MRect(int32_t inX, int32_t inY, int32_t inWidth, int32_t inHeight)
	: x(inX), y(inY), width(inWidth), height(inHeight)
{
}

inline MRect::MRect(float inX, float inY, float inWidth, float inHeight)
	: x(static_cast<int32_t>(inX)), y(static_cast<int32_t>(inY)), width(static_cast<int32_t>(inWidth)), height(static_cast<int32_t>(inHeight))
{
}

inline bool MRect::Intersects(const MRect &inRHS) const
{
	return x < inRHS.x + inRHS.width and
		   x + width > inRHS.x and
		   y < inRHS.y + inRHS.height and
		   y + height > inRHS.y;
}

inline bool MRect::ContainsPoint(int32_t inX, int32_t inY) const
{
	return x <= inX and x + width > inX and
		   y <= inY and y + height > inY;
}

inline bool MRect::operator==(const MRect &rhs) const
{
	return x == rhs.x and y == rhs.y and
		   width == rhs.width and height == rhs.height;
}

inline bool MRect::operator!=(const MRect &rhs) const
{
	return x != rhs.x or y != rhs.y or
		   width != rhs.width or height != rhs.height;
}
