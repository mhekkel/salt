//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include <boost/algorithm/string.hpp>

#include "MXcbWindowImpl.hpp"
#include "MX11ControlsImpl.hpp"
#include "MX11CanvasImpl.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MColorPicker.hpp"

using namespace std;
namespace ba = boost::algorithm;

// --------------------------------------------------------------------

namespace MThemes
{



	
enum MThemeAlignment
{
	kThemeAlignNone, kThemeAlignLeft, kThemeAlignCenter, kThemeAlignRight
};

struct MTheme
{
	MColor foreColor, backColor;
	uint32_t rounded;
	string fontFamily;
	uint32_t fontStyle;
	MColor fontColor;
	MThemeAlignment textAlignment;
};

enum MThemeSelector {
	kThemeButton,
	
	kThemeSelectorCount
};

enum MThemeState {
	kThemeActive,
	kThemeInactive,
	kThemeHighlighted,
	
	kThemeStateCount
};

class MThemeStore
{
  public:
	static MThemeStore& Instance()		{ return sInstance; }

	MTheme GetTheme(MThemeSelector inTheme, MThemeState inState)
	{
		return mThemes[inTheme][inState];
	}

  private:
	MThemeStore();
	~MThemeStore();

	MTheme**	mThemes;
	
	static MThemeStore sInstance;
};

MThemeStore MThemeStore::sInstance;

inline MTheme GetTheme(MThemeSelector inTheme, MThemeState inState)
{
	return MThemeStore::Instance().GetTheme(inTheme, inState);
}

MThemeStore::MThemeStore()
{
//	static MTheme[kThemeSelectorCount][kThemeStateCount] =
//	{
//		{
//			{ kWhite, MColor("#888"),  }
//		}
//	};
	
}

MThemeStore::~MThemeStore()
{
}

}

// --------------------------------------------------------------------

const int kScrollbarWidth = 16;//::GetThemeSysSize(nullptr, SM_CXVSCROLL);

// --------------------------------------------------------------------

MXcbBasicControl::MXcbBasicControl(MView* inControl, const std::string& inLabel)
	: MXcbCairoSurface(inControl)
	, mLabel(inLabel)
{
	for (bool& b : mButtonPressed)
		b = false;
}

bool MXcbBasicControl::IsActive() const
{
	return mView->IsActive();
}

bool MXcbBasicControl::IsEnabled() const
{
	return mView->IsEnabled();
}

void MXcbBasicControl::ActivateSelf()
{
	Invalidate();
}

void MXcbBasicControl::DeactivateSelf()
{
	Invalidate();
}

void MXcbBasicControl::EnableSelf()
{
	Invalidate();
}

void MXcbBasicControl::DisableSelf()
{
	Invalidate();
}

void MXcbBasicControl::ShowSelf()
{
	Show();
	Invalidate();
}

void MXcbBasicControl::HideSelf()
{
	Hide();
}

string MXcbBasicControl::GetText() const
{
	return mLabel;
}

void MXcbBasicControl::SetText(const string& inText)
{
	mLabel = inText;
	
	Invalidate();
}

void MXcbBasicControl::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	if (inEvent->detail >= 1 and inEvent->detail <= 5)
		mButtonPressed[Button1 + inEvent->detail - 1] = true;
	
	MXcbCairoSurface::ButtonPressEvent(inEvent);
}

void MXcbBasicControl::ButtonReleaseEvent(xcb_key_release_event_t* inEvent)
{
	if (inEvent->detail >= 1 and
		inEvent->detail <= 5 and
		mButtonPressed[Button1 + inEvent->detail - 1])
	{
		if (mMouseIsIn)
			Clicked();
		
		mButtonPressed[Button1 + inEvent->detail - 1] = false;
	}

	MXcbCairoSurface::ButtonReleaseEvent(inEvent);
}

void MXcbBasicControl::KeyPressEvent(xcb_key_press_event_t* inEvent)
{
	
}

// --------------------------------------------------------------------

MX11SimpleControlImpl::MX11SimpleControlImpl(MSimpleControl* inControl)
	: MX11ControlImpl(inControl, "")
{
}

void MX11SimpleControlImpl::CreateWidget()
{
//	SetWidget(gtk_hbox_new(false, 0));
}

MSimpleControlImpl* MSimpleControlImpl::Create(MSimpleControl* inControl)
{
	return new MX11SimpleControlImpl(inControl);
}

// --------------------------------------------------------------------

MX11ButtonImpl::MX11ButtonImpl(MButton* inButton, const string& inLabel,
		MButtonFlags inFlags)
	: MX11ControlImpl(inButton, inLabel)
	, mFlags(inFlags)
	, mDefault(false)
{
}

void MX11ButtonImpl::Clicked()
{
	mControl->eClicked(mControl->GetID());
}

void MX11ButtonImpl::SimulateClick()
{
//	::SendMessage(GetWidget(), BM_SETSTATE, 1, 0);
//	::UpdateWindow(GetWidget());
//	::delay(12.0 / 60.0);
//	::SendMessage(GetWidget(), BM_SETSTATE, 0, 0);
//	::UpdateWindow(GetWidget());
//
//	mControl->eClicked(mControl->GetID());
}

void MX11ButtonImpl::MakeDefault(bool inDefault)
{
	mDefault = inDefault;
	
//	if (GetWidget() != nullptr)
//		gtk_widget_grab_default(GetWidget());
}

void MX11ButtonImpl::SetText(const std::string& inText)
{
	mLabel = inText;
//	gtk_button_set_label(GTK_BUTTON(GetWidget()), mLabel.c_str());
//	wstring text(c2w(inText));
//	::SendMessage(GetWidget(), WM_SETTEXT, 0, (LPARAM)text.c_str());
}

void MX11ButtonImpl::GetIdealSize(int32_t& outWidth, int32_t& outHeight)
{
//	outWidth = 75;
//	outHeight = 23;
//
//	SIZE size;
//	if (GetWidget() != nullptr and Button_GetIdealSize(GetWidget(), &size))
//	{
//		if (outWidth < size.cx + 20)
//			outWidth = size.cx + 20;
//
//		if (outHeight < size.cy + 2)
//			outHeight = size.cy + 2;
//	}
}

