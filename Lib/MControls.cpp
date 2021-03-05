//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.h"

#include <cassert>

#include "MControlsImpl.h"
#include "MWindow.h"

using namespace std;

// --------------------------------------------------------------------

MSimpleControl::MSimpleControl(const string& inID, MRect inBounds)
	: MControl<MSimpleControlImpl>(inID, inBounds, MSimpleControlImpl::Create(this))
{
}

// --------------------------------------------------------------------

MButton::MButton(const string& inID, MRect inBounds, const string& inLabel,
	MButtonFlags inFlags)
	: MControl<MButtonImpl>(inID, inBounds, MButtonImpl::Create(this, inLabel, inFlags))
{
}

void MButton::SimulateClick()
{
	mImpl->SimulateClick();
}

void MButton::MakeDefault(bool inDefault)
{
	mImpl->MakeDefault(inDefault);
}

void MButton::SetText(const string& inText)
{
	mImpl->SetText(inText);
}

// --------------------------------------------------------------------

MExpander::MExpander(const string& inID, MRect inBounds, const string& inLabel)
	: MControl<MExpanderImpl>(inID, inBounds, MExpanderImpl::Create(this, inLabel))
{
}

void MExpander::SetOpen(bool inOpen)
{
	mImpl->SetOpen(inOpen);
}

bool MExpander::IsOpen() const
{
	return mImpl->IsOpen();
}

// --------------------------------------------------------------------

MScrollbar::MScrollbar(const string& inID, MRect inBounds)
	: MControl<MScrollbarImpl>(inID, inBounds, MScrollbarImpl::Create(this))
{
}

int32 MScrollbar::GetValue() const
{
	return mImpl->GetValue();
}

void MScrollbar::SetValue(int32 inValue)
{
	mImpl->SetValue(inValue);
}

int32 MScrollbar::GetTrackValue() const
{
	return mImpl->GetTrackValue();
}

void MScrollbar::SetAdjustmentValues(int32 inMinValue, int32 inMaxValue,
	int32 inScrollUnit, int32 inPageSize, int32 inValue)
{
	mImpl->SetAdjustmentValues(inMinValue, inMaxValue,
		inScrollUnit, inPageSize, inValue);
}

//void MScrollbar::SetMinValue(int32 inMinValue)
//{
//	mImpl->SetMinValue(inMinValue);
//}

int32 MScrollbar::GetMinValue() const
{
	return mImpl->GetMinValue();
}
	
//void MScrollbar::SetMaxValue(int32 inMaxValue)
//{
//	mImpl->SetMaxValue(inMaxValue);
//}

int32 MScrollbar::GetMaxValue() const
{
	return mImpl->GetMaxValue();
}

//void MScrollbar::SetViewSize(int32 inViewSize)
//{
//	mImpl->SetViewSize(inViewSize);
//}

// --------------------------------------------------------------------

MStatusbar::MStatusbar(const string& inID, MRect inBounds, uint32 inPartCount, MStatusBarElement inParts[])
	: MControl<MStatusbarImpl>(inID, inBounds, MStatusbarImpl::Create(this, inPartCount, inParts))
{
	SetBindings(true, false, true, true);
}

void MStatusbar::SetStatusText(uint32 inPartNr, const string& inText, bool inBorder)
{
	mImpl->SetStatusText(inPartNr, inText, inBorder);
}

// --------------------------------------------------------------------

MCombobox::MCombobox(const string& inID, MRect inBounds)
	: MControl<MComboboxImpl>(inID, inBounds, MComboboxImpl::Create(this))
{
}

void MCombobox::SetText(const string& inText)
{
	mImpl->SetText(inText);
}

string MCombobox::GetText() const
{
	return mImpl->GetText();
}

void MCombobox::SetChoices(const vector<string>& inChoices)
{
	mImpl->SetChoices(inChoices);
}

// --------------------------------------------------------------------

MPopup::MPopup(const string& inID, MRect inBounds)
	: MControl<MPopupImpl>(inID, inBounds, MPopupImpl::Create(this))
{
}

void MPopup::SetValue(int32 inValue)
{
	mImpl->SetValue(inValue);
}

int32 MPopup::GetValue() const
{
	return mImpl->GetValue();
}

void MPopup::SetText(const string& inText)
{
	mImpl->SetText(inText);
}

string MPopup::GetText() const
{
	return mImpl->GetText();
}

void MPopup::SetChoices(const vector<string>& inChoices)
{
	mImpl->SetChoices(inChoices);
}

// --------------------------------------------------------------------

MEdittext::MEdittext(const string& inID, MRect inBounds, uint32 inFlags)
	: MControl<MEdittextImpl>(inID, inBounds, MEdittextImpl::Create(this, inFlags))
{
}

void MEdittext::SetText(const string& inText)
{
	mImpl->SetText(inText);
}

