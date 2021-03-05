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
	virtual void ButtonReleaseEvent(xcb_button_press_event_t* inEvent);

	std::string mLabel;

	enum { Button1, Button2, Button3, Button4, Button5 };
	bool mButtonPressed[5];
};

template<class CONTROL>
class MXcbControlImpl : public CONTROL::MImpl, public MXcbBasicControl
{
  public:
					MXcbControlImpl(CONTROL* inControl, const std::string& inLabel)
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
//	virtual void	Click(int32_t inX, int32_t inY)					{ MXcbBasicControl::AddedToWindow(); }
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

class MXcbSimpleControlImpl : public MXcbControlImpl<MSimpleControl>
{
  public:
					MXcbSimpleControlImpl(MSimpleControl* inControl);
};

class MXcbButtonImpl : public MXcbControlImpl<MButton>
{
  public:
					MXcbButtonImpl(MButton* inButton, const std::string& inLabel,
						MButtonFlags inFlags);

	virtual void	SimulateClick();
	virtual void	MakeDefault(bool inDefault);

	virtual void	SetText(const std::string& inText);

	virtual void	DrawWidget(MGfxDevice& dev);

	virtual void	GetIdealSize(int32_t& outWidth, int32_t& outHeight);

	virtual void	Clicked();

	virtual void ButtonPressEvent(xcb_button_press_event_t* inEvent);
	virtual void ButtonReleaseEvent(xcb_button_press_event_t* inEvent);
	virtual void MotionNotifyEvent(xcb_motion_notify_event_t* inEvent);
	virtual void EnterNotifyEvent(xcb_enter_notify_event_t* inEvent);
	virtual void LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent);

  private:
	
	enum {
		kBtnStateNormal,
		kBtnStatePressed
	} mBtnState = kBtnStateNormal;

	MButtonFlags	mFlags;
	bool			mDefault;
};

