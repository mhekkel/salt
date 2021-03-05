//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "MColor.hpp"

using namespace std;

const MColor
	kBlack("#000000"),
	kWhite("#ffffff"),
	kNoteColor("#206cff"),
	kWarningColor("#ffeb17"),
	kErrorColor("#ff4811"),
	kSelectionColor("#f3bb6b");

MColor
	gSelectionColor = kSelectionColor;

MColor::MColor()
	: red(0), green(0), blue(0)
{
}

MColor::MColor(
	const MColor&	inOther)
{
	red = inOther.red;
	green = inOther.green;
	blue = inOther.blue;
}

//MColor::MColor(
//	const GdkColor&	inOther)
//{
//	red = inOther.red >> 8;
//	green = inOther.green >> 8;
//	blue = inOther.blue >> 8;
//}

MColor::MColor(
	const char*		inHex)
{
	hex(inHex);
}

MColor::MColor(
	const string&	inHex)
{
	hex(inHex);
}

MColor::MColor(
	uint8			inRed,
	uint8			inGreen,
	uint8			inBlue)
{
	red = inRed;
	green = inGreen;
	blue = inBlue;
}

MColor::MColor(
	float			inRed,
	float			inGreen,
	float			inBlue)
{
	red = static_cast<uint8>(inRed * 255);
	green = static_cast<uint8>(inGreen * 255);
	blue = static_cast<uint8>(inBlue * 255);
}

MColor& MColor::operator=(
	const MColor&	inOther)
{
	red = inOther.red;
	green = inOther.green;
	blue = inOther.blue;
	return *this;
}

//MColor::operator GdkColor() const
//{
//	GdkColor result = {};
//	result.red = red << 8 | red;
//	result.green = green << 8 | green;
//	result.blue = blue << 8 | blue;
//	return result;
//}

string MColor::hex() const
{
	stringstream s;
	
	s.setf(ios_base::hex, ios_base::basefield);
	
	s << '#'
		<< setw(2) << setfill('0') << static_cast<uint32>(red)
		<< setw(2) << setfill('0') << static_cast<uint32>(green)
		<< setw(2) << setfill('0') << static_cast<uint32>(blue);
	
	return s.str();
}

void MColor::hex(
	const string&	inHex)
{
	const char* h = inHex.c_str();
	uint32 l = inHex.length();
	
	if (*h == '#')
		++h, --l;
	
	if (l == 6)
	{
		uint32 v = strtoul(h, nullptr, 16);
		red =		(v >> 16) & 0x0ff;
		green = 	(v >>  8) & 0x0ff;
		blue =		(v >>  0) & 0x0ff;
	}	
	else if (inHex.length() == 4 and inHex[0] == '#')
	{
		uint32 v = strtoul(inHex.c_str() + 1, nullptr, 16);
		red =	(v >> 8) & 0x0f;	red = (red << 4) | red;
		green =	(v >> 4) & 0x0f;	green = (green << 4) | green;
		blue =	(v >> 0) & 0x0f;	blue = (blue << 4) | blue;
	}
}

MColor MColor::Disable(const MColor& inBackColor) const
{
	MColor r;
	r.red	= static_cast<uint8>((red + inBackColor.red) / 2);
	r.green = static_cast<uint8>((green + inBackColor.green) / 2);
	r.blue	= static_cast<uint8>((blue + inBackColor.blue) / 2);
	return r;
}

MColor MColor::Disable(const MColor& inBackColor, float inScale) const
{
	float r = (red / 255.f), g = (green / 255.f), b = (blue / 255.f);
	float rb = (inBackColor.red / 255.f), gb = (inBackColor.green / 255.f), bb = (inBackColor.blue / 255.f);
	
	return MColor(
		(1 - inScale) * r + inScale * (r + rb) / 2,
		(1 - inScale) * g + inScale * (g + gb) / 2,
		(1 - inScale) * b + inScale * (b + bb) / 2);
}

MColor MColor::Distinct(const MColor& inBackColor) const
{
	const uint32 kDistinctColorTresholdSquare = 10000;

	// Does a simple distance based color comparison, returns an
	// inverse color if colors close enough
	uint32 redDelta = (uint32)red - (uint32)inBackColor.red;
	uint32 greenDelta = (uint32)green - (uint32)inBackColor.green;
	uint32 blueDelta = (uint32)blue - (uint32)inBackColor.blue;

	if (redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta > kDistinctColorTresholdSquare)
		return *this;

	MColor result;
	result.red =	static_cast<uint8> (255 - red); 
	result.green =	static_cast<uint8> (255 - green); 
	result.blue =	static_cast<uint8> (255 - blue);
	
	return result;
}

MColor MColor::Bleach(float inBleachFactor) const
{
	float r = (red / 255.f), g = (green / 255.f), b = (blue / 255.f);
	
	float h, s, v;
	rgb2hsv(r, g, b, h, s, v);
	
	s = (1 - inBleachFactor) * s;
	
	if (v < 0.5)
		v = inBleachFactor + (1 - inBleachFactor) * v;
	else
		v = (1 - inBleachFactor) * v;
	
	hsv2rgb(h, s, v, r, g, b);
	
	return MColor(r, g, b);
}

ostream& operator<<(ostream& os, const MColor& inColor)
{
	ios_base::fmtflags flags = os.setf(ios_base::hex, ios_base::basefield);
	
	os << '#'
		<< setw(2) << setfill('0') << static_cast<uint32>(inColor.red)
		<< setw(2) << setfill('0') << static_cast<uint32>(inColor.green)
		<< setw(2) << setfill('0') << static_cast<uint32>(inColor.blue);
	
	os.setf(flags);
	return os;
}

// --------------------------------------------------------------------

void rgb2hsv(float r, float g, float b, float& h, float& s, float& v)
{
	float cmin, cmax, delta;
	
	cmax = max(r, max(g, b));
	cmin = min(r, min(g, b));
	delta = cmax - cmin;
	
	v = cmax;
	s = cmax ? delta / cmax : 0.0f;

	if (s == 0.0)
		h = 0;
	else
	{
		if (r == cmax)
			h = (g - b) / delta;
		else if (g == cmax)
			h = 2 + (b - r) / delta;
		else if (b == cmax)
			h = 4 + (r - g) / delta;
		h /= 6.0;
	}
} /* rgb2hsv */

void hsv2rgb(float h, float s, float v, float& r, float& g, float& b)
{
	float A, B, C, F;
	int i;
	
	if (s == 0.0)
		r = g = b = v;
	else
	{
		if (h >= 1.0 || h < 0.0)
			h = 0.0;
		h *= 6.0;
		i = (int)floor(h);
		F = h - i;
		A = v * (1 - s);
		B = v * (1 - (s * F));
		C = v * (1 - (s * (1 - F)));
		switch (i)
		{
			case 0:	r = v; g = C; b = A; break;
			case 1:	r = B; g = v; b = A; break;
			case 2:	r = A; g = v; b = C; break;
			case 3:	r = A; g = B; b = v; break;
			case 4:	r = C; g = A; b = v; break;
			case 5:	r = v; g = A; b = B; break;
		}
	}
} /* hsv2rgb */

