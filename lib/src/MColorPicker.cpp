//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <regex>

#include "MColorPicker.hpp"
#include "MCanvas.hpp"
#include "MDevice.hpp"
#include "MPreferences.hpp"
#include "MControls.hpp"
#include "MUtils.hpp"

using namespace std;

// --------------------------------------------------------------------

class MColorSquare : public MCanvas
{
  public:
					MColorSquare(const string& inID, MRect inBounds, MColorPicker& inPicker);

	// virtual void	Draw(MRect inUpdate);
	virtual void	Draw();
	
	void			SetColor(MColor inColor);
	void			SetMode(MPickerMode inMode);
	
	MEventIn<void(MColor)>		eChangedColor;
	MEventIn<void(MPickerMode)>	eChangedMode;

	virtual void	MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers);
	virtual void	MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void	MouseExit();
	virtual void	MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);

  private:
	bool			mMouseDown;
	MColorPicker&	mPicker;
};

MColorSquare::MColorSquare(const string& inID, MRect inBounds, MColorPicker& inPicker)
	: MCanvas(inID, inBounds, false, false)
	, eChangedColor(this, &MColorSquare::SetColor)
	, eChangedMode(this, &MColorSquare::SetMode)
	, mMouseDown(false)
	, mPicker(inPicker)
{
}

void MColorSquare::Draw()
{
	MDevice dev(this);
	
	MRect bounds;
	GetBounds(bounds);

	dev.EraseRect(bounds);
	
	MBitmap bitmap(bounds.width, bounds.height);

	char* data = reinterpret_cast<char*>(bitmap.Data());
	
	MPickerMode mode = mPicker.GetMode();
	
	float r = 0, g = 0, b = 0, h = 0, s = 0, v = 0, sfx = 0, sfy = 0;

	switch (mode)
	{
		case ePickRGB:	mPicker.GetRGB(r, g, b); sfx = r; sfy = 1.f - g; break;
		case ePickBGR:	mPicker.GetRGB(r, g, b); sfx = b; sfy = 1.f - g; break;
		case ePickBRG:	mPicker.GetRGB(r, g, b); sfx = b; sfy = 1.f - r; break;
		case ePickSVH:	mPicker.GetHSV(h, s, v); sfx = s; sfy = 1.f - v; break;
		case ePickHVS:	mPicker.GetHSV(h, s, v); sfx = h; sfy = 1.f - v; break;
		case ePickHSV:	mPicker.GetHSV(h, s, v); sfx = h; sfy = 1.f - s; break;
	}

	int32_t sx = static_cast<int32_t>(sfx * bounds.width);
	int32_t sy = static_cast<int32_t>(sfy * bounds.height);

	for (int32_t y = 0; y < bounds.height; ++y)
	{
		uint8_t* row = reinterpret_cast<uint8_t*>(data + y * bitmap.Stride());
		
		switch (mode)
		{
			case ePickRGB:
			case ePickBGR:	g = 1.f - (float(y) / bounds.height); break;
			case ePickBRG:	r = 1.f - (float(y) / bounds.height); break;
			case ePickSVH:
			case ePickHVS:	v = 1.f - (float(y) / bounds.height); break;
			case ePickHSV:	s = 1.f - (float(y) / bounds.height); break;
		}
		
		for (int32_t x = 0; x < bounds.width; ++x)
		{
			switch (mode)
			{
				case ePickRGB:	r = float(x) / bounds.width; break;
				case ePickBGR:
				case ePickBRG:	b = float(x) / bounds.width; break;
				case ePickSVH:	s = float(x) / bounds.width; hsv2rgb(h, s, v, r, g, b); break;
				case ePickHVS:
				case ePickHSV:	h = float(x) / bounds.width; hsv2rgb(h, s, v, r, g, b); break;
			}
			
			MColor c(r, g, b);
			if ((x == sx and abs(y - sy) < 3) or
				(y == sy and abs(x - sx) < 3))
			{
				if ((y - sy) & 1 or (x - sx) & 1)
					c = kBlack.Distinct(c);
				else
					c = kWhite.Distinct(c);
			}

			*row++ = c.blue; 
			*row++ = c.green; 
			*row++ = c.red; 
			*row++ = 255;
		}
	}

	dev.DrawBitmap(bitmap, 0, 0);
}

