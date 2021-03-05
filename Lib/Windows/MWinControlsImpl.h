//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MWINCONTROLSIMPL_H
#define MWINCONTROLSIMPL_H

#include "MControlsImpl.h"
#include "MWinProcMixin.h"

template<class CONTROL>
class MWinControlImpl : public CONTROL::MImpl, public MWinProcMixin
{
public:
					MWinControlImpl(CONTROL* inControl, const std::string& inLabel);
	virtual			~MWinControlImpl();

	virtual void	GetParentAndBounds(MWinProcMixin*& outParent, MRect& outBounds);

	virtual void	SetFocus();

	virtual void	AddedToWindow();
	virtual void	FrameMoved();
	virtual void	FrameResized();
	virtual void	Draw(MRect inBounds);
	virtual void	ActivateSelf();
	virtual void	DeactivateSelf();
	virtual void	EnableSelf();
	virtual void	DisableSelf();
	virtual void	ShowSelf();
	virtual void	HideSelf();
	virtual std::string
					GetText() const;
	virtual void	SetText(const std::string& inText);

	virtual MWinProcMixin*
					GetWinProcMixin();

protected:
	std::string		mLabel;
};

// actual implementations

class MWinSimpleControlImpl : public MWinControlImpl<MSimpleControl>
{
  public:
					MWinSimpleControlImpl(MSimpleControl* inControl);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
};

class MWinButtonImpl : public MWinControlImpl<MButton>
{
  public:
					MWinButtonImpl(MButton* inButton, const std::string& inLabel,
						MButtonFlags inFlags);

	virtual void	SimulateClick();
	virtual void	MakeDefault(bool inDefault);

	virtual void	SetText(const std::string& inText);

