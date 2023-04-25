//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MColor.hpp"
#include "MHandler.hpp"
#include "MP2PEvents.hpp"
#include "MView.hpp"

struct MControlImplBase;

class MControlBase : public MView, public MHandler
{
  public:
	MControlBase(const std::string &inID, MRect inBounds)
		: MView(inID, inBounds)
		, MHandler(nullptr)
		, mPacking(ePackStart)
		, mPadding(0)
		, mExpand(false)
		, mFill(false)
	{
	}

	virtual MControlImplBase *GetControlImplBase() = 0;

	virtual bool IsFocus() const = 0;
	virtual void SetFocus() = 0;
	virtual MHandler *
	FindFocus() { return IsFocus() ? this : MView::FindFocus(); }

	// for auto layout of controls
	MControlPacking GetPacking() const { return mPacking; }
	void SetPacking(MControlPacking inPacking)
	{
		mPacking = inPacking;
	}

	uint32_t GetPadding() const { return mPadding; }
	void SetPadding(uint32_t inPadding) { mPadding = inPadding; }

	bool GetExpand() const { return mExpand; }
	void SetExpand(bool inExpand) { mExpand = inExpand; }

	bool GetFill() const { return mFill; }
	void SetFill(bool inFill) { mFill = inFill; }

	void SetLayout(MControlPacking inPacking, bool inExpand, bool inFill, uint32_t inPadding)
	{
		mPacking = inPacking;
		mExpand = inExpand;
		mFill = inFill;
		mPadding = inPadding;
	}

  protected:
	MControlPacking mPacking;
	uint32_t mPadding;
	bool mExpand, mFill;
};

template <class I>
class MControl : public MControlBase
{
  public:
	virtual ~MControl();

	virtual void MoveFrame(int32_t inXDelta, int32_t inYDelta);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);
	virtual void SetMargins(int32_t inLeftMargin, int32_t inTopMargin, int32_t inRightMargin, int32_t inBottomMargin);

	// virtual void	Draw(MRect inUpdate);
	virtual void Draw();

	virtual bool IsFocus() const;
	virtual void SetFocus();

	I *GetImpl() const { return mImpl; }
	void SetImpl(I *inImpl) { mImpl = inImpl; }

	virtual MControlImplBase *GetControlImplBase();

  protected:
	MControl(const std::string &inID, MRect inBounds, I *inImpl);

	virtual void ActivateSelf();
	virtual void DeactivateSelf();

	virtual void EnableSelf();
	virtual void DisableSelf();

	virtual void ShowSelf();
	virtual void HideSelf();

	virtual void AddedToWindow();

  protected:
	MControl(const MControl &);
	MControl &operator=(const MControl &);

	I *mImpl;
};

// --------------------------------------------------------------------

class MSimpleControlImpl;

class MSimpleControl : public MControl<MSimpleControlImpl>
{
  public:
	typedef MSimpleControlImpl MImpl;

	MSimpleControl(const std::string &inID, MRect inBounds);
};

// --------------------------------------------------------------------

class MButtonImpl;

enum MButtonFlags
{
	eBF_None = 0,
	eBF_Split = (1 << 0),
};

class MButton : public MControl<MButtonImpl>
{
  public:
	typedef MButtonImpl MImpl;

	MButton(const std::string &inID, MRect inBounds, const std::string &inLabel,
	        MButtonFlags = eBF_None);

	void SimulateClick();
	void MakeDefault(bool inDefault = true);

	virtual void SetText(const std::string &inText);

	MEventOut<void(const std::string &)>
		eClicked;
	MEventOut<void(const std::string &, int32_t, int32_t)>
		eDropDown;
};

//// --------------------------------------------------------------------
//
//class MImageButton : public MControl<MImageButtonImpl>
//{
//  public:
//	typedef MImageButtonImpl MImpl;
//
//					MImageButtonImpl(const std::string& inID, MRect inBounds,
//						const std::string& inImageResource);
//
//	MEventOut<void(const std::string&)> eClicked;
//};

