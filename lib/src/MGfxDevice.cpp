/* 
   Created by: Maarten L. Hekkelman
   Date: woensdag 09 januari, 2019
*/

#include "MLib.hpp"

#include "MColor.hpp"

#include "MGfxDevice.hpp"

using namespace std;

// --------------------------------------------------------------------

struct MThemeColorRGB
{
	MColor	normal, disabled, active;
} kThemeColors[kThemeColorCount] = {
	{ "#fff", "#fff", "#3f67a5" },	//	kThemeColorMenuBackground,
	{ "#888", "#888", "#888" },		//	kThemeColorMenuFrame,
	{ "#000", "#777", "#fff" },		//	kThemeColorMenuText,

	{ "#ddd", "#ddd", "#3f67a5" },	//	kThemeColorMenubarBackground,
	{ "#M000", "#777", "#fff" },	//	kThemeColorMenubarText,

	{ "#eee", "#fff", "#3f67a5" },	//	kThemeColorButtonBackground,
	{ "#bbb", "#888", "#888" },		//	kThemeColorButtonFrame,
	{ "#000", "#777", "#fff" },		//	kThemeColorButtonText,

//	{ "#2b71df", "#666", "#fa4" },	//	kThemeColorButtonBackground,
//	{ "#888", "#888", "#888" },		//	kThemeColorButtonFrame,
//	{ "#fff", "#777", "#fff" },		//	kThemeColorButtonText,
	
};

// --------------------------------------------------------------------



// --------------------------------------------------------------------

void MGfxDevice::SetThemeColor(MThemeColor inThemeColor, bool inEnabled, bool inActive)
{
	auto& themeColor = kThemeColors[inThemeColor];
	if (inEnabled == false)
		SetColorRGB(themeColor.disabled);
	else if (inActive)
		SetColorRGB(themeColor.active);
	else
		SetColorRGB(themeColor.normal);
}

void MGfxDevice::StrokeLine(float x1, float y1, float x2, float y2)
{
	MoveTo(x1, y1);
	LineTo(x2, y2);
	Stroke();
}

void MGfxDevice::ShowTextInRect(MRect bounds, const char* inText)
{
	auto fontExtends = GetFontExtents();
	auto textExtends = GetTextExtents(inText);
	
	float x = bounds.x + (bounds.width - textExtends.width) / 2;
	float y = bounds.y + (bounds.height - fontExtends.height) / 2 + fontExtends.ascent;

	MoveTo(x, y);
	ShowText(inText);
}
