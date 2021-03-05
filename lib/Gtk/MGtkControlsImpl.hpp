//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MControlsImpl.hpp"
#include "MGtkWidgetMixin.hpp"

template<class CONTROL>
class MGtkControlImpl : public CONTROL::MImpl, public MGtkWidgetMixin
{
  public:
					MGtkControlImpl(CONTROL* inControl, const std::string& inLabel);
	virtual			~MGtkControlImpl();

	virtual bool	IsFocus() const;
	virtual void	SetFocus();

	virtual void	AddedToWindow();
	virtual void	FrameMoved();
	virtual void	FrameResized();
	virtual void	MarginsChanged();
	virtual void	ActivateSelf();
	virtual void	DeactivateSelf();
	virtual void	EnableSelf();
	virtual void	DisableSelf();
	virtual void	ShowSelf();
	virtual void	HideSelf();
	virtual std::string
					GetText() const;
	virtual void	SetText(const std::string& inText);

  protected:

	void GetParentAndBounds(MGtkWidgetMixin*& outParent, MRect& outBounds);

	virtual void	CreateWidget() = 0;

	virtual bool	OnDestroy();

	virtual bool	OnKeyPressEvent(GdkEventKey* inEvent);
	virtual void	OnPopupMenu();

	virtual void	OnChanged();
	MSlot<void()>	mChanged;

	std::string		mLabel;
};

// actual implementations

class MGtkSimpleControlImpl : public MGtkControlImpl<MSimpleControl>
{
  public:
					MGtkSimpleControlImpl(MSimpleControl* inControl);
	virtual void	CreateWidget();
};

class MGtkButtonImpl : public MGtkControlImpl<MButton>
{
  public:
					MGtkButtonImpl(MButton* inButton, const std::string& inLabel,
						MButtonFlags inFlags);

	virtual void	SimulateClick();
	virtual void	MakeDefault(bool inDefault);

	virtual void	SetText(const std::string& inText);

	virtual void	CreateWidget();
	virtual void	GetIdealSize(int32& outWidth, int32& outHeight);

	virtual void	AddedToWindow();

	MSlot<void()>	mClicked;
	void			Clicked();

  private:

	// MButtonFlags	mFlags;
	bool			mDefault;
};

//class MGtkImageButtonImpl : public MGtkControlImpl<MImageButton>
//{
//  public:
//					MGtkImageButtonImpl(MImageButton* inButton, const std::string& inImageResource);
//
//	virtual void	CreateWidget();
//
//	MSlot<void()>	mClicked;
//	void			Clicked();
//
//	MBitmap			mBitmaps[3];
//};

class MGtkExpanderImpl : public MGtkControlImpl<MExpander>
{
public:
					MGtkExpanderImpl(MExpander* inExpander, const std::string& inLabel);
	virtual			~MGtkExpanderImpl();

	virtual void	SetOpen(bool inOpen);
	virtual bool	IsOpen() const;

	virtual void	CreateWidget();
	virtual void	AddedToWindow();

	virtual void	Append(MGtkWidgetMixin* inChild, MControlPacking inPacking,
						bool inExpand, bool inFill, uint32 inPadding);

  private:

	bool			mIsOpen;
	// bool			mMouseInside;
	// bool			mMouseDown;
	// bool			mMouseTracking;
	// double			mLastExit;
};

class MGtkScrollbarImpl : public MGtkControlImpl<MScrollbar>
{
public:
					MGtkScrollbarImpl(MScrollbar* inScrollbar);

	virtual void	CreateWidget();

//	virtual void	ShowSelf();
//	virtual void	HideSelf();

	virtual int32	GetValue() const;
	virtual void	SetValue(int32 inValue);
	
	virtual int32	GetTrackValue() const;

	virtual void	SetAdjustmentValues(int32 inMinValue, int32 inMaxValue,
						int32 inScrollUnit,  int32 inPageSize, int32 inValue);

	virtual int32	GetMinValue() const;
//	virtual void	SetMinValue(int32 inValue);
	virtual int32	GetMaxValue() const;
//	virtual void	SetMaxValue(int32 inValue);
//
//	virtual void	SetViewSize(int32 inValue);

	MSlot<void()>	eValueChanged;
	void			ValueChanged();
};

class MGtkStatusbarImpl : public MGtkControlImpl<MStatusbar>
{
  public:
	MGtkStatusbarImpl(MStatusbar* inControl, uint32 inPartCount, MStatusBarElement inParts[]);

	virtual void CreateWidget();
	virtual void SetStatusText(uint32 inPartNr, const std::string& inText, bool inBorder);
	virtual void AddedToWindow();

  private:

	std::vector<MStatusBarElement> mParts;
	std::vector<GtkWidget*> mPanels;

	bool			Clicked(GdkEventButton* inEvent);
	MSlot<bool(GdkEventButton*)>	mClicked;
};

class MGtkComboboxImpl : public MGtkControlImpl<MCombobox>
{
public:
					MGtkComboboxImpl(MCombobox* inCombobox);

	virtual void	CreateWidget();
	virtual void	AddedToWindow();

	virtual std::string
					GetText() const;

	virtual void	SetChoices(const std::vector<std::string>& inChoices);

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
	virtual void	OnChanged();

  private:
	std::vector<std::string> mChoices;
};

