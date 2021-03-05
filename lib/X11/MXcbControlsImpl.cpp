//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MXcbLib.hpp"

#include <boost/algorithm/string.hpp>

#include "MXcbWindowImpl.hpp"
#include "MXcbControlsImpl.hpp"
#include "MXcbCanvasImpl.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MColorPicker.hpp"
#include "MGfxDevice.hpp"

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
	uint32 rounded;
	string fontFamily;
	uint32 fontStyle;
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

void MXcbBasicControl::ButtonReleaseEvent(xcb_button_press_event_t* inEvent)
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

MXcbSimpleControlImpl::MXcbSimpleControlImpl(MSimpleControl* inControl)
	: MXcbControlImpl(inControl, "")
{
}

MSimpleControlImpl* MSimpleControlImpl::Create(MSimpleControl* inControl)
{
	return new MXcbSimpleControlImpl(inControl);
}

// --------------------------------------------------------------------

MXcbButtonImpl::MXcbButtonImpl(MButton* inButton, const string& inLabel,
		MButtonFlags inFlags)
	: MXcbControlImpl(inButton, inLabel)
	, mFlags(inFlags)
	, mDefault(false)
{
}

void MXcbButtonImpl::Clicked()
{
	mControl->eClicked(mControl->GetID());
}

void MXcbButtonImpl::SimulateClick()
{
//	::SendMessage(GetWidget(), BM_SETSTATE, 1, 0);
//	::UpdateWindow(GetWidget());
//	::delay(12.0 / 60.0);
//	::SendMessage(GetWidget(), BM_SETSTATE, 0, 0);
//	::UpdateWindow(GetWidget());
//
//	mControl->eClicked(mControl->GetID());
}

void MXcbButtonImpl::MakeDefault(bool inDefault)
{
	mDefault = inDefault;
	
//	if (GetWidget() != nullptr)
//		gtk_widget_grab_default(GetWidget());
}

void MXcbButtonImpl::SetText(const std::string& inText)
{
	mLabel = inText;
}

void MXcbButtonImpl::GetIdealSize(int32& outWidth, int32& outHeight)
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

void MXcbButtonImpl::DrawWidget(MGfxDevice& dev)
{
	MRect bounds;
	mControl->GetBounds(bounds);

	bounds.InsetBy(1, 1);
	
	dev.SetThemeColor(kThemeColorButtonBackground,
		mControl->IsActive(), mBtnState == kBtnStatePressed and mMouseIsIn);
//	
//	if (mControl->IsActive() == false)
//		SetColor(MColor("#666"));
//	else if (mBtnState == kBtnStatePressed and mMouseIsIn)
//		SetColor(MColor("#fa4"));
//	else if (mMouseIsIn)
//		SetColor(MColor("#3f67a5"));
//	else
//		SetColor(MColor("#2B71DF"));

	dev.FillRoundedRect(bounds, 5);

	dev.SetThemeColor(kThemeColorButtonFrame,
		mControl->IsActive(), mBtnState == kBtnStatePressed and mMouseIsIn);
	dev.StrokeRoundedRect(bounds, 5);
	
	dev.SetThemeColor(kThemeColorButtonText,
		mControl->IsActive(), mBtnState == kBtnStatePressed and mMouseIsIn);
	dev.SetFont("Sans", 12, true);
	
	dev.ShowTextInRect(bounds, "Click me!");
}

void MXcbButtonImpl::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	MXcbControlImpl::ButtonPressEvent(inEvent);
	mBtnState = kBtnStatePressed;
	Invalidate();
}

void MXcbButtonImpl::ButtonReleaseEvent(xcb_button_press_event_t* inEvent)
{
	MXcbControlImpl::ButtonReleaseEvent(inEvent);
	mBtnState = kBtnStateNormal;
	if (mMouseIsIn)
		Clicked();
	Invalidate();
}

void MXcbButtonImpl::MotionNotifyEvent(xcb_motion_notify_event_t* inEvent)
{
	MXcbControlImpl::MotionNotifyEvent(inEvent);
	Invalidate();
}