void MX11ButtonImpl::DrawWidget()
{
	MRect bounds;
	mControl->GetBounds(bounds);

	bounds.InsetBy(1, 1);
	
//	MThemeState state = 
	
	

	MColor color;
	if (mControl->IsActive() == false)
		color = MColor("#888");
	else if (mMouseIsIn)
	{
		if (mButtonPressed[Button1])
			color = MColor("#f40");
		else
			color = MColor("#fa4");
	}
	else
		color = MColor("#2B71DF");

	SetColor(color);
	FillRoundedRectangle(bounds, 5);
	
	SetColor(kWhite);
	SetFont("Sans", 12, fsNormal, fwBold);
	DrawString(bounds, "Click me!");
}

MButtonImpl* MButtonImpl::Create(MButton* inButton, const string& inLabel,
	MButtonFlags inFlags)
{
	return new MX11ButtonImpl(inButton, inLabel, inFlags);
}

// --------------------------------------------------------------------

//MX11ExpanderImpl::MX11ExpanderImpl(MExpander* inExpander, const string& inLabel)
//	: MX11ControlImpl(inExpander, inLabel)
//	, mIsOpen(false)
//	, mMouseInside(false)
//	, mMouseDown(false)
//	, mMouseTracking(false)
//	, mLastExit(0)
//{
////	AddHandler(WM_PAINT,			boost::bind(&MX11ExpanderImpl::WMPaint, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_ACTIVATE,			boost::bind(&MX11ExpanderImpl::WMActivate, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_LBUTTONDOWN,		boost::bind(&MX11ExpanderImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_LBUTTONDBLCLK,	boost::bind(&MX11ExpanderImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_LBUTTONUP,		boost::bind(&MX11ExpanderImpl::WMMouseUp, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_MOUSEMOVE,		boost::bind(&MX11ExpanderImpl::WMMouseMove, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_MOUSELEAVE,		boost::bind(&MX11ExpanderImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_CAPTURECHANGED,	boost::bind(&MX11ExpanderImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
//}
//
//MX11ExpanderImpl::~MX11ExpanderImpl()
//{
////	if (mDC)
////		::ReleaseDC(GetWidget(), mDC);
//}
//
//void MX11ExpanderImpl::CreateWidget()
//{
//	SetWidget(gtk_expander_new(mLabel.c_str()));
//}
//
//void MX11ExpanderImpl::Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//		bool inExpand, bool inFill, uint32_t inPadding)
//{
//	assert(GTK_IS_CONTAINER(GetWidget()));
//	gtk_container_add(GTK_CONTAINER(GetWidget()), inChild->GetWidget());
//}
//
//void MX11ExpanderImpl::SetOpen(bool inOpen)
//{
//	if (inOpen != mIsOpen)
//	{
//		mIsOpen = inOpen;
//		mControl->Invalidate();
//		mControl->eClicked(mControl->GetID());
//	}
//}
//
//bool MX11ExpanderImpl::IsOpen() const
//{
//	return mIsOpen;
//}
//
////bool MX11ExpanderImpl::WMActivate(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
////{
////	mControl->Invalidate();
////	return false;
////}
////
////bool MX11ExpanderImpl::WMPaint(HWND inHWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	bool result = false;
////
////	HTHEME hTheme = ::OpenThemeData(inHWnd, VSCLASS_TASKDIALOG);
////	if (hTheme != nullptr)
////	{
////		PAINTSTRUCT lPs;
////		HDC hdc = ::BeginPaint(inHWnd, &lPs);
////		THROW_IF_NIL(hdc);
////		
//////		::GetUpdateRect(inHWnd, &lPs.rcPaint, false);
////		
////		if (::IsThemeBackgroundPartiallyTransparent(hTheme, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL))
////			::DrawThemeParentBackground(inHWnd, hdc, &lPs.rcPaint);
////
////		RECT clientRect;
////		::GetClientRect(inHWnd, &clientRect);
////
////		RECT contentRect;
////		::GetThemeBackgroundContentRect(hTheme, hdc, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, &clientRect, &contentRect);
////		
////		int w = contentRect.bottom - contentRect.top;
////		::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_WIDTH, &w);
////		int sr = contentRect.right;
////		contentRect.right = contentRect.left + w;
////		
////		int state = TDLGEBS_NORMAL;
////		if ((mMouseInside and not mMouseDown) or (not mMouseInside and mMouseDown))
////			state = TDLGEBS_HOVER;
////		else if (mMouseDown)
////			state = TDLGEBS_PRESSED;
////
////		if (mIsOpen)
////			state += 3;		// expanded
////
////		::DrawThemeBackground(hTheme, hdc, TDLG_EXPANDOBUTTON, state, &contentRect, 0);
////
////	    ::CloseThemeData(hTheme);
////	    hTheme = ::OpenThemeData(inHWnd, VSCLASS_TEXTSTYLE);
////	    if (hTheme != nullptr)
////	    {
////			contentRect.left = contentRect.right;
////			contentRect.right = sr;
////			
////			wstring label(c2w(mLabel));
////	
////			RECT r;
////			::GetThemeTextExtent(hTheme, mDC, TEXT_BODYTEXT, 0, label.c_str(), label.length(), 0, nullptr, &r);
////			contentRect.left += (r.bottom - r.top) / 2;
////			contentRect.top += ((contentRect.bottom - contentRect.top) - (r.bottom - r.top)) / 2;
////
////			int state = TS_CONTROLLABEL_NORMAL;
////			if (not ::IsWindowEnabled(inHWnd))
////				state = TS_CONTROLLABEL_DISABLED;
////			
////			::DrawThemeText(hTheme, hdc, TEXT_BODYTEXT, state, label.c_str(), label.length(),
////				0, 0, &contentRect);
////
////			::CloseThemeData(hTheme);
////	    }
////
////		::EndPaint (inHWnd, &lPs);
////
////	    result = true;
////	}
////
////	return result;
////}
////
////bool MX11ExpanderImpl::WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	::SetFocus(inHWnd);
////	::SetCapture(inHWnd);
////
////	mMouseInside = true;
////	mMouseDown = true;
////	mControl->Invalidate();
////	mControl->UpdateNow();
////
////	return true;
////}
////
////bool MX11ExpanderImpl::WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	if (not mMouseTracking)
////	{
////		TRACKMOUSEEVENT me = { sizeof(TRACKMOUSEEVENT) };
////		me.dwFlags = TME_LEAVE;
////		me.hwndTrack = GetWidget();
////
////		if (not mMouseDown)
////			me.dwHoverTime = HOVER_DEFAULT;
////
////		if (::TrackMouseEvent(&me))
////			mMouseTracking = true;
////
////		mControl->Invalidate();
////		mControl->UpdateNow();
////	}
////
////	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
////	int32_t y = static_cast<int16_t>(HIWORD(inLParam));
////	
////	MRect bounds;
////	mControl->GetBounds(bounds);
////	
////	if (mMouseInside != bounds.ContainsPoint(x, y))
////	{
////		mMouseInside = not mMouseInside;
////		mControl->Invalidate();
////		mControl->UpdateNow();
////	}
////
////	return true;
////}
////
////bool MX11ExpanderImpl::WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	mMouseInside = false;
////	mMouseTracking = false;
////	mLastExit = GetLocalTime();
////	
////	mControl->Invalidate();
////	mControl->UpdateNow();
////	
////	return true;
////}
////
////bool MX11ExpanderImpl::WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	::ReleaseCapture();
////	
////	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
////	int32_t y = static_cast<int16_t>(HIWORD(inLParam));
////	
////	MRect bounds;
////	mControl->GetBounds(bounds);
////	mMouseInside = bounds.ContainsPoint(x, y);
////
////	if (mMouseInside)
////	{
////		mIsOpen = not mIsOpen;
////		mControl->eClicked(mControl->GetID());
////	}
////	mMouseDown = false;
////
////	mControl->Invalidate();
////	mControl->UpdateNow();
////
////	return true;
////}
//
//void MX11ExpanderImpl::AddedToWindow()
//{
//	MX11ControlImpl::AddedToWindow();
//	
////	mDC = ::GetDC(GetWidget());
////	
////	HTHEME hTheme = ::OpenThemeData(GetWidget(), VSCLASS_TASKDIALOG);
////	if (hTheme != nullptr)
////	{
////		int w, h;
////		RECT r;
////
////		wstring label(c2w(mLabel));
////		
////		if (::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_WIDTH, &w) == S_OK and
////			::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_HEIGHT, &h) == S_OK and
////			::GetThemeTextExtent(hTheme, mDC, TDLG_EXPANDOTEXT, 0, label.c_str(), label.length(), 0, nullptr, &r) == S_OK)
////		{
////			MX11ProcMixin* parent;
////			MRect bounds;
////			
////			GetParentAndBounds(parent, bounds);
////
////			int lw = r.right - r.left + (r.bottom - r.top) / 2;
////			mControl->ResizeFrame(w + lw - bounds.width, h - bounds.height);
////		}
////	}
//}