// --------------------------------------------------------------------

class MExpanderImpl;

class MExpander : public MControl<MExpanderImpl>
{
  public:
	typedef MExpanderImpl MImpl;

	MExpander(const std::string &inID, MRect inBounds, const std::string &inLabel);

	void SetOpen(bool inOpen);
	bool IsOpen() const;

	MEventOut<void(const std::string &)>
		eClicked;
};

// --------------------------------------------------------------------

extern const int kScrollbarWidth;
class MScrollbarImpl;

class MScrollbar : public MControl<MScrollbarImpl>
{
  public:
	typedef MScrollbarImpl MImpl;

	MScrollbar(const std::string &inID, MRect inBounds);

	virtual int32_t GetValue() const;
	virtual void SetValue(int32_t inValue);

	virtual int32_t GetTrackValue() const;
	virtual int32_t GetMinValue() const;
	virtual int32_t GetMaxValue() const;

	virtual void SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
	                                 int32_t inScrollUnit, int32_t inPageSize, int32_t inValue);

	MEventOut<void(MScrollMessage)> eScroll;
};

// --------------------------------------------------------------------

class MStatusbarImpl;

struct MStatusBarElement
{
	uint32_t width;
	MControlPacking packing;
	uint32_t padding;
	bool fill;
	bool expand;
};

class MStatusbar : public MControl<MStatusbarImpl>
{
  public:
	typedef MStatusbarImpl MImpl;

	MStatusbar(const std::string &inID, MRect inBounds,
	           uint32_t inPartCount, MStatusBarElement inParts[]);

	virtual void SetStatusText(uint32_t inPartNr, const std::string &inText, bool inBorder);

	MEventOut<void(uint32_t, MRect)> ePartClicked;
};

// --------------------------------------------------------------------

class MComboboxImpl;

class MCombobox : public MControl<MComboboxImpl>
{
  public:
	typedef MComboboxImpl MImpl;

	MCombobox(const std::string &inID, MRect inBounds);

	MEventOut<void(const std::string &, const std::string &)>
		eValueChanged;

	virtual void SetText(const std::string &inText);
	virtual std::string
	GetText() const;

	virtual void SetChoices(const std::vector<std::string> &inChoices);
};

// --------------------------------------------------------------------

class MPopupImpl;

class MPopup : public MControl<MPopupImpl>
{
  public:
	typedef MPopupImpl MImpl;

	MPopup(const std::string &inID, MRect inBounds);

	MEventOut<void(const std::string &, int32_t)>
		eValueChanged;

	virtual void SetValue(int32_t inValue);
	virtual int32_t GetValue() const;

	virtual void SetText(const std::string &inText);
	virtual std::string
	GetText() const;

	virtual void SetChoices(const std::vector<std::string> &inChoices);
};

// --------------------------------------------------------------------

class MCaptionImpl;

class MCaption : public MControl<MCaptionImpl>
{
  public:
	typedef MCaptionImpl MImpl;

	MCaption(const std::string &inID, MRect inBounds,
	         const std::string &inText);

	virtual void SetText(const std::string &inText);
};

// --------------------------------------------------------------------

class MEdittextImpl;

enum
{
	eMEditTextNoFlags = 0,
	eMEditTextAlignRight = 1 << 0,
	eMEditTextNumbers = 1 << 1,
	eMEditTextMultiLine = 1 << 2,
	eMEditTextReadOnly = 1 << 3
};

class MEdittext : public MControl<MEdittextImpl>
{
  public:
	typedef MEdittextImpl MImpl;

	MEdittext(const std::string &inID, MRect inBounds,
	          uint32_t inFlags = eMEditTextNoFlags);

	MEventOut<void(const std::string &, const std::string &)> eValueChanged;

	virtual void SetText(const std::string &inText);
	virtual std::string
	GetText() const;

	uint32_t GetFlags() const;