void MColorSquare::MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers)
{
	mMouseDown = true;
	MouseMove(inX, inY, inModifiers);
}

void MColorSquare::MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	if (mMouseDown)
	{
		MRect bounds;
		GetBounds(bounds);
		
		bounds.PinPoint(inX, inY);

		float r, g, b, h, s, v;
		mPicker.GetRGB(r, g, b);
		mPicker.GetHSV(h, s, v);

		float x = float(inX) / bounds.width;
		float y = 1.f - (float(inY) / bounds.height);

		switch (mPicker.GetMode())
		{
			case ePickSVH:	mPicker.SetHSV(h, x, y); break;
			case ePickHVS:	mPicker.SetHSV(x, s, y); break;
			case ePickHSV:	mPicker.SetHSV(x, y, v); break;
			case ePickBGR:	mPicker.SetRGB(r, y, x); break;
			case ePickBRG:	mPicker.SetRGB(y, g, x); break;
			case ePickRGB:	mPicker.SetRGB(x, y, b); break;
		}
	}
}

void MColorSquare::MouseExit()
{
	mMouseDown = false;
}

void MColorSquare::MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	mMouseDown = false;
}

void MColorSquare::SetMode(MPickerMode inMode)
{
	Invalidate();
}

void MColorSquare::SetColor(MColor inColor)
{
	Invalidate();
}

// --------------------------------------------------------------------

class MColorSlider : public MCanvas
{
  public:
					MColorSlider(const string& inID, MRect inBounds, MColorPicker& inPicker);

	// virtual void	Draw(MRect inUpdate);
	virtual void	Draw();
	
	void			SetColor(MColor inColor);
	void			SetMode(MPickerMode inMode);
	
	MEventIn<void(MColor)>		eChangedColor;
	MEventIn<void(MPickerMode)>	eChangedMode;

	virtual void	MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers);
	virtual void	MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void	MouseExit();
	virtual void	MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);

  private:
	bool			mMouseDown;
	MColorPicker&	mPicker;
};

MColorSlider::MColorSlider(const string& inID, MRect inBounds, MColorPicker& inPicker)
	: MCanvas(inID, inBounds, false, false)
	, eChangedColor(this, &MColorSlider::SetColor)
	, eChangedMode(this, &MColorSlider::SetMode)
	, mMouseDown(false)
	, mPicker(inPicker)
{
}

void MColorSlider::Draw()
{
	MDevice dev(this);
	
	MRect bounds;
	GetBounds(bounds);

	dev.EraseRect(bounds);
	
	MBitmap bitmap(bounds.width, bounds.height);

	uint8_t* data = reinterpret_cast<uint8_t*>(bitmap.Data());
	
	MPickerMode mode = mPicker.GetMode();

	float r = 0, g = 0, b = 0, h = 0, s = 0, v = 0;
	int32_t sy = 0;

	switch (mode)
	{
		case ePickRGB:	mPicker.GetRGB(r, g, b); sy = static_cast<int32_t>((1.f - b) * bounds.height); break;
		case ePickBGR:	mPicker.GetRGB(r, g, b); sy = static_cast<int32_t>((1.f - r) * bounds.height); break;
		case ePickBRG:	mPicker.GetRGB(r, g, b); sy = static_cast<int32_t>((1.f - g) * bounds.height); break;
		case ePickSVH:	mPicker.GetHSV(h, s, v); sy = static_cast<int32_t>(h * bounds.height); break;
		case ePickHVS:	mPicker.GetHSV(h, s, v); sy = static_cast<int32_t>((1.f - s) * bounds.height); break;
		case ePickHSV:	mPicker.GetHSV(h, s, v); sy = static_cast<int32_t>((1.f - v) * bounds.height); break;
	}

	for (int32_t y = 0; y < bounds.height; ++y)
	{
		uint8_t* row = reinterpret_cast<uint8_t*>(data + y * bitmap.Stride());
		
		switch (mode)
		{
			case ePickRGB:	b = 1.f - float(y) / bounds.height; break;
			case ePickBGR:	r = 1.f - float(y) / bounds.height; break;
			case ePickBRG:	g = 1.f - float(y) / bounds.height; break;
			case ePickSVH:	h = float(y) / bounds.height; 		hsv2rgb(h, s, v, r, g, b); break;
			case ePickHVS:	s = 1.f - float(y) / bounds.height; hsv2rgb(h, s, v, r, g, b); break;
			case ePickHSV:	v = 1.f - float(y) / bounds.height; hsv2rgb(h, s, v, r, g, b); break;
		}
		
		for (int32_t x = 0; x < bounds.width; ++x)
		{		
			MColor c(r, g, b);
		
			if (y == sy)
			{
				if (x & 1)
					c = kBlack.Distinct(c);
				else
					c = kWhite.Distinct(c);
			}

			*row++ = c.blue; 
			*row++ = c.green; 
			*row++ = c.red; 
			*row++ = 255;
		}
	}

	dev.DrawBitmap(bitmap, 0, 0);
}

