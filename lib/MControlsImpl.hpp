//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MControls.hpp"
#include "MControls.inl"

class MWinProcMixin;

struct MControlImplBase
{
	virtual ~MControlImplBase() {}

#if defined(_MSC_VER)
	virtual MWinProcMixin* GetWinProcMixin() = 0;
#endif
};

template<class CONTROL>
class MControlImpl : public MControlImplBase
{
public:
					MControlImpl(CONTROL* inControl)
						: mControl(inControl)					{}
	virtual			~MControlImpl()								{}

	virtual bool	IsFocus() const								{ return false; }
	virtual void	SetFocus()									{}

	virtual void	AddedToWindow()								{}
	virtual void	FrameMoved()								{}
	virtual void	FrameResized()								{}
	virtual void	MarginsChanged()							{}
	// virtual void	Draw(MRect inBounds)						{}
	virtual void	Draw(cairo_t* inCairo)						{}
	virtual void	Click(int32 inX, int32 inY)					{}
	virtual void	ActivateSelf()								{}
	virtual void	DeactivateSelf()							{}
	virtual void	EnableSelf()								{}
	virtual void	DisableSelf()								{}
	virtual void	ShowSelf()									{}
	virtual void	HideSelf()									{}

protected:
	CONTROL*		mControl;
};

class MSimpleControlImpl : public MControlImpl<MSimpleControl>
{
  public:
					MSimpleControlImpl(MSimpleControl* inControl)
						: MControlImpl(inControl) {}

	static MSimpleControlImpl*
					Create(MSimpleControl* inControl);
};

class MScrollbarImpl : public MControlImpl<MScrollbar>
{
public:
					MScrollbarImpl(MScrollbar* inControl)
						: MControlImpl<MScrollbar>(inControl)				{}

	virtual int32	GetValue() const = 0;
	virtual void	SetValue(int32 inValue) = 0;

	virtual int32	GetTrackValue() const = 0;

	virtual void	SetAdjustmentValues(int32 inMinValue, int32 inMaxValue,
						int32 inScrollUnit, int32 inPageSize, int32 inValue) = 0;

	virtual int32	GetMinValue() const = 0;
//	virtual void	SetMinValue(int32 inValue) = 0;
	virtual int32	GetMaxValue() const = 0;
//	virtual void	SetMaxValue(int32 inValue) = 0;
//
//	virtual void	SetViewSize(int32 inViewSize) = 0;

	static MScrollbarImpl*
					Create(MScrollbar* inControl);
};

class MButtonImpl : public MControlImpl<MButton>
{
public:
					MButtonImpl(MButton* inButton)
						: MControlImpl<MButton>(inButton)				{}

	virtual void	SimulateClick() = 0;
	virtual void	MakeDefault(bool inDefault) = 0;

	virtual void	SetText(const std::string& inText) = 0;

	virtual void	GetIdealSize(int32& outWidth, int32& outHeight) = 0;

	static MButtonImpl*
					Create(MButton* inButton, const std::string& inLabel,
						MButtonFlags inFlags);
};

//class MImageButtonImpl : public MControlImpl<MImageButton>
//{
//public:
//					MImageButtonImpl(MImageButton* inButton)
//						: MControlImpl<MImageButton>(inButton)				{}
//
//	static MImageButtonImpl*
//					Create(MImageButton* inButton, const std::string& inImageResource);
//};

class MExpanderImpl : public MControlImpl<MExpander>
{
public:
					MExpanderImpl(MExpander* inExpander)
						: MControlImpl<MExpander>(inExpander)				{}

	virtual void	SetOpen(bool inOpen) = 0;
	virtual bool	IsOpen() const = 0;

	static MExpanderImpl*
					Create(MExpander* inExpander, const std::string& inLabel);
};

class MStatusbarImpl : public MControlImpl<MStatusbar>
{
public:
					MStatusbarImpl(MStatusbar* inStatusbar)
						: MControlImpl<MStatusbar>(inStatusbar)				{}

	virtual void	SetStatusText(uint32 inPartNr, const std::string& inText, bool inBorder) = 0;

	static MStatusbarImpl*
					Create(MStatusbar* inStatusbar, uint32 inPartCount, MStatusBarElement inParts[]);
};

class MComboboxImpl : public MControlImpl<MCombobox>
{
public:
					MComboboxImpl(MCombobox* inCombobox)
						: MControlImpl<MCombobox>(inCombobox) {}
	
	virtual void	SetText(const std::string& inText) = 0;
	virtual std::string
					GetText() const = 0;
	
	virtual void	SetChoices(const std::vector<std::string>& inChoices) = 0;

	static MComboboxImpl*
					Create(MCombobox* inCombobox);
};

class MPopupImpl : public MControlImpl<MPopup>
{
public:
					MPopupImpl(MPopup* inPopup)
						: MControlImpl<MPopup>(inPopup) {}
	
	virtual void	SetValue(int32 inValue) = 0;
	virtual int32	GetValue() const = 0;
	
