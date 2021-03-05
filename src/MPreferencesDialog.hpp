// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MDialog.hpp"

class MPreferencesDialog : public MDialog
{
  public:

	static MPreferencesDialog&		Instance();

	static MEventOut<void()>		ePreferencesChanged;
	static MEventOut<void(MColor)>	eColorPreview;

  private:
					MPreferencesDialog();
	virtual			~MPreferencesDialog();

	virtual bool	OKClicked();
	
	virtual bool	AllowClose(bool inLogOff);

	virtual void	ButtonClicked(const std::string& inID);
	virtual void	CheckboxChanged(const std::string& inID, bool inValue);
	virtual void	TextChanged(const std::string& inID, const std::string& inText);
	virtual void	ValueChanged(const std::string& inID, int32 inValue);
	virtual void	ColorChanged(const std::string& inID, MColor inValue);

	MEventIn<void(const std::string&,MColor)>		ePreviewColor;
	void			ColorPreview(const std::string& inID, MColor inValue);

	virtual void	Apply();

	static MPreferencesDialog*		sInstance;
};