void MColorSlider::MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers)
{
	mMouseDown = true;
	MouseMove(inX, inY, inModifiers);
}

void MColorSlider::MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	if (mMouseDown)
	{
		MRect bounds;
		GetBounds(bounds);

		bounds.PinPoint(inX, inY);
		
		float r, g, b, h, s, v;
		mPicker.GetRGB(r, g, b);
		mPicker.GetHSV(h, s, v);

		float y = 1.f - float(inY) / bounds.height;

		switch (mPicker.GetMode())
		{
			case ePickSVH:	mPicker.SetHSV(1.0f - y, s, v); break;
			case ePickHVS:	mPicker.SetHSV(h, y, v); break;
			case ePickHSV:	mPicker.SetHSV(h, s, y); break;
			case ePickBGR:	mPicker.SetRGB(y, g, b); break;
			case ePickBRG:	mPicker.SetRGB(r, y, b); break;
			case ePickRGB:	mPicker.SetRGB(r, g, y); break;
		}
	}
}

void MColorSlider::MouseExit()
{
	mMouseDown = false;
}

void MColorSlider::MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	mMouseDown = false;
}

void MColorSlider::SetMode(MPickerMode inMode)
{
	Invalidate();
}

void MColorSlider::SetColor(MColor inColor)
{
	Invalidate();
}

// --------------------------------------------------------------------

class MColorSample : public MCanvas
{
  public:
					MColorSample(const string& inID, MRect inBounds, MColorPicker& inPicker, MColor& inColor);

	// virtual void	Draw(MRect inUpdate);
	virtual void	Draw();

	virtual void	MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers);
	virtual void	MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);

	void			SetColor(MColor inColor);
	
	MEventIn<void(MColor)>		eChangedColor;

  private:
	MColorPicker&	mPicker;
	MColor			mColor;
	bool			mMouseDown;
};

MColorSample::MColorSample(const string& inID, MRect inBounds, MColorPicker& inPicker, MColor& inColor)
	: MCanvas(inID, inBounds, false, false)
	, eChangedColor(this, &MColorSample::SetColor)
	, mPicker(inPicker)
	, mColor(inColor)
{
}

void MColorSample::Draw()
{
	MDevice dev(this);

	dev.SetForeColor(mColor);
	
	MRect bounds;
	GetBounds(bounds);

	dev.FillRect(bounds);
}

void MColorSample::MouseDown(int32_t inX, int32_t inY,
	uint32_t inClickCount, uint32_t inModifiers)
{
	mMouseDown = true;
}

void MColorSample::MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	if (mMouseDown and mBounds.ContainsPoint(inX, inY))
		mPicker.SetColor(mColor);
	
	mMouseDown = false;
}

void MColorSample::SetColor(MColor inColor)
{
	mColor = inColor;
	Invalidate();
}

// --------------------------------------------------------------------