	virtual void	SetText(const std::string& inText) = 0;
	virtual std::string
					GetText() const = 0;

	virtual void	SetChoices(const std::vector<std::string>& inChoices) = 0;

	static MPopupImpl*
					Create(MPopup* inPopup);
};

class MEdittextImpl : public MControlImpl<MEdittext>
{
public:
					MEdittextImpl(MEdittext* inEdittext)
						: MControlImpl<MEdittext>(inEdittext) {}
	
	virtual void	SetText(const std::string& inText) = 0;
	virtual std::string
					GetText() const = 0;

	virtual uint32	GetFlags() const = 0;

	virtual void	SetPasswordChar(uint32 inUnicode) = 0;

	static MEdittextImpl*
					Create(MEdittext* inEdittext, uint32 inFlags);
};

class MCaptionImpl : public MControlImpl<MCaption>
{
public:
					MCaptionImpl(MCaption* inCaption)
						: MControlImpl<MCaption>(inCaption)				{}

	virtual void	SetText(const std::string& inText) = 0;

	static MCaptionImpl*
					Create(MCaption* inCaption, const std::string& inText);
};

class MSeparatorImpl : public MControlImpl<MSeparator>
{
public:
					MSeparatorImpl(MSeparator* inSeparator)
						: MControlImpl<MSeparator>(inSeparator)				{}

	static MSeparatorImpl*
					Create(MSeparator* inSeparator);
};

class MCheckboxImpl : public MControlImpl<MCheckbox>
{
public:
					MCheckboxImpl(MCheckbox* inCheckbox)
						: MControlImpl<MCheckbox>(inCheckbox)				{}

	virtual bool	IsChecked() const = 0;
	virtual void	SetChecked(bool inChecked) = 0;

	static MCheckboxImpl*
					Create(MCheckbox* inCheckbox, const std::string& inTitle);
};

class MRadiobuttonImpl : public MControlImpl<MRadiobutton>
{
public:
					MRadiobuttonImpl(MRadiobutton* inRadiobutton)
						: MControlImpl<MRadiobutton>(inRadiobutton)				{}

	virtual bool	IsChecked() const = 0;
	virtual void	SetChecked(bool inChecked) = 0;

	virtual void	SetGroup(const std::list<MRadiobutton*>& inButtons) = 0;

	static MRadiobuttonImpl*
					Create(MRadiobutton* inRadiobutton, const std::string& inTitle);
};

class MListHeaderImpl : public MControlImpl<MListHeader>
{
  public:
					MListHeaderImpl(MListHeader* inListHeader)
						: MControlImpl<MListHeader>(inListHeader) {}
	
	virtual void	AppendColumn(const std::string& inLabel, int inWidth) = 0;

	static MListHeaderImpl*
					Create(MListHeader* inListHeader);
};

class MNotebookImpl : public MControlImpl<MNotebook>
{
public:
					MNotebookImpl(MNotebook* inNotebook)
						: MControlImpl<MNotebook>(inNotebook)				{}

	virtual void	AddPage(const std::string& inLabel, MView* inPage) = 0;

	virtual void	SelectPage(uint32 inPage) = 0;
	virtual uint32	GetSelectedPage() const = 0;

	static MNotebookImpl*
					Create(MNotebook* inNotebook);
};

class MColorSwatchImpl : public MControlImpl<MColorSwatch>
{
public:
					MColorSwatchImpl(MColorSwatch* inColorSwatch)
						: MControlImpl<MColorSwatch>(inColorSwatch)				{}

	virtual MColor	GetColor() const = 0;
	virtual void	SetColor(MColor inColor) = 0;

	static MColorSwatchImpl*
					Create(MColorSwatch* inColorSwatch, MColor inColor);
};

class MListBoxImpl : public MControlImpl<MListBox>
{
public:
					MListBoxImpl(MListBox* inListBox)
						: MControlImpl<MListBox>(inListBox)				{}

	virtual void	AddItem(const std::string& inText) = 0;

	virtual int32	GetValue() const = 0;
	virtual void	SetValue(int32 inValue) = 0;

	static MListBoxImpl*
					Create(MListBox* inListBox);
};

class MListViewImpl : public MControlImpl<MListView>
{
public:
					MListViewImpl(MListView* inListView)
						: MControlImpl<MListView>(inListView)				{}

	virtual void	AddItem(const std::string& inText) = 0;

	static MListViewImpl*
					Create(MListView* inListView);
};

#ifndef _MSC_VER
// --------------------------------------------------------------------
// Gtk specific controls

class MBoxControlImpl : public MControlImpl<MBoxControl>
{
  public:
	MBoxControlImpl(MBoxControl* inControl)
		: MControlImpl(inControl)							{}

	static MBoxControlImpl* Create(MBoxControl* inControl, bool inHorizontal,
		bool inHomogeneous, bool inExpand, bool inFill, uint32 inSpacing, uint32 inPadding);
};

#endif
