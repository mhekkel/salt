//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

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
	: red(0)
	, green(0)
	, blue(0)
{
}

MColor::MColor(
	const MColor &inOther)
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

MColor::MColor(const char *inHex)
{
	hex(inHex);
}

MColor::MColor(const string &inHex)
{
	hex(inHex);
}

MColor::MColor(
	uint8_t inRed,
	uint8_t inGreen,
	uint8_t inBlue)
{
	red = inRed;
	green = inGreen;
	blue = inBlue;
}

MColor::MColor(float inRed, float inGreen, float inBlue)
{
	red = static_cast<uint8_t>(inRed * 255);
	green = static_cast<uint8_t>(inGreen * 255);
	blue = static_cast<uint8_t>(inBlue * 255);
}

MColor &MColor::operator=(const MColor &inOther)
{
	red = inOther.red;
	green = inOther.green;
	blue = inOther.blue;
	return *this;
}

string MColor::hex() const
{
	stringstream s;

	s.setf(ios_base::hex, ios_base::basefield);

	s << '#'
	  << setw(2) << setfill('0') << static_cast<uint32_t>(red)
	  << setw(2) << setfill('0') << static_cast<uint32_t>(green)
	  << setw(2) << setfill('0') << static_cast<uint32_t>(blue);

	return s.str();
}

void MColor::hex(const string &inHex)
{
	const char *h = inHex.c_str();
	auto l = inHex.length();

	if (*h == '#')
		++h, --l;

	if (l == 6)
	{
		uint32_t v = strtoul(h, nullptr, 16);
		red = (v >> 16) & 0x0ff;
		green = (v >> 8) & 0x0ff;
		blue = (v >> 0) & 0x0ff;
	}
	else if (inHex.length() == 4 and inHex[0] == '#')
	{
		uint32_t v = strtoul(inHex.c_str() + 1, nullptr, 16);
		red = (v >> 8) & 0x0f;
		red = (red << 4) | red;
		green = (v >> 4) & 0x0f;
		green = (green << 4) | green;
		blue = (v >> 0) & 0x0f;
		blue = (blue << 4) | blue;
	}
}

MColor MColor::Disable(const MColor &inBackColor) const
{
	MColor r;
	r.red = static_cast<uint8_t>((red + inBackColor.red) / 2);
	r.green = static_cast<uint8_t>((green + inBackColor.green) / 2);
	r.blue = static_cast<uint8_t>((blue + inBackColor.blue) / 2);
	return r;
}

MColor MColor::Disable(const MColor &inBackColor, float inScale) const
{
	float r = (red / 255.f), g = (green / 255.f), b = (blue / 255.f);
	float rb = (inBackColor.red / 255.f), gb = (inBackColor.green / 255.f), bb = (inBackColor.blue / 255.f);

	return MColor(
		(1 - inScale) * r + inScale * (r + rb) / 2,
		(1 - inScale) * g + inScale * (g + gb) / 2,
		(1 - inScale) * b + inScale * (b + bb) / 2);
}

MColor MColor::Distinct(const MColor &inBackColor) const
{
	const uint32_t
		kDistinctColorTresholdSquare_1 = 10000,
		kDistinctColorTresholdSquare_2 = 50000;

	// Does a simple distance based color comparison, returns an
	// inverse color if colors close enough
	uint32_t redDelta = (uint32_t)red - (uint32_t)inBackColor.red;
	uint32_t greenDelta = (uint32_t)green - (uint32_t)inBackColor.green;
	uint32_t blueDelta = (uint32_t)blue - (uint32_t)inBackColor.blue;

	auto distance = redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta;

	MColor result;

	if (distance > kDistinctColorTresholdSquare_2) // very good distance
		result = *this;
	else if (distance > kDistinctColorTresholdSquare_1) // poor distance
	{
		float fr = (red / 255.f), fg = (green / 255.f), fb = (blue / 255.f);
		float br = (inBackColor.red / 255.f), bg = (inBackColor.green / 255.f), bb = (inBackColor.blue / 255.f);

		float fh, fs, fv, bh, bs, bv;
		rgb2hsv(fr, fg, fb, fh, fs, fv);
		rgb2hsv(br, bg, bb, bh, bs, bv);

		if (fv > bv) // fore color is lighter than background, make it even lighter
			fv = (1 + fv) / 2;
		else
			fv = (0 + fv) / 2;

		hsv2rgb(fh, fs, fv, fr, fg, fb);

		result.red = static_cast<uint8_t>(fr * 255);
		result.green = static_cast<uint8_t>(fg * 255);
		result.blue = static_cast<uint8_t>(fb * 255);
	}
	else // really need to invert
	{
		result.red = static_cast<uint8_t>(255 - red);
		result.green = static_cast<uint8_t>(255 - green);
		result.blue = static_cast<uint8_t>(255 - blue);
	}

	return result;
}

MColor MColor::Bleach(float inBleachFactor) const
{
	float r = (red / 255.f), g = (green / 255.f), b = (blue / 255.f);

	float h = 0, s = 0, v = 0;
	rgb2hsv(r, g, b, h, s, v);

	s = (1 - inBleachFactor) * s;

	if (v < 0.5)
		v = inBleachFactor + (1 - inBleachFactor) * v;
	else
		v = (1 - inBleachFactor) * v;

	hsv2rgb(h, s, v, r, g, b);

	return MColor(r, g, b);
}

std::string MColor::str() const
{
	return hex();
}

ostream &operator<<(ostream &os, const MColor &inColor)
{
	ios_base::fmtflags flags = os.setf(ios_base::hex, ios_base::basefield);

	os << '#'
	   << setw(2) << setfill('0') << static_cast<uint32_t>(inColor.red)
	   << setw(2) << setfill('0') << static_cast<uint32_t>(inColor.green)
	   << setw(2) << setfill('0') << static_cast<uint32_t>(inColor.blue);

	os.setf(flags);
	return os;
}

// --------------------------------------------------------------------

void rgb2hsv(float r, float g, float b, float &h, float &s, float &v)
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

void hsv2rgb(float h, float s, float v, float &r, float &g, float &b)
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
			case 0:
				r = v;
				g = C;
				b = A;
				break;
			case 1:
				r = B;
				g = v;
				b = A;
				break;
			case 2:
				r = A;
				g = v;
				b = C;
				break;
			case 3:
				r = A;
				g = B;
				b = v;
				break;
			case 4:
				r = C;
				g = A;
				b = v;
				break;
			case 5:
				r = v;
				g = A;
				b = B;
				break;
		}
	}
} /* hsv2rgb */
