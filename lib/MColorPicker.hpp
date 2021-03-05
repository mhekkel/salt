//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MDialog.hpp"

enum MPickerMode
{
	ePickSVH,
	ePickHVS,
	ePickHSV,
	ePickBGR,
	ePickBRG,
	ePickRGB
};

class MColorPicker : public MDialog
{
  public:
					MColorPicker(MWindow* inWindow, MColor inColor = kBlack);

	virtual bool	OKClicked();
	virtual bool	CancelClicked();

	virtual void	RadiobuttonChanged(const std::string& inID, bool inValue);
	virtual void	TextChanged(const std::string& inID, const std::string& inText);

	MEventOut<void(MPickerMode)>	eChangedMode;
	MEventOut<void(MColor)>			eChangedColor;
	MEventOut<void(MColor)>			eSelectedColor;

	void			SetColor(MColor inColor);
	void			SetRGB(float inRed, float inGreen, float inBlue);
	void			SetHSV(float inHue, float inSaturation, float inValue);

	void			GetColor(MColor& outColor) const;
	void			GetRGB(float& outRed, float& outGreen, float& outBlue) const;
	void			GetHSV(float& outHue, float& outSaturation, float& outValue) const;
	
	void			SetMode(MPickerMode inMode);
	MPickerMode		GetMode() const;

  private:

	void			UpdateColor();

	MPickerMode		mMode;
	bool			mSettingText;
	float			mRed, mGreen, mBlue, mHue, mSaturation, mValue;
	MColor			mOriginal;
};