MColorPicker::MColorPicker(
	MWindow*		inWindow,
	MColor			inColor)
	: MDialog("color-picker")
	, mMode(ePickHSV)
	, mSettingText(false)
	, mOriginal(inColor)
{
	// init color
	mRed = inColor.red / 255.f;
	mGreen = inColor.green / 255.f;
	mBlue = inColor.blue / 255.f;
	rgb2hsv(mRed, mGreen, mBlue, mHue, mSaturation, mValue);

	// build dialog
	
	MView* placeholder = FindSubViewByID("square");
	MRect bounds;
	placeholder->GetBounds(bounds);
	
	// correct the size
	int32_t dx = 256 - bounds.width;
	int32_t dy = 256 - bounds.height;
	ResizeWindow(dx, dy);

	placeholder->GetBounds(bounds);
	MColorSquare* square = new MColorSquare("square-control", bounds, *this);
	placeholder->AddChild(square);

	placeholder = FindSubViewByID("slider");
	placeholder->GetBounds(bounds);
	MColorSlider* slider = new MColorSlider("slider-control", bounds, *this);
	placeholder->AddChild(slider);

	placeholder = FindSubViewByID("sample-before");
	placeholder->GetBounds(bounds);
	MColorSample* sample = new MColorSample("sample-control", bounds, *this, inColor);
	placeholder->AddChild(sample);

	placeholder = FindSubViewByID("sample-after");
	placeholder->GetBounds(bounds);
	sample = new MColorSample("sample-control", bounds, *this, inColor);
	placeholder->AddChild(sample);
	
	AddRoute(eChangedMode, square->eChangedMode);
	AddRoute(eChangedMode, slider->eChangedMode);

	AddRoute(eChangedColor, square->eChangedColor);
	AddRoute(eChangedColor, slider->eChangedColor);
	AddRoute(eChangedColor, sample->eChangedColor);
	
	UpdateColor();
	
	Show(inWindow);

	string mode = Preferences::GetString("color-picker-mode", "hue");
	MRadiobutton* button = dynamic_cast<MRadiobutton*>(FindSubViewByID(mode));
	if (button != nullptr)
	{
		button->SetChecked(true);
		RadiobuttonChanged(mode, true);
	}
	
	Select();
}

void MColorPicker::RadiobuttonChanged(const string& inID, bool inValue)
{
	if (inValue)
	{
		if (inID == "hue")				SetMode(ePickSVH);
		else if (inID == "saturation")	SetMode(ePickHVS);
		else if (inID == "value")		SetMode(ePickHSV);
		else if (inID == "red")			SetMode(ePickBGR);
		else if (inID == "green")		SetMode(ePickBRG);
		else if (inID == "blue")		SetMode(ePickRGB);
	}
}

void MColorPicker::TextChanged(const string& inID, const string& inText)
{
	if (mSettingText)
		return;
	
	if (inID == "hex")
	{
		const std::regex re("([[:xdigit:]]{2})([[:xdigit:]]{2})([[:xdigit:]]{2})");
		std::smatch m;
		if (std::regex_match(inText, m, re))
		{
			MColor color(
				static_cast<uint8_t>(strtoul(m[1].str().c_str(), nullptr, 16)),
				static_cast<uint8_t>(strtoul(m[2].str().c_str(), nullptr, 16)),
				static_cast<uint8_t>(strtoul(m[0].str().c_str(), nullptr, 16)));
			SetColor(color);
		}
	}
	else
	{
		uint32_t v;

		try
		{
			v = std::stoi(inText);
			
			if (inID == "red-text")				SetRGB(v / 255.f, mGreen, mBlue);
			else if (inID == "green-text")		SetRGB(mRed, v / 255.f, mBlue);
			else if (inID == "blue-text")		SetRGB(mRed, mGreen, v / 255.f);
			else if (inID == "hue-text")		SetHSV(v / 360.f, mSaturation, mValue);
			else if (inID == "saturation-text")	SetHSV(mHue, v / 100.f, mValue);
			else if (inID == "value-text")		SetHSV(mHue, mSaturation, v / 100.f);
		}
		catch (std::invalid_argument&)
		{
			
		}
	}
}

void MColorPicker::SetMode(MPickerMode inMode)
{
	mMode = inMode;
	eChangedMode(inMode);

	switch (inMode)
	{
		case ePickSVH: Preferences::SetString("color-picker-mode", "hue"); break;
		case ePickHVS: Preferences::SetString("color-picker-mode", "saturation"); break;
		case ePickHSV: Preferences::SetString("color-picker-mode", "value"); break;
		case ePickBGR: Preferences::SetString("color-picker-mode", "red"); break;
		case ePickBRG: Preferences::SetString("color-picker-mode", "green"); break;
		case ePickRGB: Preferences::SetString("color-picker-mode", "blue"); break;
	}
}