void MXcbButtonImpl::EnterNotifyEvent(xcb_enter_notify_event_t* inEvent)
{
	PRINT(("Enter button"));
	MXcbControlImpl::EnterNotifyEvent(inEvent);
	Invalidate();
}

void MXcbButtonImpl::LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent)
{
	MXcbControlImpl::LeaveNotifyEvent(inEvent);
	Invalidate();
}

MButtonImpl* MButtonImpl::Create(MButton* inButton, const string& inLabel,
	MButtonFlags inFlags)
{
	return new MXcbButtonImpl(inButton, inLabel, inFlags);
}

// --------------------------------------------------------------------

//MXcbExpanderImpl::MXcbExpanderImpl(MExpander* inExpander, const string& inLabel)
//	: MXcbControlImpl(inExpander, inLabel)
//	, mIsOpen(false)
//	, mMouseInside(false)
//	, mMouseDown(false)
//	, mMouseTracking(false)
//	, mLastExit(0)
//{
////	AddHandler(WM_PAINT,			boost::bind(&MXcbExpanderImpl::WMPaint, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_ACTIVATE,			boost::bind(&MXcbExpanderImpl::WMActivate, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_LBUTTONDOWN,		boost::bind(&MXcbExpanderImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_LBUTTONDBLCLK,	boost::bind(&MXcbExpanderImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_LBUTTONUP,		boost::bind(&MXcbExpanderImpl::WMMouseUp, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_MOUSEMOVE,		boost::bind(&MXcbExpanderImpl::WMMouseMove, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_MOUSELEAVE,		boost::bind(&MXcbExpanderImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
////	AddHandler(WM_CAPTURECHANGED,	boost::bind(&MXcbExpanderImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
//}
//
//MXcbExpanderImpl::~MXcbExpanderImpl()
//{
////	if (mDC)
////		::ReleaseDC(GetWidget(), mDC);
//}
//
//void MXcbExpanderImpl::CreateWidget()
//{
//	SetWidget(gtk_expander_new(mLabel.c_str()));
//}
//
//void MXcbExpanderImpl::Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//		bool inExpand, bool inFill, uint32 inPadding)
//{
//	assert(GTK_IS_CONTAINER(GetWidget()));
//	gtk_container_add(GTK_CONTAINER(GetWidget()), inChild->GetWidget());
//}
//
//void MXcbExpanderImpl::SetOpen(bool inOpen)
//{
//	if (inOpen != mIsOpen)
//	{
//		mIsOpen = inOpen;
//		mControl->Invalidate();
//		mControl->eClicked(mControl->GetID());
//	}
//}
//
//bool MXcbExpanderImpl::IsOpen() const
//{
//	return mIsOpen;
//}
//
////bool MXcbExpanderImpl::WMActivate(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
////{
////	mControl->Invalidate();
////	return false;
////}
////
////bool MXcbExpanderImpl::WMPaint(HWND inHWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
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
////bool MXcbExpanderImpl::WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
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
////bool MXcbExpanderImpl::WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
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
////	int32 x = static_cast<int16>(LOWORD(inLParam));
////	int32 y = static_cast<int16>(HIWORD(inLParam));
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
////bool MXcbExpanderImpl::WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
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
////bool MXcbExpanderImpl::WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	::ReleaseCapture();
////	
////	int32 x = static_cast<int16>(LOWORD(inLParam));
////	int32 y = static_cast<int16>(HIWORD(inLParam));
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
//void MXcbExpanderImpl::AddedToWindow()
//{
//	MXcbControlImpl::AddedToWindow();
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
////			MXcbProcMixin* parent;
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
//	return new MXcbExpanderImpl(inExpander, inLabel);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbScrollbarImpl::MXcbScrollbarImpl(MScrollbar* inScrollbar)
//	: MXcbControlImpl(inScrollbar, "")
//	, eValueChanged(this, &MXcbScrollbarImpl::ValueChanged)
//{
//}
//
//void MXcbScrollbarImpl::CreateWidget()
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
//int32 MXcbScrollbarImpl::GetValue() const
//{
//	int32 result = 0;
//
//	if (GetWidget() != nullptr)
//		result = gtk_range_get_value(GTK_RANGE(GetWidget()));
//
//	return result;
//}
//
//void MXcbScrollbarImpl::SetValue(int32 inValue)
//{
//	gtk_range_set_value(GTK_RANGE(GetWidget()), inValue);
//}
//
//int32 MXcbScrollbarImpl::GetTrackValue() const
//{
//	return GetValue();
//}
//
//void MXcbScrollbarImpl::SetAdjustmentValues(int32 inMinValue, int32 inMaxValue,
//	int32 inScrollUnit, int32 inPageSize, int32 inValue)
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
//int32 MXcbScrollbarImpl::GetMinValue() const
//{
//	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));
//	return adj == nullptr ? 0 : adj->lower;
//}
//
//int32 MXcbScrollbarImpl::GetMaxValue() const
//{
//	GtkAdjustment* adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));
//
//	int32 result = 0;
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
//void MXcbScrollbarImpl::ValueChanged()
//{
//	mControl->eScroll(kScrollToThumb);
//}

