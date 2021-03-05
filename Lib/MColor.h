//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <iostream>
#include <iomanip>

struct MColor
{
  public:
	uint8		red;
	uint8		green;
	uint8		blue;

				MColor();
				MColor(const MColor& inOther);
				//MColor(// const GdkColor& inColor);
				MColor(const char* inHex);
				MColor(const std::string& inHex);
				MColor(uint8 inRed, uint8 inGreen, uint8 inBlue);
				MColor(float inRed, float inGreen, float inBlue);
	MColor&		operator=(const MColor& inOther);

	MColor		Disable(const MColor& inBackColor) const;
	MColor		Disable(const MColor& inBackColor, float inScale) const;
	MColor		Distinct(const MColor& inBackColor) const;
	
	// bleach out a color (toward white, 0 <= factor <= 1)
	MColor		Bleach(float inBleachFactor) const;	
				//operator GdkColor() const;

	bool		operator==(const MColor& rhs) const
				{
					return red == rhs.red and green == rhs.green and blue == rhs.blue;
				}

	bool		operator!=(const MColor& rhs) const
				{
					return red != rhs.red or green != rhs.green or blue != rhs.blue;
				}

	std::string	hex() const;
	void		hex(const std::string& inHex);
};

extern const MColor
	kWhite,
	kBlack,
	kNoteColor,
	kWarningColor,
	kErrorColor,
	kSelectionColor,
	kDialogBackgroundColor;

std::ostream& operator<<(std::ostream& os, const MColor& inColor);

void rgb2hsv(float r, float g, float b, float& h, float& s, float& v);
void hsv2rgb(float h, float s, float v, float& r, float& g, float& b);

