//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MControlsImpl.hpp"
#include "MXcbCairoSurface.hpp"

class MXcbBasicControl : public MXcbCairoSurface
{
  public:
	MXcbBasicControl(MView* inControl, const std::string& inLabel);

	bool IsActive() const;
	bool IsEnabled() const;

	virtual std::string GetText() const;
	virtual void SetText(const std::string& inText);

	virtual void ActivateSelf();
	virtual void DeactivateSelf();
	virtual void EnableSelf();
	virtual void DisableSelf();
	virtual void ShowSelf();
	virtual void HideSelf();
//	virtual void FrameMoved();
//	virtual void FrameResized();

  protected:

	virtual void Clicked() {}

	virtual void KeyPressEvent(xcb_key_press_event_t* inEvent);
	virtual void ButtonPressEvent(xcb_button_press_event_t* inEvent);
	virtual void ButtonReleaseEvent(xcb_key_release_event_t* inEvent);

	std::string mLabel;

	enum { Button1, Button2, Button3, Button4, Button5 };
	bool mButtonPressed[5];
};

template<class CONTROL>
class MX11ControlImpl : public CONTROL::MImpl, public MXcbBasicControl
{
  public:
					MX11ControlImpl(CONTROL* inControl, const std::string& inLabel)
						: CONTROL::MImpl(inControl), MXcbBasicControl(inControl, inLabel) {}

//	virtual bool	IsFocus() const;
//	virtual void	SetFocus();

	virtual bool	IsFocus() const								{ return false; }
	virtual void	SetFocus()									{}

	virtual void	AddedToWindow()								{ MXcbBasicControl::AddedToWindow(); }
//	virtual void	FrameMoved()								{ MXcbBasicControl::FrameMoved(); }
	virtual void	FrameResized()								{ MXcbBasicControl::FrameResized(); }
//	virtual void	MarginsChanged()							{ MXcbBasicControl::MarginsChanged(); }
//	virtual void	Draw(MRect inBounds)						{ MXcbBasicControl::AddedToWindow(); }
//	virtual void	Click(int32 inX, int32 inY)					{ MXcbBasicControl::AddedToWindow(); }
	virtual void	ActivateSelf()								{ MXcbBasicControl::ActivateSelf(); }
	virtual void	DeactivateSelf()							{ MXcbBasicControl::DeactivateSelf(); }
	virtual void	EnableSelf()								{ MXcbBasicControl::EnableSelf(); }
	virtual void	DisableSelf()								{ MXcbBasicControl::DisableSelf(); }
	virtual void	ShowSelf()									{ MXcbBasicControl::ShowSelf(); }
	virtual void	HideSelf()									{ MXcbBasicControl::HideSelf(); }


  protected:

	virtual void DestroyNotifyEvent(xcb_destroy_notify_event_t* inEvent)
	{
		if (this->mControl != nullptr)
		{
			this->mControl->SetImpl(nullptr);
			delete this;
		}
	
		MXcbWinMixin::DestroyNotifyEvent(inEvent);
	}
};

// actual implementations

class MX11SimpleControlImpl : public MX11ControlImpl<MSimpleControl>
{
  public:
					MX11SimpleControlImpl(MSimpleControl* inControl);
	virtual void	CreateWidget();
};

class MX11ButtonImpl : public MX11ControlImpl<MButton>
{
  public:
					MX11ButtonImpl(MButton* inButton, const std::string& inLabel,
						MButtonFlags inFlags);

	virtual void	SimulateClick();
	virtual void	MakeDefault(bool inDefault);

	virtual void	SetText(const std::string& inText);

	virtual void	DrawWidget();

	virtual void	GetIdealSize(int32& outWidth, int32& outHeight);

	virtual void	Clicked();

  private:

	MButtonFlags	mFlags;
	bool			mDefault;
};