MScrollbarImpl* MScrollbarImpl::Create(MScrollbar* inScrollbar)
{
//	return new MXcbScrollbarImpl(inScrollbar);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbStatusbarImpl::MXcbStatusbarImpl(MStatusbar* inStatusbar, uint32 inPartCount, MStatusBarElement inParts[])
//	: MXcbControlImpl(inStatusbar, "")
//	, mParts(inParts, inParts + inPartCount)
//	, mClicked(this, &MXcbStatusbarImpl::Clicked)
//{
//}
//
//void MXcbStatusbarImpl::CreateWidget()
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
//void MXcbStatusbarImpl::AddedToWindow()
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
//void MXcbStatusbarImpl::SetStatusText(uint32 inPartNr, const string& inText, bool inBorder)
//{
//	if (inPartNr < mPanels.size())
//		gtk_label_set_text(GTK_LABEL(mPanels[inPartNr]), inText.c_str());
//}
//
//bool MXcbStatusbarImpl::Clicked(GdkEventButton* inEvent)
//{
//	GtkWidget* source = GTK_WIDGET(mClicked.GetSourceGObject());
//	
//	auto panel = find(mPanels.begin(), mPanels.end(), source);
//	if (panel != mPanels.end())
//		mControl->ePartClicked(panel - mPanels.begin(), MRect());
//	
//	return true;
//}

MStatusbarImpl* MStatusbarImpl::Create(MStatusbar* inStatusbar, uint32 inPartCount, MStatusBarElement inParts[])
{
//	return new MXcbStatusbarImpl(inStatusbar, inPartCount, inParts);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbComboboxImpl::MXcbComboboxImpl(MCombobox* inCombobox)
//	: MXcbControlImpl(inCombobox, "")
//{
//}
//
//void MXcbComboboxImpl::CreateWidget()
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
//string MXcbComboboxImpl::GetText() const
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
//void MXcbComboboxImpl::SetChoices(const std::vector<std::string>& inChoices)
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
//void MXcbComboboxImpl::AddedToWindow()
//{
//	MXcbControlImpl::AddedToWindow();
//	
//	if (not mChoices.empty())
//		SetChoices(mChoices);
//}
//
//bool MXcbComboboxImpl::DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat)
//{
//	bool result = false;
//
//	if (inKeyCode == kReturnKeyCode or
//		inKeyCode == kEnterKeyCode or
//		inKeyCode == kTabKeyCode or
//		inKeyCode == kEscapeKeyCode or
//		(inModifiers & ~kShiftKey) != 0)
//	{
////		result = MXcbControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
//	}
//
//	return result;
//}
//
//void MXcbComboboxImpl::OnChanged()
//{
//	mControl->eValueChanged(mControl->GetID(), GetText());
//}

MComboboxImpl* MComboboxImpl::Create(MCombobox* inCombobox)
{
//	return new MXcbComboboxImpl(inCombobox);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbPopupImpl::MXcbPopupImpl(MPopup* inPopup)
//	: MXcbControlImpl(inPopup, "")
//{
//}
//
//void MXcbPopupImpl::CreateWidget()
//{
//	SetWidget(gtk_combo_box_text_new());
//}
//
//void MXcbPopupImpl::SetChoices(const std::vector<std::string>& inChoices)
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
//void MXcbPopupImpl::AddedToWindow()
//{
//	MXcbControlImpl::AddedToWindow();
//
//	if (not mChoices.empty())
//		SetChoices(mChoices);
//}
//
//int32 MXcbPopupImpl::GetValue() const
//{
//	return gtk_combo_box_get_active(GTK_COMBO_BOX(GetWidget()));
//}
//
//void MXcbPopupImpl::SetValue(int32 inValue)
//{
//	gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), inValue);
//}
//
//void MXcbPopupImpl::SetText(const string& inText)
//{
//	auto i = find(mChoices.begin(), mChoices.end(), inText);
//	if (i != mChoices.end())
//		SetValue(i - mChoices.begin());
//}
//
//string MXcbPopupImpl::GetText() const
//{
//	const char* s = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(GetWidget()));
//	return s ? s : "";
//}
//
//bool MXcbPopupImpl::DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat)
//{
//	bool result = false;
//
//	if (inKeyCode == kReturnKeyCode or
//		inKeyCode == kEnterKeyCode or
//		inKeyCode == kTabKeyCode or
//		inKeyCode == kEscapeKeyCode or
//		(inModifiers & ~kShiftKey) != 0)
//	{
////		result = MXcbControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
//	}
//
//	return result;
//}

MPopupImpl* MPopupImpl::Create(MPopup* inPopup)
{
//	return new MXcbPopupImpl(inPopup);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbEdittextImpl::MXcbEdittextImpl(MEdittext* inEdittext, uint32 inFlags)
//	: MXcbControlImpl(inEdittext, "")
//	, mFlags(inFlags)
//{
//}
//
//void MXcbEdittextImpl::CreateWidget()
//{
//	SetWidget(gtk_entry_new());
//}
//
//void MXcbEdittextImpl::SetFocus()
//{
//	MXcbControlImpl::SetFocus();
////	
////	::SendMessage(GetWidget(), EM_SETSEL, 0, -1);
//}
//
//string MXcbEdittextImpl::GetText() const
//{
//	const char* result = nullptr;
//	if (GTK_IS_ENTRY(GetWidget()))
//		result = gtk_entry_get_text(GTK_ENTRY(GetWidget()));
//	return result ? result : "";
//}
//
//void MXcbEdittextImpl::SetText(const std::string& inText)
//{
//	if (GTK_IS_ENTRY(GetWidget()))
//		gtk_entry_set_text(GTK_ENTRY(GetWidget()), inText.c_str());
//}
//
//void MXcbEdittextImpl::SetPasswordChar(uint32 inUnicode)
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
//bool MXcbEdittextImpl::DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat)
//{
//	bool result = false;
//
//	if (inKeyCode == kReturnKeyCode or
//		inKeyCode == kEnterKeyCode or
//		inKeyCode == kTabKeyCode or
//		inKeyCode == kEscapeKeyCode or
//		(inModifiers & ~kShiftKey) != 0)
//	{
////		result = MXcbControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
//	}
//
//	return result;
//}

MEdittextImpl* MEdittextImpl::Create(MEdittext* inEdittext, uint32 inFlags)
{
//	return new MXcbEdittextImpl(inEdittext, inFlags);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbCaptionImpl::MXcbCaptionImpl(MCaption* inControl, const string& inText)
//	: MXcbControlImpl(inControl, inText)
//{
//}
//
//void MXcbCaptionImpl::CreateWidget()
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
//void MXcbCaptionImpl::SetText(const string& inText)
//{
//	mLabel = inText;
//	if (GTK_IS_LABEL(GetWidget()))
//		gtk_label_set_text(GTK_LABEL(GetWidget()), inText.c_str());
//}

MCaptionImpl* MCaptionImpl::Create(MCaption* inCaption, const std::string& inText)
{
//	return new MXcbCaptionImpl(inCaption, inText);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbSeparatorImpl::MXcbSeparatorImpl(MSeparator* inControl)
//	: MXcbControlImpl(inControl, "")
//{
//}
//
//void MXcbSeparatorImpl::CreateWidget()
//{
//	SetWidget(gtk_hseparator_new());	
//}

MSeparatorImpl* MSeparatorImpl::Create(MSeparator* inSeparator)
{
//	return new MXcbSeparatorImpl(inSeparator);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbCheckboxImpl::MXcbCheckboxImpl(MCheckbox* inControl, const string& inText)
//	: MXcbControlImpl(inControl, inText)
//	, mChecked(false)
//{
//}
//
//void MXcbCheckboxImpl::CreateWidget()
//{
//	SetWidget(gtk_check_button_new_with_label(mLabel.c_str()));
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), mChecked);
//}
//
//bool MXcbCheckboxImpl::IsChecked() const
//{
//	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GetWidget()));
//}
//
//void MXcbCheckboxImpl::SetChecked(bool inChecked)
//{
//	mChecked = inChecked;
//	if (GetWidget())
//		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), mChecked);
//}

MCheckboxImpl* MCheckboxImpl::Create(MCheckbox* inCheckbox, const string& inText)
{
//	return new MXcbCheckboxImpl(inCheckbox, inText);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbRadiobuttonImpl::MXcbRadiobuttonImpl(MRadiobutton* inControl, const string& inText)
//	: MXcbControlImpl(inControl, inText)
//{
//}
//
//void MXcbRadiobuttonImpl::CreateWidget()
//{
//	if (mGroup.empty() or mGroup.front() == mControl)
//		SetWidget(gtk_radio_button_new_with_label(nullptr, mLabel.c_str()));
//	else if (not mGroup.empty())
//	{
//		MXcbRadiobuttonImpl* first = dynamic_cast<MXcbRadiobuttonImpl*>(mGroup.front()->GetImpl());
//		
//		SetWidget(gtk_radio_button_new_with_label_from_widget(
//			GTK_RADIO_BUTTON(first->GetWidget()), mLabel.c_str()));
//	}
//}
//
//bool MXcbRadiobuttonImpl::IsChecked() const
//{
//	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GetWidget()));
//}
//
//void MXcbRadiobuttonImpl::SetChecked(bool inChecked)
//{
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), inChecked);
//}
//
//void MXcbRadiobuttonImpl::SetGroup(const list<MRadiobutton*>& inButtons)
//{
//	mGroup = inButtons;
//}
//
////bool MXcbRadiobuttonImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
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
//	return new MXcbRadiobuttonImpl(inRadiobutton, inText);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbListHeaderImpl::MXcbListHeaderImpl(MListHeader* inListHeader)
//	: MXcbControlImpl(inListHeader, "")
//{
//}
//
//void MXcbListHeaderImpl::CreateWidget()
//{
//	assert(false);
//}
//
//void MXcbListHeaderImpl::AppendColumn(const string& inLabel, int inWidth)
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
//	return new MXcbListHeaderImpl(inListHeader);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbNotebookImpl::MXcbNotebookImpl(MNotebook* inControl)
//	: MXcbControlImpl(inControl, "")
//{
//}
//
//void MXcbNotebookImpl::CreateWidget()
//{
//	assert(false);
//}
//
//void MXcbNotebookImpl::AddedToWindow()
//{
//	MXcbControlImpl::AddedToWindow();
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
////	MXcbProcMixin* parent;
////
////	GetParentAndBounds(parent, bounds);
////	
////	parent->AddNotify(TCN_SELCHANGE, GetWidget(),
////		boost::bind(&MXcbNotebookImpl::TCNSelChange, this, _1, _2, _3));
//}
//
//void MXcbNotebookImpl::FrameResized()
//{
//	MXcbControlImpl<MNotebook>::FrameResized();
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
////		int32 dx, dy, dw, dh;
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
//void MXcbNotebookImpl::AddPage(const string& inLabel, MView* inPage)
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
//void MXcbNotebookImpl::SelectPage(uint32 inPage)
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
//uint32 MXcbNotebookImpl::GetSelectedPage() const
//{
////	return ::SendMessage(GetWidget(), TCM_GETCURSEL, 0, 0);
//	return 0;
//}

MNotebookImpl* MNotebookImpl::Create(MNotebook* inNotebook)
{
//	return new MXcbNotebookImpl(inNotebook);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbColorSwatchImpl::MXcbColorSwatchImpl(MColorSwatch* inColorSwatch, MColor inColor)
//	: MXcbControlImpl(inColorSwatch, "")
//	, eSelectedColor(this, &MXcbColorSwatchImpl::SelectedColor)
//	, ePreviewColor(this, &MXcbColorSwatchImpl::PreviewColor)
//	, mColorSet(this, &MXcbColorSwatchImpl::OnColorSet)
//	, mColor(inColor)
//{
//}
//
//void MXcbColorSwatchImpl::CreateWidget()
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
//void MXcbColorSwatchImpl::OnColorSet()
//{
//	GdkColor color = {};
//	gtk_color_button_get_color(GTK_COLOR_BUTTON(GetWidget()), &color);
//
//	mColor.red = color.red >> 8;
//	mColor.green = color.green >> 8;
//	mColor.blue = color.blue >> 8;
//}
//
//void MXcbColorSwatchImpl::SelectedColor(MColor inColor)
//{
//	SetColor(inColor);
//	mControl->eColorChanged(mControl->GetID(), mColor);
//}
//
//void MXcbColorSwatchImpl::PreviewColor(MColor inColor)
//{
//	mControl->eColorPreview(mControl->GetID(), inColor);
//}
//
//MColor MXcbColorSwatchImpl::GetColor() const
//{
//	return mColor;
//}
//
//void MXcbColorSwatchImpl::SetColor(MColor inColor)
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
////void MXcbColorSwatchImpl::GetIdealSize(int32& outWidth, int32& outHeight)
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
//	return new MXcbColorSwatchImpl(inColorSwatch, inColor);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbListBoxImpl::MXcbListBoxImpl(MListBox* inListBox)
//	: MXcbControlImpl(inListBox, "")
//	, mSelectionChanged(this, &MXcbListBoxImpl::OnSelectionChanged)
//	, mStore(nullptr), mNr(0)
//{
//}
//
//void MXcbListBoxImpl::CreateWidget()
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
////void MXcbListBoxImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
////	wstring& outClassName, HMENU& outMenu)
////{
////	MXcbControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
////
////	outStyle = WS_CHILD | LBS_HASSTRINGS | LBS_NOTIFY;
////	outExStyle |= WS_EX_CLIENTEDGE;
////	
////	outClassName = WC_LISTBOX;
////}
//
//void MXcbListBoxImpl::AddedToWindow()
//{
//	MXcbControlImpl::AddedToWindow();
//	
//	for (string& item: mItems)
//		AddItem(item);
//	
//	mItems.clear();
//	
//	SetValue(0);
//}
//
//void MXcbListBoxImpl::AddItem(const string& inText)
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
//int32 MXcbListBoxImpl::GetValue() const
//{
//	int32 result = -1;
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
//void MXcbListBoxImpl::SetValue(int32 inValue)
//{
////	::SendMessage(GetWidget(), LB_SETCURSEL, inValue, 0);
//}
//
//void MXcbListBoxImpl::OnSelectionChanged()
//{
//	GtkTreeIter iter;
//    GtkTreeModel *model;
//
//	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));
//
//    if (gtk_tree_selection_get_selected(selection, &model, &iter))
//    {
//    	int32 selected;
//		gtk_tree_model_get(GTK_TREE_MODEL(mStore), &iter, 1, &selected, -1);
//		mControl->eValueChanged(mControl->GetID(), selected);
//    }
//}
//
////bool MXcbListBoxImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
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
//	return new MXcbListBoxImpl(inListBox);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbListViewImpl::MXcbListViewImpl(MListView* inListView)
//	: MXcbControlImpl(inListView, "")
//	, mStore(nullptr)
//{
//}
//
//void MXcbListViewImpl::CreateWidget()
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
////void MXcbListViewImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
////	wstring& outClassName, HMENU& outMenu)
////{
////	MXcbControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
////
////	outStyle = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
////	outExStyle |= WS_EX_STATICEDGE;
////	
////	outClassName = WC_LISTVIEW;
////}
////
////void MXcbListViewImpl::CreateHandle(MXcbProcMixin* inParent, MRect inBounds, const wstring& inTitle)
////{
////	MXcbControlImpl::CreateHandle(inParent, inBounds, inTitle);
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
////			boost::bind(&MXcbListViewImpl::LVMItemActivate, this, _1, _2, _3));
////		inParent->AddNotify(LVN_GETDISPINFO, GetWidget(),
////			boost::bind(&MXcbListViewImpl::LVMGetDispInfo, this, _1, _2, _3));
////	}
////}
////
//void MXcbListViewImpl::AddedToWindow()
//{
//	MXcbControlImpl::AddedToWindow();
//	
//	for (string& item: mItems)
//		AddItem(item);
//	
//	mItems.clear();
//}
//
//void MXcbListViewImpl::AddItem(const string& inText)
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
////bool MXcbListViewImpl::LVMItemActivate(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	NMITEMACTIVATE* nmItemActivate = reinterpret_cast<NMITEMACTIVATE*>(inLParam);
////	
////	mControl->eValueChanged(mControl->GetID(), nmItemActivate->iItem);
////	
////	return true;
////}
////
////bool MXcbListViewImpl::LVMGetDispInfo(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
////{
////	NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(inLParam);
////	
////	
////	return true;
////}
////
MListViewImpl* MListViewImpl::Create(MListView* inListView)
{
//	return new MXcbListViewImpl(inListView);
	return nullptr;
}

// --------------------------------------------------------------------

//MXcbBoxControlImpl::MXcbBoxControlImpl(MBoxControl* inControl, bool inHorizontal,
//		bool inHomogeneous, bool inExpand, bool inFill, uint32 inSpacing, uint32 inPadding)
//	: MXcbControlImpl(inControl, ""), mHorizontal(inHorizontal)
//	, mHomogeneous(inHomogeneous), mExpand(inExpand), mFill(inFill)
//	, mSpacing(inSpacing), mPadding(inPadding)
//{
//}
//
//void MXcbBoxControlImpl::CreateWidget()
//{
//	SetWidget(mHorizontal ? gtk_hbox_new(mHomogeneous, mSpacing) : gtk_vbox_new(mHomogeneous, mSpacing));
//}
//
//void MXcbBoxControlImpl::Append(MXcbWinMixin* inChild, MControlPacking inPacking,
//		bool inExpand, bool inFill, uint32 inPadding)
//{
//	assert(GTK_IS_BOX(GetWidget()));
//	
//	if (inPacking == ePackStart)
//		gtk_box_pack_start(GTK_BOX(GetWidget()), inChild->GetWidget(), inExpand, inFill, inPadding);
//	else
//		gtk_box_pack_end(GTK_BOX(GetWidget()), inChild->GetWidget(), inExpand, inFill, inPadding);
//}

MBoxControlImpl* MBoxControlImpl::Create(MBoxControl* inControl, bool inHorizontal,
		bool inHomogeneous, bool inExpand, bool inFill, uint32 inSpacing, uint32 inPadding)
{
//	return new MXcbBoxControlImpl(inControl, inHorizontal, inHomogeneous, inExpand, inFill, inSpacing, inPadding);
	return nullptr;
}