	virtual void SetPasswordChar(uint32_t inUnicode = 0x2022);
};

// --------------------------------------------------------------------

class MSeparatorImpl;

class MSeparator : public MControl<MSeparatorImpl>
{
  public:
	typedef MSeparatorImpl MImpl;

	MSeparator(const std::string &inID, MRect inBounds);
};

// --------------------------------------------------------------------

class MCheckboxImpl;

class MCheckbox : public MControl<MCheckboxImpl>
{
  public:
	typedef MCheckboxImpl MImpl;

	MCheckbox(const std::string &inID, MRect inBounds,
	          const std::string &inTitle);

	bool IsChecked() const;
	void SetChecked(bool inChecked);

	MEventOut<void(const std::string &, bool)>
		eValueChanged;
};

// --------------------------------------------------------------------

class MRadiobuttonImpl;

class MRadiobutton : public MControl<MRadiobuttonImpl>
{
  public:
	typedef MRadiobuttonImpl MImpl;

	MRadiobutton(const std::string &inID, MRect inBounds,
	             const std::string &inTitle);

	bool IsChecked() const;
	void SetChecked(bool inChecked);

	void SetGroup(const std::list<MRadiobutton *> &inButtons);

	MEventOut<void(const std::string &, bool)>
		eValueChanged;
};

// --------------------------------------------------------------------

class MListHeaderImpl;

class MListHeader : public MControl<MListHeaderImpl>
{
  public:
	typedef MListHeaderImpl MImpl;
	MListHeader(const std::string &inID, MRect inBounds);

	MEventOut<void(uint32_t, uint32_t)> eColumnResized;

	void AppendColumn(const std::string &inLabel, int inWidth = -1);
};

// --------------------------------------------------------------------

class MNotebookImpl;

class MNotebook : public MControl<MNotebookImpl>
{
  public:
	typedef MNotebookImpl MImpl;

	MNotebook(const std::string &inID, MRect inBounds);

	void AddPage(const std::string &inLabel, MView *inPage);

	void SelectPage(uint32_t inPage);
	uint32_t GetSelectedPage() const;

	MEventOut<void(uint32_t)>
		ePageSelected;
};

// --------------------------------------------------------------------

class MColorSwatchImpl;

class MColorSwatch : public MControl<MColorSwatchImpl>
{
  public:
	typedef MColorSwatchImpl MImpl;

	MColorSwatch(const std::string &inID, MRect inBounds,
	             MColor inColor);

	virtual MColor GetColor() const;
	virtual void SetColor(MColor inColor);

	MEventOut<void(const std::string &, MColor)> eColorChanged;
	MEventOut<void(const std::string &, MColor)> eColorPreview;
};

// --------------------------------------------------------------------

class MListBoxImpl;

class MListBox : public MControl<MListBoxImpl>
{
  public:
	typedef MListBoxImpl MImpl;

	MListBox(const std::string &inID, MRect inBounds);

	void AddItem(const std::string &inLabel);

	virtual int32_t GetValue() const;
	virtual void SetValue(int32_t inValue);

	MEventOut<void(const std::string &, int32_t)> eValueChanged;
};

// // --------------------------------------------------------------------

// class MListViewImpl;

// class MListView : public MControl<MListViewImpl>
// {
//   public:
// 	typedef MListViewImpl MImpl;

// 	MListView(const std::string &inID, MRect inBounds);

// 	void AddItem(const std::string &inLabel);

// 	MEventOut<void(const std::string &, int32_t)> eValueChanged;
// };

// --------------------------------------------------------------------
// Gtk specific controls

class MBoxControlImpl;

class MBoxControl : public MControl<MBoxControlImpl>
{
  public:
	typedef MBoxControlImpl MImpl;

	MBoxControl(const std::string &inID, MRect inBounds, bool inHorizontal,
	            bool inHomogeneous = false, bool inExpand = false, bool inFill = false,
	            uint32_t inSpacing = 0, uint32_t inPadding = 0);
};