////class MX11ImageButtonImpl : public MX11ControlImpl<MImageButton>
////{
////  public:
////					MX11ImageButtonImpl(MImageButton* inButton, const std::string& inImageResource);
////
////	virtual void	CreateWidget();
////
////	MSlot<void()>	mClicked;
////	void			Clicked();
////
////	MBitmap			mBitmaps[3];
////};
//
//class MX11ExpanderImpl : public MX11ControlImpl<MExpander>
//{
//public:
//					MX11ExpanderImpl(MExpander* inExpander, const std::string& inLabel);
//	virtual			~MX11ExpanderImpl();
//
//	virtual void	SetOpen(bool inOpen);
//	virtual bool	IsOpen() const;
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual void	Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//						bool inExpand, bool inFill, uint32 inPadding);
//
//  private:
//
//	bool			mIsOpen;
//	bool			mMouseInside;
//	bool			mMouseDown;
//	bool			mMouseTracking;
//	double			mLastExit;
//};
//
//class MX11ScrollbarImpl : public MX11ControlImpl<MScrollbar>
//{
//public:
//					MX11ScrollbarImpl(MScrollbar* inScrollbar);
//
//	virtual void	CreateWidget();
//
////	virtual void	ShowSelf();
////	virtual void	HideSelf();
//
//	virtual int32	GetValue() const;
//	virtual void	SetValue(int32 inValue);
//	
//	virtual int32	GetTrackValue() const;
//
//	virtual void	SetAdjustmentValues(int32 inMinValue, int32 inMaxValue,
//						int32 inScrollUnit,  int32 inPageSize, int32 inValue);
//
//	virtual int32	GetMinValue() const;
////	virtual void	SetMinValue(int32 inValue);
//	virtual int32	GetMaxValue() const;
////	virtual void	SetMaxValue(int32 inValue);
////
////	virtual void	SetViewSize(int32 inValue);
//
//	MSlot<void()>	eValueChanged;
//	void			ValueChanged();
//};
//
//class MX11StatusbarImpl : public MX11ControlImpl<MStatusbar>
//{
//  public:
//	MX11StatusbarImpl(MStatusbar* inControl, uint32 inPartCount, MStatusBarElement inParts[]);
//
//	virtual void CreateWidget();
//	virtual void SetStatusText(uint32 inPartNr, const std::string& inText, bool inBorder);
//	virtual void AddedToWindow();
//
//  private:
//
//	std::vector<MStatusBarElement> mParts;
//	std::vector<GtkWidget*> mPanels;
//
//	bool			Clicked(GdkEventButton* inEvent);
//	MSlot<bool(GdkEventButton*)>	mClicked;
//};
//
//class MX11ComboboxImpl : public MX11ControlImpl<MCombobox>
//{
//public:
//					MX11ComboboxImpl(MCombobox* inCombobox);
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual std::string
//					GetText() const;
//
//	virtual void	SetChoices(const std::vector<std::string>& inChoices);
//
//	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
//	virtual void	OnChanged();
//
//  private:
//	std::vector<std::string> mChoices;
//};
//
//class MX11PopupImpl : public MX11ControlImpl<MPopup>
//{
//public:
//					MX11PopupImpl(MPopup* inPopup);
//	
//	virtual void	SetChoices(const std::vector<std::string>& inChoices);
//
//	virtual int32	GetValue() const;
//	virtual void	SetValue(int32 inValue);
//	
//	virtual void	SetText(const std::string& inText);
//	virtual std::string
//					GetText() const;
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
//
//  private:
//	std::vector<std::string>
//					mChoices;
//};
//
//class MX11EdittextImpl : public MX11ControlImpl<MEdittext>
//{
//public:
//					MX11EdittextImpl(MEdittext* inEdittext, uint32 inFlags);
//	
//	virtual void	CreateWidget();
//
//	virtual void	SetFocus();
//
//	virtual std::string
//					GetText() const;
//	virtual void	SetText(const std::string& inText);
//
//	virtual uint32	GetFlags() const						{ return mFlags; }
//
//	virtual void	SetPasswordChar(uint32 inUnicode);
//
//	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
//
//protected:
//	uint32			mFlags;
//};
//
//class MX11CaptionImpl : public MX11ControlImpl<MCaption>
//{
//  public:
//					MX11CaptionImpl(MCaption* inControl, const std::string& inText);
//
//	virtual void	CreateWidget();
//
//	virtual void	SetText(const std::string& inText);
//};
//
//class MX11SeparatorImpl : public MX11ControlImpl<MSeparator>
//{
//  public:
//					MX11SeparatorImpl(MSeparator* inControl);
//	virtual void	CreateWidget();
//};
//
//class MX11CheckboxImpl : public MX11ControlImpl<MCheckbox>
//{
//  public:
//					MX11CheckboxImpl(MCheckbox* inControl, const std::string& inText);
//
//	virtual void	CreateWidget();
//
////	virtual void	SubClass();
//	virtual bool	IsChecked() const;
//	virtual void	SetChecked(bool inChecked);
//
//  private:
//	bool			mChecked;
//};
//
//class MX11RadiobuttonImpl : public MX11ControlImpl<MRadiobutton>
//{
//  public:
//					MX11RadiobuttonImpl(MRadiobutton* inControl, const std::string& inText);
//
//	virtual void	CreateWidget();
//
//	virtual bool	IsChecked() const;
//	virtual void	SetChecked(bool inChecked);
//
//	virtual void	SetGroup(const std::list<MRadiobutton*>& inButtons);
//
//  private:
//	std::list<MRadiobutton*> mGroup;
//};
//
//class MX11ListHeaderImpl : public MX11ControlImpl<MListHeader>
//{
//  public:
//					MX11ListHeaderImpl(MListHeader* inListHeader);
//
//	virtual void	CreateWidget();
//
//	virtual void	AppendColumn(const std::string& inLabel, int inWidth);
//};
//
//class MX11NotebookImpl : public MX11ControlImpl<MNotebook>
//{
//  public:
//					MX11NotebookImpl(MNotebook* inControl);
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//	virtual void	FrameResized();
//
//	virtual void	AddPage(const std::string& inLabel, MView* inPage);
//	
//	virtual void	SelectPage(uint32 inPage);
//	virtual uint32	GetSelectedPage() const;
//
//  private:
//	struct MPage
//	{
//		std::string	mTitle;
//		MView*		mPage;
//	};
//
//	std::vector<MPage> mPages;
//};
//
//class MX11ColorSwatchImpl : public MX11ControlImpl<MColorSwatch>
//{
//  public:
//					MX11ColorSwatchImpl(MColorSwatch* inColorSwatch, MColor inColor);
//
//	virtual void	CreateWidget();
//
//	virtual MColor	GetColor() const;
//	virtual void	SetColor(MColor inColor);
//
//  private:
//	
//	MEventIn<void(MColor)>	eSelectedColor;
//	void			SelectedColor(MColor inColor);
//
//	MEventIn<void(MColor)>	ePreviewColor;
//	void			PreviewColor(MColor inColor);
//
//	MSlot<void()>	mColorSet;
//	void			OnColorSet();
//
//	MColor			mColor;
//};
//
//class MX11ListBoxImpl : public MX11ControlImpl<MListBox>
//{
//  public:
//					MX11ListBoxImpl(MListBox* inListBox);
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual void	AddItem(const std::string& inLabel);
//
//	virtual int32	GetValue() const;
//	virtual void	SetValue(int32 inValue);
//
//  private:
//
//	MSlot<void()>	mSelectionChanged;
//	
//	virtual void	OnSelectionChanged();
//
//	std::vector<std::string> mItems;
//	GtkListStore*	mStore;
//	int32			mNr;
//};
//
//class MX11ListViewImpl : public MX11ControlImpl<MListView>
//{
//  public:
//					MX11ListViewImpl(MListView* inListView);
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual void	AddItem(const std::string& inLabel);
//
//  private:
//
//	std::vector<std::string> mItems;
//	GtkListStore* mStore;
//};
//
//class MX11BoxControlImpl : public MX11ControlImpl<MBoxControl>
//{
//  public:
//	MX11BoxControlImpl(MBoxControl* inControl,
//		bool inHorizontal, bool inHomogeneous, bool inExpand, bool inFill,
//		uint32 inSpacing, uint32 inPadding);
//
//	virtual void CreateWidget();
//
//	virtual void Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//		bool inExpand, bool inFill, uint32 inPadding);
//
//	bool mHorizontal, mHomogeneous, mExpand, mFill;
//	uint32 mSpacing, mPadding;
//};