MExpanderImpl* MExpanderImpl::Create(MExpander* inExpander, const string& inLabel)
{
//	return new MX11ExpanderImpl(inExpander, inLabel);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11ScrollbarImpl::MX11ScrollbarImpl(MScrollbar* inScrollbar)
//	: MX11ControlImpl(inScrollbar, "")
//	, eValueChanged(this, &MX11ScrollbarImpl::ValueChanged)
//{
//}
//
//void MX11ScrollbarImpl::CreateWidget()
//{
//	MRect bounds;
//	mControl->GetBounds(bounds);
//
//	GtkAdjustment* adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 1, 1, 1));
//
//	if (bounds.width > bounds.height)
//		SetWidget(gtk_hscrollbar_new(adjustment));
//	else
//		SetWidget(gtk_vscrollbar_new(adjustment));
//
//	eValueChanged.Connect(GetWidget(), "value-changed");
//}
//
//int32_t MX11ScrollbarImpl::GetValue() const
//{
//	int32_t result = 0;
//
//	if (GetWidget() != nullptr)
//		result = gtk_range_get_value(GTK_RANGE(GetWidget()));
//
//	return result;
//}
//
//void MX11ScrollbarImpl::SetValue(int32_t inValue)
//{
//	gtk_range_set_value(GTK_RANGE(GetWidget()), inValue);
//}
//
//int32_t MX11ScrollbarImpl::GetTrackValue() const
//{
//	return GetValue();
//}
//
//void MX11ScrollbarImpl::SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
//	int32_t inScrollUnit, int32_t inPageSize, int32_t inValue)
//{
//	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));
//	
//	if (adj != nullptr)
//	{
//		adj->lower = inMinValue;
//		adj->upper = inMaxValue + 1;
//		adj->step_increment = inScrollUnit;
//		adj->page_increment = inPageSize;
//		adj->page_size = inPageSize;
//		adj->value = inValue;
//		gtk_adjustment_changed(adj);
//	}
//}
//
//int32_t MX11ScrollbarImpl::GetMinValue() const
//{
//	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));
//	return adj == nullptr ? 0 : adj->lower;
//}
//
//int32_t MX11ScrollbarImpl::GetMaxValue() const
//{
//	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));
//
//	int32_t result = 0;
//	if (adj != nullptr)
//	{
//		result = adj->upper;
//		if (adj->page_size > 1)
//			result -= adj->page_size;
//	}
//
//	return result;
//}
//
//void MX11ScrollbarImpl::ValueChanged()
//{
//	mControl->eScroll(kScrollToThumb);
//}

MScrollbarImpl* MScrollbarImpl::Create(MScrollbar* inScrollbar)
{
//	return new MX11ScrollbarImpl(inScrollbar);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11StatusbarImpl::MX11StatusbarImpl(MStatusbar* inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
//	: MX11ControlImpl(inStatusbar, "")
//	, mParts(inParts, inParts + inPartCount)
//	, mClicked(this, &MX11StatusbarImpl::Clicked)
//{
//}
//
//void MX11StatusbarImpl::CreateWidget()
//{
//	GtkWidget* statusBar = gtk_statusbar_new();
//
//	GtkShadowType shadow_type = GTK_SHADOW_NONE;
////	gtk_widget_style_get(statusBar, "shadow_type", &shadow_type, nullptr);
//
//	for (auto part : mParts)
//	{
//		GtkWidget* frame = gtk_frame_new(nullptr);
//		gtk_frame_set_shadow_type(GTK_FRAME(frame), shadow_type);
//		
//		GtkWidget* label = gtk_label_new("");
//		gtk_label_set_single_line_mode(GTK_LABEL(label), true);
//		gtk_label_set_selectable(GTK_LABEL(label), true);
//		gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
//		
//		gtk_container_add(GTK_CONTAINER(frame), label);
//
//		if (part.packing == ePackStart)
//			gtk_box_pack_start(GTK_BOX(statusBar), frame, part.expand, part.fill, part.padding);
//		else
//			gtk_box_pack_end(GTK_BOX(statusBar), frame, part.expand, part.fill, part.padding);
//
//		if (part.width > 0)
//			gtk_widget_set_size_request(frame, part.width, -1);
//			
//		mPanels.push_back(label);
//		
//		mClicked.Connect(label, "button-press-event");
//	}
//	
//	SetWidget(statusBar);
//}
//
//void MX11StatusbarImpl::AddedToWindow()
//{
//	CreateWidget();
//	
//	MXcbWinMixin* parent;
//	MRect bounds;
//	
//	GetParentAndBounds(parent, bounds);
//	
//	MXcbWindowImpl* impl = dynamic_cast<MXcbWindowImpl*>(parent);
//	assert(impl != nullptr);
//	impl->AddStatusbarWidget(this);
//}
//
//void MX11StatusbarImpl::SetStatusText(uint32_t inPartNr, const string& inText, bool inBorder)
//{
//	if (inPartNr < mPanels.size())
//		gtk_label_set_text(GTK_LABEL(mPanels[inPartNr]), inText.c_str());
//}
//
//bool MX11StatusbarImpl::Clicked(GdkEventButton* inEvent)
//{
//	GtkWidget* source = GTK_WIDGET(mClicked.GetSourceGObject());
//	
//	auto panel = find(mPanels.begin(), mPanels.end(), source);
//	if (panel != mPanels.end())
//		mControl->ePartClicked(panel - mPanels.begin(), MRect());
//	
//	return true;
//}

MStatusbarImpl* MStatusbarImpl::Create(MStatusbar* inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
{
//	return new MX11StatusbarImpl(inStatusbar, inPartCount, inParts);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11ComboboxImpl::MX11ComboboxImpl(MCombobox* inCombobox)
//	: MX11ControlImpl(inCombobox, "")
//{
//}
//
//void MX11ComboboxImpl::CreateWidget()
//{
//	GtkTreeModel* list_store = GTK_TREE_MODEL(gtk_list_store_new(1, G_TYPE_STRING));
//	THROW_IF_NIL(list_store);
//	
//	GtkWidget* wdgt = gtk_combo_box_new_with_model_and_entry(list_store);
//	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(wdgt), 0);
//	
//	SetWidget(wdgt);
//}
//
//string MX11ComboboxImpl::GetText() const
//{
//	string result;
//	
//	char* text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(GetWidget()));
//	if (text != nullptr)
//	{
//		result = text;
//		g_free(text);
//	}
//
//	return result;
//}
//
//void MX11ComboboxImpl::SetChoices(const std::vector<std::string>& inChoices)
//{
//	GtkWidget* wdgt = GetWidget();
//	
//	if (wdgt == nullptr)
//		mChoices = inChoices;
//	else
//	{
//		if (not GTK_IS_COMBO_BOX(wdgt))
//			THROW(("Item is not a combo box"));
//
//		GtkListStore* model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(wdgt)));
//		THROW_IF_NIL(model);
//		
//		gtk_list_store_clear(model);
//	
//		for (auto s: inChoices)
//		{
//			GtkTreeIter iter;
//
//			gtk_list_store_append(model, &iter);
//			gtk_list_store_set(model, &iter,
//                          0, s.c_str(),
//                          -1);
//		}
//		
//		gtk_combo_box_set_active(GTK_COMBO_BOX(wdgt), 0);
//		
//		// connect signal
//		mChanged.Connect(wdgt, "changed");
//	}
//}
//
//void MX11ComboboxImpl::AddedToWindow()
//{
//	MX11ControlImpl::AddedToWindow();
//	
//	if (not mChoices.empty())
//		SetChoices(mChoices);
//}
//
//bool MX11ComboboxImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
//{
//	bool result = false;
//
//	if (inKeyCode == kReturnKeyCode or
//		inKeyCode == kEnterKeyCode or
//		inKeyCode == kTabKeyCode or
//		inKeyCode == kEscapeKeyCode or
//		(inModifiers & ~kShiftKey) != 0)
//	{
////		result = MX11ControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
//	}
//
//	return result;
//}
//
//void MX11ComboboxImpl::OnChanged()
//{
//	mControl->eValueChanged(mControl->GetID(), GetText());
//}

MComboboxImpl* MComboboxImpl::Create(MCombobox* inCombobox)
{
//	return new MX11ComboboxImpl(inCombobox);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11PopupImpl::MX11PopupImpl(MPopup* inPopup)
//	: MX11ControlImpl(inPopup, "")
//{
//}
//
//void MX11PopupImpl::CreateWidget()
//{
//	SetWidget(gtk_combo_box_text_new());
//}
//
//void MX11PopupImpl::SetChoices(const std::vector<std::string>& inChoices)
//{
//	mChoices = inChoices;
//
//	if (GetWidget() != nullptr)
//	{
//		for (auto s: inChoices)
//			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(GetWidget()), s.c_str());
//		
//		gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), 0);
//		
//		// connect signal
//		mChanged.Connect(GetWidget(), "changed");
//	}
//}
//
//void MX11PopupImpl::AddedToWindow()
//{
//	MX11ControlImpl::AddedToWindow();
//
//	if (not mChoices.empty())
//		SetChoices(mChoices);
//}
//
//int32_t MX11PopupImpl::GetValue() const
//{
//	return gtk_combo_box_get_active(GTK_COMBO_BOX(GetWidget()));
//}
//
//void MX11PopupImpl::SetValue(int32_t inValue)
//{
//	gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), inValue);
//}
//
//void MX11PopupImpl::SetText(const string& inText)
//{
//	auto i = find(mChoices.begin(), mChoices.end(), inText);
//	if (i != mChoices.end())
//		SetValue(i - mChoices.begin());
//}
//
//string MX11PopupImpl::GetText() const
//{
//	const char* s = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(GetWidget()));
//	return s ? s : "";
//}
//
//bool MX11PopupImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
//{
//	bool result = false;
//
//	if (inKeyCode == kReturnKeyCode or
//		inKeyCode == kEnterKeyCode or
//		inKeyCode == kTabKeyCode or
//		inKeyCode == kEscapeKeyCode or
//		(inModifiers & ~kShiftKey) != 0)
//	{
////		result = MX11ControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
//	}
//
//	return result;
//}

MPopupImpl* MPopupImpl::Create(MPopup* inPopup)
{
//	return new MX11PopupImpl(inPopup);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11EdittextImpl::MX11EdittextImpl(MEdittext* inEdittext, uint32_t inFlags)
//	: MX11ControlImpl(inEdittext, "")
//	, mFlags(inFlags)
//{
//}
//
//void MX11EdittextImpl::CreateWidget()
//{
//	SetWidget(gtk_entry_new());
//}
//
//void MX11EdittextImpl::SetFocus()
//{
//	MX11ControlImpl::SetFocus();
////	
////	::SendMessage(GetWidget(), EM_SETSEL, 0, -1);
//}
//
//string MX11EdittextImpl::GetText() const
//{
//	const char* result = nullptr;
//	if (GTK_IS_ENTRY(GetWidget()))
//		result = gtk_entry_get_text(GTK_ENTRY(GetWidget()));
//	return result ? result : "";
//}
//
//void MX11EdittextImpl::SetText(const std::string& inText)
//{
//	if (GTK_IS_ENTRY(GetWidget()))
//		gtk_entry_set_text(GTK_ENTRY(GetWidget()), inText.c_str());
//}
//
//void MX11EdittextImpl::SetPasswordChar(uint32_t inUnicode)
//{
//	GtkWidget* wdgt = GetWidget();
//	if (GTK_IS_ENTRY(wdgt))
//	{
//		gtk_entry_set_visibility(GTK_ENTRY(wdgt), false);
//		gtk_entry_set_invisible_char(GTK_ENTRY(wdgt), inUnicode);
//	}
//	else
//		THROW(("item is not an entry"));
//}
//
//bool MX11EdittextImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
//{
//	bool result = false;
//
//	if (inKeyCode == kReturnKeyCode or
//		inKeyCode == kEnterKeyCode or
//		inKeyCode == kTabKeyCode or
//		inKeyCode == kEscapeKeyCode or
//		(inModifiers & ~kShiftKey) != 0)
//	{
////		result = MX11ControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
//	}
//
//	return result;
//}

MEdittextImpl* MEdittextImpl::Create(MEdittext* inEdittext, uint32_t inFlags)
{
//	return new MX11EdittextImpl(inEdittext, inFlags);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11CaptionImpl::MX11CaptionImpl(MCaption* inControl, const string& inText)
//	: MX11ControlImpl(inControl, inText)
//{
//}
//
//void MX11CaptionImpl::CreateWidget()
//{
//	GtkWidget* widget = gtk_label_new(mLabel.c_str());
////	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
//	float x, y;
//	gtk_misc_get_alignment(GTK_MISC(widget), &x, &y);
//	x = 0;
//	gtk_misc_set_alignment(GTK_MISC(widget), x, y);
//	SetWidget(widget);
//}
//
//void MX11CaptionImpl::SetText(const string& inText)
//{
//	mLabel = inText;
//	if (GTK_IS_LABEL(GetWidget()))
//		gtk_label_set_text(GTK_LABEL(GetWidget()), inText.c_str());
//}

MCaptionImpl* MCaptionImpl::Create(MCaption* inCaption, const std::string& inText)
{
//	return new MX11CaptionImpl(inCaption, inText);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11SeparatorImpl::MX11SeparatorImpl(MSeparator* inControl)
//	: MX11ControlImpl(inControl, "")
//{
//}
//
//void MX11SeparatorImpl::CreateWidget()
//{
//	SetWidget(gtk_hseparator_new());	
//}

MSeparatorImpl* MSeparatorImpl::Create(MSeparator* inSeparator)
{
//	return new MX11SeparatorImpl(inSeparator);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11CheckboxImpl::MX11CheckboxImpl(MCheckbox* inControl, const string& inText)
//	: MX11ControlImpl(inControl, inText)
//	, mChecked(false)
//{
//}
//
//void MX11CheckboxImpl::CreateWidget()
//{
//	SetWidget(gtk_check_button_new_with_label(mLabel.c_str()));
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), mChecked);
//}
//
//bool MX11CheckboxImpl::IsChecked() const
//{
//	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GetWidget()));
//}
//
//void MX11CheckboxImpl::SetChecked(bool inChecked)
//{
//	mChecked = inChecked;
//	if (GetWidget())
//		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), mChecked);
//}

MCheckboxImpl* MCheckboxImpl::Create(MCheckbox* inCheckbox, const string& inText)
{
//	return new MX11CheckboxImpl(inCheckbox, inText);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11RadiobuttonImpl::MX11RadiobuttonImpl(MRadiobutton* inControl, const string& inText)
//	: MX11ControlImpl(inControl, inText)
//{
//}
//
//void MX11RadiobuttonImpl::CreateWidget()
//{
//	if (mGroup.empty() or mGroup.front() == mControl)
//		SetWidget(gtk_radio_button_new_with_label(nullptr, mLabel.c_str()));
//	else if (not mGroup.empty())
//	{
//		MX11RadiobuttonImpl* first = dynamic_cast<MX11RadiobuttonImpl*>(mGroup.front()->GetImpl());
//		
//		SetWidget(gtk_radio_button_new_with_label_from_widget(
//			GTK_RADIO_BUTTON(first->GetWidget()), mLabel.c_str()));
//	}
//}
//
//bool MX11RadiobuttonImpl::IsChecked() const
//{
//	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GetWidget()));
//}
//
//void MX11RadiobuttonImpl::SetChecked(bool inChecked)
//{
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), inChecked);
//}
//
//void MX11RadiobuttonImpl::SetGroup(const list<MRadiobutton*>& inButtons)
//{
//	mGroup = inButtons;
//}
//
////bool MX11RadiobuttonImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	bool result = false;
////
////	if (inUMsg == BN_CLICKED)
////	{
////		bool checked = not IsChecked();
////
////		SetChecked(checked);
////		mControl->eValueChanged(mControl->GetID(), checked);
////
////		outResult = 1;
////		result = true;
////	}
////
////	return result;
////}
////
MRadiobuttonImpl* MRadiobuttonImpl::Create(MRadiobutton* inRadiobutton, const std::string& inText)
{
//	return new MX11RadiobuttonImpl(inRadiobutton, inText);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11ListHeaderImpl::MX11ListHeaderImpl(MListHeader* inListHeader)
//	: MX11ControlImpl(inListHeader, "")
//{
//}
//
//void MX11ListHeaderImpl::CreateWidget()
//{
//	assert(false);
//}
//
//void MX11ListHeaderImpl::AppendColumn(const string& inLabel, int inWidth)
//{
////	HDITEM item = {};
////	
////	wstring label = c2w(inLabel);
////	
////	item.mask = HDI_FORMAT | HDI_TEXT;
////	item.pszText = const_cast<wchar_t*>(label.c_str());
////	item.cchTextMax = label.length();
////	item.fmt = HDF_LEFT | HDF_STRING;
////
////	if (inWidth > 0)
////	{
////		item.mask |= HDI_WIDTH;
////		item.cxy = inWidth;
////	}
////	
////	int insertAfter = ::SendMessage(GetWidget(), HDM_GETITEMCOUNT, 0, 0);
////	::SendMessage(GetWidget(), HDM_INSERTITEM, (WPARAM)&insertAfter, (LPARAM)&item);
//}

MListHeaderImpl* MListHeaderImpl::Create(MListHeader* inListHeader)
{
//	return new MX11ListHeaderImpl(inListHeader);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11NotebookImpl::MX11NotebookImpl(MNotebook* inControl)
//	: MX11ControlImpl(inControl, "")
//{
//}
//
//void MX11NotebookImpl::CreateWidget()
//{
//	assert(false);
//}
//
//void MX11NotebookImpl::AddedToWindow()
//{
//	MX11ControlImpl::AddedToWindow();
//
//	// add pages
//	if (not mPages.empty())
//	{
//		vector<MPage> pages(mPages);
//		mPages.clear();
//		
//		for (MPage& page: pages)
//			AddPage(page.mTitle, page.mPage);
//		
//		FrameResized();
//	}
//
////	MRect bounds;
////	MX11ProcMixin* parent;
////
////	GetParentAndBounds(parent, bounds);
////	
////	parent->AddNotify(TCN_SELCHANGE, GetWidget(),
////		boost::bind(&MX11NotebookImpl::TCNSelChange, this, _1, _2, _3));
//}
//
//void MX11NotebookImpl::FrameResized()
//{
//	MX11ControlImpl<MNotebook>::FrameResized();
//
////	RECT rc;
////	::GetClientRect(GetWidget(), &rc);
////	TabCtrl_AdjustRect(GetWidget(), false, &rc);
////
////	for (MPage& page: mPages)
////	{
////		MRect frame;
////		page.mPage->GetFrame(frame);
////
////		int32_t dx, dy, dw, dh;
////		dx = rc.left - frame.x;
////		dy = rc.top - frame.y;
////		dw = rc.right - rc.left - frame.width;
////		dh = rc.bottom - rc.top - frame.height;
////
////		page.mPage->MoveFrame(dx, dy);
////		page.mPage->ResizeFrame(dw, dh);
////	}
//}
//
//void MX11NotebookImpl::AddPage(const string& inLabel, MView* inPage)
//{
//	MPage page = { inLabel, inPage };
//	mPages.push_back(page);
//	
////	if (GetWidget())
////	{
////		wstring s(c2w(inLabel));
////		
////		TCITEM tci = {};
////		tci.mask = TCIF_TEXT;
////		tci.pszText = const_cast<wchar_t*>(s.c_str());
////		
////		::SendMessage(GetWidget(), TCM_INSERTITEM, (WPARAM)mPages.size(), (LPARAM)&tci);
////
////		RECT r;
////		::GetClientRect(GetWidget(), &r);
////		TabCtrl_AdjustRect(GetWidget(), false, &r);
////
////		MRect frame;
////		inPage->GetFrame(frame);
////
////		if (frame.x != r.left or frame.y != r.top)
////			inPage->MoveFrame(r.left - frame.x, r.top - frame.y);
////
////		if (frame.height != r.bottom - r.top or
////			frame.width != r.right - r.left)
////		{
////			inPage->ResizeFrame((r.right - r.left) - frame.width, (r.bottom - r.top) - frame.height);
////		}
////
////		mControl->AddChild(inPage);
////		
////		if (mPages.size() > 1)
////			inPage->Hide();
////	}
//}
//
//void MX11NotebookImpl::SelectPage(uint32_t inPage)
//{
////	if (inPage != ::SendMessage(GetWidget(), TCM_GETCURSEL, 0, 0))
////	{
////		::SendMessage(GetWidget(), TCM_SETCURSEL, (WPARAM)inPage, 0);
////		::UpdateWindow(GetWidget());
////	}
////	
////	if (inPage < mPages.size())
////	{
////		for (MPage& page: mPages)
////		{
////			page.mPage->Hide();
////			page.mPage->Disable();
////		}
////		mPages[inPage].mPage->Show();
////		mPages[inPage].mPage->Enable();
////		mControl->ePageSelected(inPage);
////	}
//}
//
//uint32_t MX11NotebookImpl::GetSelectedPage() const
//{
////	return ::SendMessage(GetWidget(), TCM_GETCURSEL, 0, 0);
//	return 0;
//}

MNotebookImpl* MNotebookImpl::Create(MNotebook* inNotebook)
{
//	return new MX11NotebookImpl(inNotebook);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11ColorSwatchImpl::MX11ColorSwatchImpl(MColorSwatch* inColorSwatch, MColor inColor)
//	: MX11ControlImpl(inColorSwatch, "")
//	, eSelectedColor(this, &MX11ColorSwatchImpl::SelectedColor)
//	, ePreviewColor(this, &MX11ColorSwatchImpl::PreviewColor)
//	, mColorSet(this, &MX11ColorSwatchImpl::OnColorSet)
//	, mColor(inColor)
//{
//}
//
//void MX11ColorSwatchImpl::CreateWidget()
//{
//	GdkColor color = {};
//	color.red = mColor.red << 8 | mColor.red;
//	color.green = mColor.green << 8 | mColor.green;
//	color.blue = mColor.blue << 8 | mColor.blue;
//	
//	SetWidget(gtk_color_button_new_with_color(&color));
//	
//	mColorSet.Connect(GetWidget(), "color-set");
//}
//
//void MX11ColorSwatchImpl::OnColorSet()
//{
//	GdkColor color = {};
//	gtk_color_button_get_color(GTK_COLOR_BUTTON(GetWidget()), &color);
//
//	mColor.red = color.red >> 8;
//	mColor.green = color.green >> 8;
//	mColor.blue = color.blue >> 8;
//}
//
//void MX11ColorSwatchImpl::SelectedColor(MColor inColor)
//{
//	SetColor(inColor);
//	mControl->eColorChanged(mControl->GetID(), mColor);
//}
//
//void MX11ColorSwatchImpl::PreviewColor(MColor inColor)
//{
//	mControl->eColorPreview(mControl->GetID(), inColor);
//}
//
//MColor MX11ColorSwatchImpl::GetColor() const
//{
//	return mColor;
//}
//
//void MX11ColorSwatchImpl::SetColor(MColor inColor)
//{
//	mColor = inColor;
//
//	GdkColor color = {};
//	color.red = mColor.red << 8 | mColor.red;
//	color.green = mColor.green << 8 | mColor.green;
//	color.blue = mColor.blue << 8 | mColor.blue;
//	if (GTK_IS_COLOR_BUTTON(GetWidget()))
//		gtk_color_button_set_color(GTK_COLOR_BUTTON(GetWidget()), &color);
//}
//
////void MX11ColorSwatchImpl::GetIdealSize(int32_t& outWidth, int32_t& outHeight)
////{
////	outWidth = 30;
////	outHeight = 23;
////
////	SIZE size;
////	if (GetWidget() != nullptr and ColorSwatch_GetIdealSize(GetWidget(), &size))
////	{
////		if (outWidth < size.cx + 20)
////			outWidth = size.cx + 20;
////
////		if (outHeight < size.cy + 2)
////			outHeight = size.cy + 2;
////	}
////}

MColorSwatchImpl* MColorSwatchImpl::Create(MColorSwatch* inColorSwatch, MColor inColor)
{
//	return new MX11ColorSwatchImpl(inColorSwatch, inColor);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11ListBoxImpl::MX11ListBoxImpl(MListBox* inListBox)
//	: MX11ControlImpl(inListBox, "")
//	, mSelectionChanged(this, &MX11ListBoxImpl::OnSelectionChanged)
//	, mStore(nullptr), mNr(0)
//{
//}
//
//void MX11ListBoxImpl::CreateWidget()
//{
//	mStore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
//	
//	SetWidget(gtk_tree_view_new_with_model(GTK_TREE_MODEL(mStore)));
//	
//	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(GetWidget()), false);
//	
//	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
//	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Author",
//                                                   renderer,
//                                                   "text", 0,
//                                                   NULL);
//	gtk_tree_view_append_column(GTK_TREE_VIEW(GetWidget()), column);
//	
//	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));
//	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
//	mSelectionChanged.Connect(G_OBJECT(selection), "changed");
//}
//
////void MX11ListBoxImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
////	wstring& outClassName, HMENU& outMenu)
////{
////	MX11ControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
////
////	outStyle = WS_CHILD | LBS_HASSTRINGS | LBS_NOTIFY;
////	outExStyle |= WS_EX_CLIENTEDGE;
////	
////	outClassName = WC_LISTBOX;
////}
//
//void MX11ListBoxImpl::AddedToWindow()
//{
//	MX11ControlImpl::AddedToWindow();
//	
//	for (string& item: mItems)
//		AddItem(item);
//	
//	mItems.clear();
//	
//	SetValue(0);
//}
//
//void MX11ListBoxImpl::AddItem(const string& inText)
//{
//	if (mStore == nullptr)
//		mItems.push_back(inText);
//	else
//	{
//		GtkTreeIter iter;
//
//		gtk_list_store_append(mStore, &iter);
//		gtk_list_store_set(mStore, &iter,
//			0, inText.c_str(), 1, mNr++, -1);
//	}
//}
//
//int32_t MX11ListBoxImpl::GetValue() const
//{
//	int32_t result = -1;
//	
//	GtkTreeIter iter;
//    GtkTreeModel *model;
//	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));
//
//    if (gtk_tree_selection_get_selected(selection, &model, &iter))
//		gtk_tree_model_get(GTK_TREE_MODEL(mStore), &iter, 1, &result, -1);
//
//	return result;
//}
//
//void MX11ListBoxImpl::SetValue(int32_t inValue)
//{
////	::SendMessage(GetWidget(), LB_SETCURSEL, inValue, 0);
//}
//
//void MX11ListBoxImpl::OnSelectionChanged()
//{
//	GtkTreeIter iter;
//    GtkTreeModel *model;
//
//	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));
//
//    if (gtk_tree_selection_get_selected(selection, &model, &iter))
//    {
//    	int32_t selected;
//		gtk_tree_model_get(GTK_TREE_MODEL(mStore), &iter, 1, &selected, -1);
//		mControl->eValueChanged(mControl->GetID(), selected);
//    }
//}
//
////bool MX11ListBoxImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	switch (inUMsg)
////	{
////		case LBN_SELCHANGE:
////		case LBN_SELCANCEL:
////			mControl->eValueChanged(mControl->GetID(), ::SendMessage(GetWidget(), LB_GETCURSEL, 0, 0));
////			break;
////	}
////	
////	return true;
////}
////
MListBoxImpl* MListBoxImpl::Create(MListBox* inListBox)
{
//	return new MX11ListBoxImpl(inListBox);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11ListViewImpl::MX11ListViewImpl(MListView* inListView)
//	: MX11ControlImpl(inListView, "")
//	, mStore(nullptr)
//{
//}
//
//void MX11ListViewImpl::CreateWidget()
//{
//	mStore = gtk_list_store_new(1, G_TYPE_STRING);
//	
//	for (string& s: mItems)
//	{
//		GtkTreeIter iter;
//
//		gtk_list_store_append(mStore, &iter);
//		gtk_list_store_set(mStore, &iter,
//                      0, s.c_str(),
//                      -1);
//	}
//	
//	SetWidget(gtk_tree_view_new_with_model(GTK_TREE_MODEL(mStore)));
//	
//	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
////	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Author",
////                                                   renderer,
////                                                   "text", 0,
////                                                   NULL);
//
//	GtkTreeViewColumn* column = gtk_tree_view_column_new();
//	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
//	gtk_tree_view_column_pack_start(column, renderer, true);
//	gtk_tree_view_append_column (GTK_TREE_VIEW(GetWidget()), column);
//}
//
////void MX11ListViewImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
////	wstring& outClassName, HMENU& outMenu)
////{
////	MX11ControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
////
////	outStyle = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
////	outExStyle |= WS_EX_STATICEDGE;
////	
////	outClassName = WC_LISTVIEW;
////}
////
////void MX11ListViewImpl::CreateHandle(MX11ProcMixin* inParent, MRect inBounds, const wstring& inTitle)
////{
////	MX11ControlImpl::CreateHandle(inParent, inBounds, inTitle);
////	
////	// add a single column
////    LVCOLUMN lvc = {};
////
////    lvc.mask = LVCF_FMT | LVCF_SUBITEM;
////	lvc.cx = inBounds.width - 10;
////	lvc.fmt = LVCFMT_LEFT;
////	::SendMessage(GetWidget(), LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
////
////	if (inParent != nullptr)
////	{
////		inParent->AddNotify(LVN_ITEMACTIVATE, GetWidget(),
////			boost::bind(&MX11ListViewImpl::LVMItemActivate, this, _1, _2, _3));
////		inParent->AddNotify(LVN_GETDISPINFO, GetWidget(),
////			boost::bind(&MX11ListViewImpl::LVMGetDispInfo, this, _1, _2, _3));
////	}
////}
////
//void MX11ListViewImpl::AddedToWindow()
//{
//	MX11ControlImpl::AddedToWindow();
//	
//	for (string& item: mItems)
//		AddItem(item);
//	
//	mItems.clear();
//}
//
//void MX11ListViewImpl::AddItem(const string& inText)
//{
//	if (GetWidget() != nullptr)
//		mItems.push_back(inText);
//	else
//	{
//		GtkTreeIter iter;
//
//		gtk_list_store_append(mStore, &iter);
//		gtk_list_store_set(mStore, &iter,
//                      0, inText.c_str(),
//                      -1);
//	}
//}
//
////bool MX11ListViewImpl::LVMItemActivate(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	NMITEMACTIVATE* nmItemActivate = reinterpret_cast<NMITEMACTIVATE*>(inLParam);
////	
////	mControl->eValueChanged(mControl->GetID(), nmItemActivate->iItem);
////	
////	return true;
////}
////
////bool MX11ListViewImpl::LVMGetDispInfo(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(inLParam);
////	
////	
////	return true;
////}
////
MListViewImpl* MListViewImpl::Create(MListView* inListView)
{
//	return new MX11ListViewImpl(inListView);
	return nullptr;
}

// --------------------------------------------------------------------

//MX11BoxControlImpl::MX11BoxControlImpl(MBoxControl* inControl, bool inHorizontal,
//		bool inHomogeneous, bool inExpand, bool inFill, uint32_t inSpacing, uint32_t inPadding)
//	: MX11ControlImpl(inControl, ""), mHorizontal(inHorizontal)
//	, mHomogeneous(inHomogeneous), mExpand(inExpand), mFill(inFill)
//	, mSpacing(inSpacing), mPadding(inPadding)
//{
//}
//
//void MX11BoxControlImpl::CreateWidget()
//{
//	SetWidget(mHorizontal ? gtk_hbox_new(mHomogeneous, mSpacing) : gtk_vbox_new(mHomogeneous, mSpacing));
//}
//
//void MX11BoxControlImpl::Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//		bool inExpand, bool inFill, uint32_t inPadding)
//{
//	assert(GTK_IS_BOX(GetWidget()));
//	
//	if (inPacking == ePackStart)
//		gtk_box_pack_start(GTK_BOX(GetWidget()), inChild->GetWidget(), inExpand, inFill, inPadding);
//	else
//		gtk_box_pack_end(GTK_BOX(GetWidget()), inChild->GetWidget(), inExpand, inFill, inPadding);
//}

MBoxControlImpl* MBoxControlImpl::Create(MBoxControl* inControl, bool inHorizontal,
		bool inHomogeneous, bool inExpand, bool inFill, uint32_t inSpacing, uint32_t inPadding)
{
//	return new MX11BoxControlImpl(inControl, inHorizontal, inHomogeneous, inExpand, inFill, inSpacing, inPadding);
	return nullptr;
}

