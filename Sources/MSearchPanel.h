// Copyright Maarten L. Hekkelman 2013
// All rights reserved

#pragma once

#include "MControls.h"
#include "MP2PEvents.h"

const uint32
	kSearchPanelHeight = 28,
	cmd_HideSearchPanel	= 'hidS';

enum MSearchDirection
{
	searchUp,
	searchDown
};

class MSearchPanel : public MBoxControl
{
  public:
					MSearchPanel(const std::string&	inID,
						MRect inBounds);

	virtual			~MSearchPanel();

	virtual bool	HandleKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);

	std::string		GetSearchString() const;
	bool			GetIgnoreCase() const;

	uint32			GetHeight() const;

	MEventOut<void(MSearchDirection)>	eSearch;
	MEdittext*		GetTextBox() const				{ return mTextBox; }

  private:

	MEventIn<void()>	eClose;
	void				Close();

	MEventIn<void(const std::string&)>	eFindBtn;
	void								FindBtn(const std::string&);

	MEdittext*		mTextBox;
	MCheckbox*		mCaseSensitive;
};