MPickerMode MColorPicker::GetMode() const
{
	return mMode;
}

void MColorPicker::UpdateColor()
{
	MValueChanger<bool> save(mSettingText, true);
	
	uint32_t red = static_cast<uint32_t>(mRed * 255);
	uint32_t green = static_cast<uint32_t>(mGreen * 255);
	uint32_t blue = static_cast<uint32_t>(mBlue * 255);

	SetText("red-text", std::to_string(red));
	SetText("green-text", std::to_string(green));
	SetText("blue-text", std::to_string(blue));

	uint32_t hue = static_cast<uint32_t>(mHue * 360);
	uint32_t saturation = static_cast<uint32_t>(mSaturation * 100);
	uint32_t value = static_cast<uint32_t>(mValue * 100);

	SetText("hue-text", std::to_string(hue));
	SetText("saturation-text", std::to_string(saturation));
	SetText("value-text", std::to_string(value));

	char hex[7] = {};
	hex[0] = kHexChars[red >> 4];
	hex[1] = kHexChars[red & 0x0f];
	hex[2] = kHexChars[green >> 4];
	hex[3] = kHexChars[green & 0x0f];
	hex[4] = kHexChars[blue >> 4];
	hex[5] = kHexChars[blue & 0x0f];
	
	SetText("hex", hex);

	MColor color(mRed, mGreen, mBlue);
	eChangedColor(color);
	
	UpdateNow();
}

void MColorPicker::SetColor(MColor inColor)
{
	mRed = inColor.red / 255.f;
	mGreen = inColor.green / 255.f;
	mBlue = inColor.blue / 255.f;
	rgb2hsv(mRed, mGreen, mBlue, mHue, mSaturation, mValue);

	UpdateColor();
}

void MColorPicker::SetRGB(float inRed, float inGreen, float inBlue)
{
	if (inRed > 1.f)		inRed = 1.f;
	else if (inRed < 0.f)	inRed = 0;
	if (inGreen > 1.f)		inGreen = 1.f;
	else if (inGreen < 0.f)	inGreen = 0;
	if (inBlue > 1.f)		inBlue = 1.f;
	else if (inBlue < 0.f)	inBlue = 0;
	
	if (mRed != inRed or mGreen != inGreen or mBlue != inBlue)
	{
		mRed = inRed;
		mGreen = inGreen;
		mBlue = inBlue;
		rgb2hsv(mRed, mGreen, mBlue, mHue, mSaturation, mValue);
	
		UpdateColor();
	}
}

void MColorPicker::SetHSV(float inHue, float inSaturation, float inValue)
{
	if (inHue > 1.f)				inHue = 1.f;
	else if (inHue < 0.f)			inHue = 0;
	if (inSaturation > 1.f)			inSaturation = 1.f;
	else if (inSaturation < 0.f)	inSaturation = 0;
	if (inValue > 1.f)				inValue = 1.f;
	else if (inValue < 0.f)			inValue = 0;
	
	if (mHue != inHue or mSaturation != inSaturation or mValue != inValue)
	{
		mHue = inHue;
		mSaturation = inSaturation;
		mValue = inValue;
		hsv2rgb(mHue, mSaturation, mValue, mRed, mGreen, mBlue);
	
		UpdateColor();
	}
}

void MColorPicker::GetColor(MColor& outColor) const
{
	outColor.red = static_cast<uint8_t>(mRed * 255);
	outColor.green = static_cast<uint8_t>(mGreen * 255);
	outColor.blue = static_cast<uint8_t>(mBlue * 255);
}

void MColorPicker::GetRGB(float& outRed, float& outGreen, float& outBlue) const
{
	outRed = mRed;
	outGreen = mGreen;
	outBlue = mBlue;
}

void MColorPicker::GetHSV(float& outHue, float& outSaturation, float& outValue) const
{
	outHue = mHue;
	outSaturation = mSaturation;
	outValue = mValue;
}

bool MColorPicker::OKClicked()
{
	eSelectedColor(MColor(mRed, mGreen, mBlue));
	return true;
}

bool MColorPicker::CancelClicked()
{
	eChangedColor(mOriginal);
	return true;
}
