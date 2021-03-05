//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id: MDialog.h 133 2007-05-01 08:34:48Z maarten $
	Copyright Maarten L. Hekkelman
	Created Sunday August 15 2004 13:51:09
*/

#pragma once

#include <vector>

#include "MWindow.hpp"

class MDialog : public MWindow
{
  public:
					~MDialog();

	virtual bool	OKClicked();

	virtual bool	CancelClicked();

	using MWindow::Show;
	void			Show(MWindow* inParent);
	bool			ShowModal(MWindow* inParent);

	void			SavePosition(const char* inName);

	void			RestorePosition(const char* inName);

	MWindow*		GetParentWindow() const				{ return mParentWindow; }
	
	using MHandler::SetFocus;
	void			SetFocus(const std::string& inID);

	std::string		GetText(const std::string& inID) const;
	void			SetText(const std::string& inID, const std::string& inText);

	int32			GetValue(const std::string& inID) const;
	void			SetValue(const std::string& inID, int32 inValue);

	// checkboxes
	bool			IsChecked(const std::string& inID) const;
	void			SetChecked(const std::string& inID, bool inChecked);

	// popup
	void			SetChoices(const std::string& inID,
						std::vector<std::string>& inChoices);
	
	// chevron
	bool			IsOpen(const std::string& inID) const;
	void			SetOpen(const std::string& inID, bool inOpen);
	
	// color swatch
	MColor			GetColor(const std::string& inID) const;
	void			SetColor(const std::string& inID, MColor inColor);
	
	void			SetEnabled(const std::string& inID, bool inEnabled);
	void			SetVisible(const std::string& inID, bool inVisible);

	void			SetPasswordChar(const std::string& inID, uint32 inUnicode = 0x2022);

	virtual void	ButtonClicked(const std::string& inID);
	virtual void	CheckboxChanged(const std::string& inID, bool inValue);
	virtual void	RadiobuttonChanged(const std::string& inID, bool inValue);
	virtual void	TextChanged(const std::string& inID, const std::string& inText);
	virtual void	ValueChanged(const std::string& inID, int32 inValue);
	virtual void	ColorChanged(const std::string& inID, MColor inColor);

	MEventIn<void(const std::string&)>						eButtonClicked;
	MEventIn<void(const std::string&,bool)>					eCheckboxClicked;
	MEventIn<void(const std::string&,bool)>					eRadiobuttonClicked;
	MEventIn<void(const std::string&,const std::string&)>	eTextChanged;
	MEventIn<void(const std::string&,int32)>				eValueChanged;
	MEventIn<void(const std::string&,MColor)>				eColorChanged;

  protected:

					MDialog(const std::string& inDialogResource);

	virtual void	ChildResized();
	virtual void	RecalculateLayout();

  private:

	MWindow*		mParentWindow;
	MDialog*		mNext;						// for the close all
	static MDialog*	sFirst;
};