////class MXcbImageButtonImpl : public MXcbControlImpl<MImageButton>
////{
////  public:
////					MXcbImageButtonImpl(MImageButton* inButton, const std::string& inImageResource);
////
////	virtual void	CreateWidget();
////
////	MSlot<void()>	mClicked;
////	void			Clicked();
////
////	MBitmap			mBitmaps[3];
////};
//
//class MXcbExpanderImpl : public MXcbControlImpl<MExpander>
//{
//public:
//					MXcbExpanderImpl(MExpander* inExpander, const std::string& inLabel);
//	virtual			~MXcbExpanderImpl();
//
//	virtual void	SetOpen(bool inOpen);
//	virtual bool	IsOpen() const;
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual void	Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//						bool inExpand, bool inFill, uint32_t inPadding);
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
//class MXcbScrollbarImpl : public MXcbControlImpl<MScrollbar>
//{
//public:
//					MXcbScrollbarImpl(MScrollbar* inScrollbar);
//
//	virtual void	CreateWidget();
//
////	virtual void	ShowSelf();
////	virtual void	HideSelf();
//
//	virtual int32_t	GetValue() const;
//	virtual void	SetValue(int32_t inValue);
//	
//	virtual int32_t	GetTrackValue() const;
//
//	virtual void	SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
//						int32_t inScrollUnit,  int32_t inPageSize, int32_t inValue);
//
//	virtual int32_t	GetMinValue() const;
////	virtual void	SetMinValue(int32_t inValue);
//	virtual int32_t	GetMaxValue() const;
////	virtual void	SetMaxValue(int32_t inValue);
////
////	virtual void	SetViewSize(int32_t inValue);
//
//	MSlot<void()>	eValueChanged;
//	void			ValueChanged();
//};
//
//class MXcbStatusbarImpl : public MXcbControlImpl<MStatusbar>
//{
//  public:
//	MXcbStatusbarImpl(MStatusbar* inControl, uint32_t inPartCount, MStatusBarElement inParts[]);
//
//	virtual void CreateWidget();
//	virtual void SetStatusText(uint32_t inPartNr, const std::string& inText, bool inBorder);
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
//class MXcbComboboxImpl : public MXcbControlImpl<MCombobox>
//{
//public:
//					MXcbComboboxImpl(MCombobox* inCombobox);
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual std::string
//					GetText() const;
//
//	virtual void	SetChoices(const std::vector<std::string>& inChoices);
//
//	virtual bool	DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
//	virtual void	OnChanged();
//
//  private:
//	std::vector<std::string> mChoices;
//};
//
//class MXcbPopupImpl : public MXcbControlImpl<MPopup>
//{
//public:
//					MXcbPopupImpl(MPopup* inPopup);
//	
//	virtual void	SetChoices(const std::vector<std::string>& inChoices);
//
//	virtual int32_t	GetValue() const;
//	virtual void	SetValue(int32_t inValue);
//	
//	virtual void	SetText(const std::string& inText);
//	virtual std::string
//					GetText() const;
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual bool	DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
//
//  private:
//	std::vector<std::string>
//					mChoices;
//};
//
//class MXcbEdittextImpl : public MXcbControlImpl<MEdittext>
//{
//public:
//					MXcbEdittextImpl(MEdittext* inEdittext, uint32_t inFlags);
//	
//	virtual void	CreateWidget();
//
//	virtual void	SetFocus();
//
//	virtual std::string
//					GetText() const;
//	virtual void	SetText(const std::string& inText);
//
//	virtual uint32_t	GetFlags() const						{ return mFlags; }
//
//	virtual void	SetPasswordChar(uint32_t inUnicode);
//
//	virtual bool	DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
//
//protected:
//	uint32_t			mFlags;
//};
//
//class MXcbCaptionImpl : public MXcbControlImpl<MCaption>
//{
//  public:
//					MXcbCaptionImpl(MCaption* inControl, const std::string& inText);
//
//	virtual void	CreateWidget();
//
//	virtual void	SetText(const std::string& inText);
//};
//
//class MXcbSeparatorImpl : public MXcbControlImpl<MSeparator>
//{
//  public:
//					MXcbSeparatorImpl(MSeparator* inControl);
//	virtual void	CreateWidget();
//};
//
//class MXcbCheckboxImpl : public MXcbControlImpl<MCheckbox>
//{
//  public:
//					MXcbCheckboxImpl(MCheckbox* inControl, const std::string& inText);
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
//class MXcbRadiobuttonImpl : public MXcbControlImpl<MRadiobutton>
//{
//  public:
//					MXcbRadiobuttonImpl(MRadiobutton* inControl, const std::string& inText);
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
//class MXcbListHeaderImpl : public MXcbControlImpl<MListHeader>
//{
//  public:
//					MXcbListHeaderImpl(MListHeader* inListHeader);
//
//	virtual void	CreateWidget();
//
//	virtual void	AppendColumn(const std::string& inLabel, int inWidth);
//};
//
//class MXcbNotebookImpl : public MXcbControlImpl<MNotebook>
//{
//  public:
//					MXcbNotebookImpl(MNotebook* inControl);
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//	virtual void	FrameResized();
//
//	virtual void	AddPage(const std::string& inLabel, MView* inPage);
//	
//	virtual void	SelectPage(uint32_t inPage);
//	virtual uint32_t	GetSelectedPage() const;
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
//class MXcbColorSwatchImpl : public MXcbControlImpl<MColorSwatch>
//{
//  public:
//					MXcbColorSwatchImpl(MColorSwatch* inColorSwatch, MColor inColor);
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
//class MXcbListBoxImpl : public MXcbControlImpl<MListBox>
//{
//  public:
//					MXcbListBoxImpl(MListBox* inListBox);
//
//	virtual void	CreateWidget();
//	virtual void	AddedToWindow();
//
//	virtual void	AddItem(const std::string& inLabel);
//
//	virtual int32_t	GetValue() const;
//	virtual void	SetValue(int32_t inValue);
//
//  private:
//
//	MSlot<void()>	mSelectionChanged;
//	
//	virtual void	OnSelectionChanged();
//
//	std::vector<std::string> mItems;
//	GtkListStore*	mStore;
//	int32_t			mNr;
//};
//
//class MXcbListViewImpl : public MXcbControlImpl<MListView>
//{
//  public:
//					MXcbListViewImpl(MListView* inListView);
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
//class MXcbBoxControlImpl : public MXcbControlImpl<MBoxControl>
//{
//  public:
//	MXcbBoxControlImpl(MBoxControl* inControl,
//		bool inHorizontal, bool inHomogeneous, bool inExpand, bool inFill,
//		uint32_t inSpacing, uint32_t inPadding);
//
//	virtual void CreateWidget();
//
//	virtual void Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//		bool inExpand, bool inFill, uint32_t inPadding);
//
//	bool mHorizontal, mHomogeneous, mExpand, mFill;
//	uint32_t mSpacing, mPadding;
//};