string MEdittext::GetText() const
{
	return mImpl->GetText();
}

uint32 MEdittext::GetFlags() const
{
	return mImpl->GetFlags();
}

void MEdittext::SetPasswordChar(uint32 inUnicode)
{
	mImpl->SetPasswordChar(inUnicode);
}

// --------------------------------------------------------------------

MCaption::MCaption(const string& inID, MRect inBounds, const string& inText)
	: MControl<MCaptionImpl>(inID, inBounds, MCaptionImpl::Create(this, inText))
{
}

void MCaption::SetText(const string& inText)
{
	mImpl->SetText(inText);
}

// --------------------------------------------------------------------

MSeparator::MSeparator(const string& inID, MRect inBounds)
	: MControl<MSeparatorImpl>(inID, inBounds, MSeparatorImpl::Create(this))
{
}

// --------------------------------------------------------------------

MCheckbox::MCheckbox(const string& inID, MRect inBounds, const string& inTitle)
	: MControl<MCheckboxImpl>(inID, inBounds, MCheckboxImpl::Create(this, inTitle))
{
}

bool MCheckbox::IsChecked() const
{
	return mImpl->IsChecked();
}

void MCheckbox::SetChecked(bool inChecked)
{
	mImpl->SetChecked(inChecked);
}

// --------------------------------------------------------------------

MRadiobutton::MRadiobutton(const string& inID, MRect inBounds, const string& inTitle)
	: MControl<MRadiobuttonImpl>(inID, inBounds, MRadiobuttonImpl::Create(this, inTitle))
{
}

bool MRadiobutton::IsChecked() const
{
	return mImpl->IsChecked();
}

void MRadiobutton::SetChecked(bool inChecked)
{
	mImpl->SetChecked(inChecked);
}

void MRadiobutton::SetGroup(const std::list<MRadiobutton*>& inButtons)
{
	mImpl->SetGroup(inButtons);
}

// --------------------------------------------------------------------

MListHeader::MListHeader(const std::string& inID, MRect inBounds)
	: MControl<MListHeaderImpl>(inID, inBounds, MListHeaderImpl::Create(this))
{
}

void MListHeader::AppendColumn(const string& inLabel, int inWidth)
{
	mImpl->AppendColumn(inLabel, inWidth);
}

// --------------------------------------------------------------------

MNotebook::MNotebook(const string& inID, MRect inBounds)
	: MControl<MNotebookImpl>(inID, inBounds, MNotebookImpl::Create(this))
{
}

void MNotebook::AddPage(const string& inLabel, MView* inPage)
{
	mImpl->AddPage(inLabel, inPage);
//	
//	MRect frame;
//	inPage->GetFrame(frame);
//	
//	MRect bounds = mBounds;
//	bounds |= frame;
//	if (bounds != mBounds)
//	{
//		ResizeFrame(bounds.width - mBounds.width,
//			bounds.height - mBounds.height);
//	}
}

void MNotebook::SelectPage(uint32 inPage)
{
	mImpl->SelectPage(inPage);
}

uint32 MNotebook::GetSelectedPage() const
{
	return mImpl->GetSelectedPage();
}

// --------------------------------------------------------------------

MColorSwatch::MColorSwatch(const string& inID, MRect inBounds, MColor inColor)
	: MControl<MColorSwatchImpl>(inID, inBounds, MColorSwatchImpl::Create(this, inColor))
{
}

MColor MColorSwatch::GetColor() const
{
	return mImpl->GetColor();
}

void MColorSwatch::SetColor(MColor inColor)
{
	mImpl->SetColor(inColor);
}

// --------------------------------------------------------------------

MListBox::MListBox(const string& inID, MRect inBounds)
	: MControl<MListBoxImpl>(inID, inBounds, MListBoxImpl::Create(this))
{
}

void MListBox::AddItem(const string& inLabel)
{
	mImpl->AddItem(inLabel);
}

int32 MListBox::GetValue() const
{
	return mImpl->GetValue();
}

void MListBox::SetValue(int32 inValue)
{
	mImpl->SetValue(inValue);
}

// --------------------------------------------------------------------

MListView::MListView(const string& inID, MRect inBounds)
	: MControl<MListViewImpl>(inID, inBounds, MListViewImpl::Create(this))
{
}

void MListView::AddItem(const string& inLabel)
{
	mImpl->AddItem(inLabel);
}

#ifndef _MSC_VER
// --------------------------------------------------------------------
// Gtk specific controls

MBoxControl::MBoxControl(const std::string& inID, MRect inBounds, bool inHorizontal,
		bool inHomogeneous, bool inExpand, bool inFill, uint32 inSpacing, uint32 inPadding)
	: MControl(inID, inBounds, MBoxControlImpl::Create(this,
		inHorizontal, inHomogeneous, inExpand, inFill, inSpacing, inPadding))
{
}

#endif