class MGtkPopupImpl : public MGtkControlImpl<MPopup>
{
public:
					MGtkPopupImpl(MPopup* inPopup);
	
	virtual void	SetChoices(const std::vector<std::string>& inChoices);

	virtual int32	GetValue() const;
	virtual void	SetValue(int32 inValue);
	
	virtual void	SetText(const std::string& inText);
	virtual std::string
					GetText() const;

	virtual void	CreateWidget();
	virtual void	AddedToWindow();

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);

  private:
	std::vector<std::string>
					mChoices;
};

class MGtkEdittextImpl : public MGtkControlImpl<MEdittext>
{
public:
					MGtkEdittextImpl(MEdittext* inEdittext, uint32 inFlags);
	
	virtual void	CreateWidget();

	virtual void	SetFocus();

	virtual std::string
					GetText() const;
	virtual void	SetText(const std::string& inText);

	virtual uint32	GetFlags() const						{ return mFlags; }

	virtual void	SetPasswordChar(uint32 inUnicode);

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);

protected:
	uint32			mFlags;
};

class MGtkCaptionImpl : public MGtkControlImpl<MCaption>
{
  public:
					MGtkCaptionImpl(MCaption* inControl, const std::string& inText);

	virtual void	CreateWidget();

	virtual void	SetText(const std::string& inText);
};

class MGtkSeparatorImpl : public MGtkControlImpl<MSeparator>
{
  public:
					MGtkSeparatorImpl(MSeparator* inControl);
	virtual void	CreateWidget();
};

class MGtkCheckboxImpl : public MGtkControlImpl<MCheckbox>
{
  public:
					MGtkCheckboxImpl(MCheckbox* inControl, const std::string& inText);

	virtual void	CreateWidget();

//	virtual void	SubClass();
	virtual bool	IsChecked() const;
	virtual void	SetChecked(bool inChecked);

  private:
	bool			mChecked;
};

class MGtkRadiobuttonImpl : public MGtkControlImpl<MRadiobutton>
{
  public:
					MGtkRadiobuttonImpl(MRadiobutton* inControl, const std::string& inText);

	virtual void	CreateWidget();

	virtual bool	IsChecked() const;
	virtual void	SetChecked(bool inChecked);

	virtual void	SetGroup(const std::list<MRadiobutton*>& inButtons);

  private:
	std::list<MRadiobutton*> mGroup;
};

class MGtkListHeaderImpl : public MGtkControlImpl<MListHeader>
{
  public:
					MGtkListHeaderImpl(MListHeader* inListHeader);

	virtual void	CreateWidget();

	virtual void	AppendColumn(const std::string& inLabel, int inWidth);
};

class MGtkNotebookImpl : public MGtkControlImpl<MNotebook>
{
  public:
					MGtkNotebookImpl(MNotebook* inControl);

	virtual void	CreateWidget();
	virtual void	AddedToWindow();
	virtual void	FrameResized();

	virtual void	AddPage(const std::string& inLabel, MView* inPage);
	
	virtual void	SelectPage(uint32 inPage);
	virtual uint32	GetSelectedPage() const;

  private:
	struct MPage
	{
		std::string	mTitle;
		MView*		mPage;
	};

	std::vector<MPage> mPages;
};

class MGtkColorSwatchImpl : public MGtkControlImpl<MColorSwatch>
{
  public:
					MGtkColorSwatchImpl(MColorSwatch* inColorSwatch, MColor inColor);

	virtual void	CreateWidget();

	virtual MColor	GetColor() const;
	virtual void	SetColor(MColor inColor);

  private:
	
	MEventIn<void(MColor)>	eSelectedColor;
	void			SelectedColor(MColor inColor);

	MEventIn<void(MColor)>	ePreviewColor;
	void			PreviewColor(MColor inColor);

	MSlot<void()>	mColorSet;
	void			OnColorSet();

	MColor			mColor;
};

class MGtkListBoxImpl : public MGtkControlImpl<MListBox>
{
  public:
					MGtkListBoxImpl(MListBox* inListBox);

	virtual void	CreateWidget();
	virtual void	AddedToWindow();

	virtual void	AddItem(const std::string& inLabel);

	virtual int32	GetValue() const;
	virtual void	SetValue(int32 inValue);

  private:

	MSlot<void()>	mSelectionChanged;
	
	virtual void	OnSelectionChanged();

	std::vector<std::string> mItems;
	GtkListStore*	mStore;
	int32			mNr;
};

class MGtkListViewImpl : public MGtkControlImpl<MListView>
{
  public:
					MGtkListViewImpl(MListView* inListView);

	virtual void	CreateWidget();
	virtual void	AddedToWindow();

	virtual void	AddItem(const std::string& inLabel);

  private:

	std::vector<std::string> mItems;
	GtkListStore* mStore;
};

class MGtkBoxControlImpl : public MGtkControlImpl<MBoxControl>
{
  public:
	MGtkBoxControlImpl(MBoxControl* inControl,
		bool inHorizontal, bool inHomogeneous, bool inExpand, bool inFill,
		uint32 inSpacing, uint32 inPadding);

	virtual void CreateWidget();

	virtual void Append(MGtkWidgetMixin* inChild, MControlPacking inPacking,
		bool inExpand, bool inFill, uint32 inPadding);

	bool mHorizontal, mHomogeneous, mExpand, mFill;
	uint32 mSpacing, mPadding;
};