	virtual void	GetIdealSize(int32& outWidth, int32& outHeight);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	CreateHandle(MWinProcMixin* inParent, MRect inBounds,
						const std::wstring& inTitle);

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMGetDlgCode(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	BCMSetDropDownState(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	virtual void	AddedToWindow();

  private:

	MButtonFlags	mFlags;
	bool			mDefault;
};

class MWinExpanderImpl : public MWinControlImpl<MExpander>
{
public:
					MWinExpanderImpl(MExpander* inExpander, const std::string& inLabel);
	virtual			~MWinExpanderImpl();

	virtual void	SetOpen(bool inOpen);
	virtual bool	IsOpen() const;

	virtual void	AddedToWindow();
	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual bool	WMPaint(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMActivate(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

  private:

	bool			mIsOpen;
	bool			mMouseInside;
	bool			mMouseDown;
	bool			mMouseTracking;
	double			mLastExit;
	HDC				mDC;
};

class MWinScrollbarImpl : public MWinControlImpl<MScrollbar>
{
public:
					MWinScrollbarImpl(MScrollbar* inScrollbar);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual void	ShowSelf();
	virtual void	HideSelf();

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

	virtual bool	WMScroll(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
};

class MWinStatusbarImpl : public MWinControlImpl<MStatusbar>
{
public:
					MWinStatusbarImpl(MStatusbar* inControl, uint32 inPartCount, MStatusBarElement inParts[]);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	CreateHandle(MWinProcMixin* inParent, MRect inBounds,
						const std::wstring& inTitle);

	virtual void	SetStatusText(uint32 inPartNr, const std::string& inText, bool inBorder);

	virtual void	AddedToWindow();

	virtual bool	NMClick(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

private:
	std::vector<int32>		mOffsets;
};

class MWinComboboxImpl : public MWinControlImpl<MCombobox>
{
public:
					MWinComboboxImpl(MCombobox* inCombobox);
	
	virtual void	SetChoices(const std::vector<std::string>& inChoices);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	AddedToWindow();

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseWheel(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);

  private:

	class MEditPart : public MWinProcMixin
	{
	  public:
							MEditPart(MWinComboboxImpl* inImpl)
								: MWinProcMixin(nullptr), mImpl(inImpl)								{}
		virtual bool		DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat)
							{
								return mImpl->DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
							}

	  private:
		MWinComboboxImpl*	mImpl;
	};
	
	MEditPart		mEditor;
	std::vector<std::string>
					mChoices;
};

class MWinPopupImpl : public MWinControlImpl<MPopup>
{
public:
					MWinPopupImpl(MPopup* inPopup);
	
	virtual void	SetChoices(const std::vector<std::string>& inChoices);

	virtual int32	GetValue() const;
	virtual void	SetValue(int32 inValue);
	
	virtual void	SetText(const std::string& inText);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	AddedToWindow();

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseWheel(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);

private:
	std::vector<std::string>
					mChoices;
};

class MWinEdittextImpl : public MWinControlImpl<MEdittext>
{
public:
					MWinEdittextImpl(MEdittext* inEdittext, uint32 inFlags);
	
	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual void	SetFocus();

	virtual std::string
					GetText() const;
	virtual void	SetText(const std::string& inText);

	virtual uint32	GetFlags() const						{ return mFlags; }

	virtual void	SetPasswordChar(uint32 inUnicode);

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

protected:
	uint32			mFlags;
};

class MWinCaptionImpl : public MWinControlImpl<MCaption>
{
public:
					MWinCaptionImpl(MCaption* inControl, const std::string& inText);

	virtual void	SetText(const std::string& inText);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual bool	WMDrawItem(DRAWITEMSTRUCT* inDrawItemStruct);
	

private:
	std::wstring	mText;
};

class MWinSeparatorImpl : public MWinControlImpl<MSeparator>
{
public:
					MWinSeparatorImpl(MSeparator* inControl);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
};

class MWinCheckboxImpl : public MWinControlImpl<MCheckbox>
{
  public:
					MWinCheckboxImpl(MCheckbox* inControl, const std::string& inText);

//	virtual void	SubClass();
	virtual bool	IsChecked() const;
	virtual void	SetChecked(bool inChecked);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
};

class MWinRadiobuttonImpl : public MWinControlImpl<MRadiobutton>
{
  public:
					MWinRadiobuttonImpl(MRadiobutton* inControl, const std::string& inText);

	virtual bool	IsChecked() const;
	virtual void	SetChecked(bool inChecked);

	virtual void	SetGroup(const std::list<MRadiobutton*>& inButtons);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

  private:
	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	std::list<MRadiobutton*>
					mGroup;
};

class MWinListHeaderImpl : public MWinControlImpl<MListHeader>
{
  public:
					MWinListHeaderImpl(MListHeader* inListHeader);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	CreateHandle(MWinProcMixin* inParent, MRect inBounds,
						const std::wstring& inTitle);

	virtual void	AppendColumn(const std::string& inLabel, int inWidth);

	virtual bool	HDNBeginDrag(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	HDNBeginTrack(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	HDNTrack(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
};

class MWinNotebookImpl : public MWinControlImpl<MNotebook>
{
  public:
					MWinNotebookImpl(MNotebook* inControl);

	virtual void	AddedToWindow();
	virtual void	FrameResized();

	virtual void	AddPage(const std::string& inLabel, MView* inPage);
	
	virtual void	SelectPage(uint32 inPage);
	virtual uint32	GetSelectedPage() const;

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual bool	TCNSelChange(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

  private:
	struct MPage
	{
		std::string	mTitle;
		MView*		mPage;
	};

	std::vector<MPage>
					mPages;
};

class MWinColorSwatchImpl : public MWinControlImpl<MColorSwatch>
{
  public:
					MWinColorSwatchImpl(MColorSwatch* inColorSwatch, MColor inColor);

	virtual MColor	GetColor() const;
	virtual void	SetColor(MColor inColor);

	//virtual void	GetIdealSize(int32& outWidth, int32& outHeight);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

  private:
	
	MEventIn<void(MColor)>	eSelectedColor;
	void			SelectedColor(MColor inColor);

	MEventIn<void(MColor)>	ePreviewColor;
	void			PreviewColor(MColor inColor);

	MColor			mColor;
};

class MWinListBoxImpl : public MWinControlImpl<MListBox>
{
  public:
					MWinListBoxImpl(MListBox* inListBox);

	virtual void	AddItem(const std::string& inLabel);

	virtual int32	GetValue() const;
	virtual void	SetValue(int32 inValue);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);

	virtual void	AddedToWindow();

  private:

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	std::vector<std::string> mItems;
};

class MWinListViewImpl : public MWinControlImpl<MListView>
{
  public:
					MWinListViewImpl(MListView* inListView);

	virtual void	AddItem(const std::string& inLabel);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	CreateHandle(MWinProcMixin* inParent, MRect inBounds,
						const std::wstring& inTitle);

	virtual void	AddedToWindow();

  private:

	virtual bool	LVMItemActivate(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	LVMGetDispInfo(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	std::vector<std::string> mItems;
};



#endif
