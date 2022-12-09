//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <boost/algorithm/string.hpp>

#include "MColorPicker.hpp"
#include "MGtkCanvasImpl.hpp"
#include "MGtkControlsImpl.hpp"
#include "MGtkWindowImpl.hpp"
#include "MUtils.hpp"

#include "MGtkControlsImpl.inl"

using namespace std;
namespace ba = boost::algorithm;

const int kScrollbarWidth = 16; //::GetThemeSysSize(nullptr, SM_CXVSCROLL);

// --------------------------------------------------------------------

MGtkSimpleControlImpl::MGtkSimpleControlImpl(MSimpleControl *inControl)
	: MGtkControlImpl(inControl, "")
{
}

void MGtkSimpleControlImpl::CreateWidget()
{
	SetWidget(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
}

MSimpleControlImpl *MSimpleControlImpl::Create(MSimpleControl *inControl)
{
	return new MGtkSimpleControlImpl(inControl);
}

// --------------------------------------------------------------------

MGtkButtonImpl::MGtkButtonImpl(MButton *inButton, const string &inLabel,
                               MButtonFlags inFlags)
	: MGtkControlImpl(inButton, inLabel)
	, mClicked(this, &MGtkButtonImpl::Clicked)
	, mDefault(false)
{
}

void MGtkButtonImpl::CreateWidget()
{
	SetWidget(gtk_button_new_with_label(mLabel.c_str()));
	mClicked.Connect(GetWidget(), "clicked");
}

void MGtkButtonImpl::Clicked()
{
	mControl->eClicked(mControl->GetID());
}

void MGtkButtonImpl::SimulateClick()
{
	//	::SendMessage(GetWidget(), BM_SETSTATE, 1, 0);
	//	::UpdateWindow(GetWidget());
	//	::delay(12.0 / 60.0);
	//	::SendMessage(GetWidget(), BM_SETSTATE, 0, 0);
	//	::UpdateWindow(GetWidget());
	//
	//	mControl->eClicked(mControl->GetID());
}

void MGtkButtonImpl::MakeDefault(bool inDefault)
{
	mDefault = inDefault;

	if (GetWidget() != nullptr)
		gtk_widget_grab_default(GetWidget());
}

void MGtkButtonImpl::SetText(const std::string &inText)
{
	mLabel = inText;
	gtk_button_set_label(GTK_BUTTON(GetWidget()), mLabel.c_str());
	//	wstring text(c2w(inText));
	//	::SendMessage(GetWidget(), WM_SETTEXT, 0, (LPARAM)text.c_str());
}

void MGtkButtonImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();
	//	if (mDefault)
	//		::SendMessage(GetWidget(), BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, 0);
	//	else
	//		::SendMessage(GetWidget(), BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, 0);
}

void MGtkButtonImpl::GetIdealSize(int32_t &outWidth, int32_t &outHeight)
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

MButtonImpl *MButtonImpl::Create(MButton *inButton, const string &inLabel,
                                 MButtonFlags inFlags)
{
	return new MGtkButtonImpl(inButton, inLabel, inFlags);
}

// --------------------------------------------------------------------

MGtkExpanderImpl::MGtkExpanderImpl(MExpander *inExpander, const string &inLabel)
	: MGtkControlImpl(inExpander, inLabel)
	, mIsOpen(false)
{
}

MGtkExpanderImpl::~MGtkExpanderImpl()
{
	//	if (mDC)
	//		::ReleaseDC(GetWidget(), mDC);
}

void MGtkExpanderImpl::CreateWidget()
{
	SetWidget(gtk_expander_new(mLabel.c_str()));
}

void MGtkExpanderImpl::Append(MGtkWidgetMixin *inChild, MControlPacking inPacking,
                              bool inExpand, bool inFill, uint32_t inPadding)
{
	assert(GTK_IS_CONTAINER(GetWidget()));
	gtk_container_add(GTK_CONTAINER(GetWidget()), inChild->GetWidget());
}

void MGtkExpanderImpl::SetOpen(bool inOpen)
{
	if (inOpen != mIsOpen)
	{
		mIsOpen = inOpen;

		gtk_expander_set_expanded(GTK_EXPANDER(GetWidget()), mIsOpen);

		mControl->Invalidate();
		mControl->eClicked(mControl->GetID());
	}
}

bool MGtkExpanderImpl::IsOpen() const
{
	return mIsOpen;
}

//bool MGtkExpanderImpl::WMActivate(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
//{
//	mControl->Invalidate();
//	return false;
//}
//
//bool MGtkExpanderImpl::WMPaint(HWND inHWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	bool result = false;
//
//	HTHEME hTheme = ::OpenThemeData(inHWnd, VSCLASS_TASKDIALOG);
//	if (hTheme != nullptr)
//	{
//		PAINTSTRUCT lPs;
//		HDC hdc = ::BeginPaint(inHWnd, &lPs);
//		THROW_IF_NIL(hdc);
//
////		::GetUpdateRect(inHWnd, &lPs.rcPaint, false);
//
//		if (::IsThemeBackgroundPartiallyTransparent(hTheme, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL))
//			::DrawThemeParentBackground(inHWnd, hdc, &lPs.rcPaint);
//
//		RECT clientRect;
//		::GetClientRect(inHWnd, &clientRect);
//
//		RECT contentRect;
//		::GetThemeBackgroundContentRect(hTheme, hdc, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, &clientRect, &contentRect);
//
//		int w = contentRect.bottom - contentRect.top;
//		::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_WIDTH, &w);
//		int sr = contentRect.right;
//		contentRect.right = contentRect.left + w;
//
//		int state = TDLGEBS_NORMAL;
//		if ((mMouseInside and not mMouseDown) or (not mMouseInside and mMouseDown))
//			state = TDLGEBS_HOVER;
//		else if (mMouseDown)
//			state = TDLGEBS_PRESSED;
//
//		if (mIsOpen)
//			state += 3;		// expanded
//
//		::DrawThemeBackground(hTheme, hdc, TDLG_EXPANDOBUTTON, state, &contentRect, 0);
//
//	    ::CloseThemeData(hTheme);
//	    hTheme = ::OpenThemeData(inHWnd, VSCLASS_TEXTSTYLE);
//	    if (hTheme != nullptr)
//	    {
//			contentRect.left = contentRect.right;
//			contentRect.right = sr;
//
//			wstring label(c2w(mLabel));
//
//			RECT r;
//			::GetThemeTextExtent(hTheme, mDC, TEXT_BODYTEXT, 0, label.c_str(), label.length(), 0, nullptr, &r);
//			contentRect.left += (r.bottom - r.top) / 2;
//			contentRect.top += ((contentRect.bottom - contentRect.top) - (r.bottom - r.top)) / 2;
//
//			int state = TS_CONTROLLABEL_NORMAL;
//			if (not ::IsWindowEnabled(inHWnd))
//				state = TS_CONTROLLABEL_DISABLED;
//
//			::DrawThemeText(hTheme, hdc, TEXT_BODYTEXT, state, label.c_str(), label.length(),
//				0, 0, &contentRect);
//
//			::CloseThemeData(hTheme);
//	    }
//
//		::EndPaint (inHWnd, &lPs);
//
//	    result = true;
//	}
//
//	return result;
//}
//
//bool MGtkExpanderImpl::WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	::SetFocus(inHWnd);
//	::SetCapture(inHWnd);
//
//	mMouseInside = true;
//	mMouseDown = true;
//	mControl->Invalidate();
//	mControl->UpdateNow();
//
//	return true;
//}
//
//bool MGtkExpanderImpl::WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	if (not mMouseTracking)
//	{
//		TRACKMOUSEEVENT me = { sizeof(TRACKMOUSEEVENT) };
//		me.dwFlags = TME_LEAVE;
//		me.hwndTrack = GetWidget();
//
//		if (not mMouseDown)
//			me.dwHoverTime = HOVER_DEFAULT;
//
//		if (::TrackMouseEvent(&me))
//			mMouseTracking = true;
//
//		mControl->Invalidate();
//		mControl->UpdateNow();
//	}
//
//	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
//	int32_t y = static_cast<int16_t>(HIWORD(inLParam));
//
//	MRect bounds;
//	mControl->GetBounds(bounds);
//
//	if (mMouseInside != bounds.ContainsPoint(x, y))
//	{
//		mMouseInside = not mMouseInside;
//		mControl->Invalidate();
//		mControl->UpdateNow();
//	}
//
//	return true;
//}
//
//bool MGtkExpanderImpl::WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	mMouseInside = false;
//	mMouseTracking = false;
//	mLastExit = GetLocalTime();
//
//	mControl->Invalidate();
//	mControl->UpdateNow();
//
//	return true;
//}
//
//bool MGtkExpanderImpl::WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	::ReleaseCapture();
//
//	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
//	int32_t y = static_cast<int16_t>(HIWORD(inLParam));
//
//	MRect bounds;
//	mControl->GetBounds(bounds);
//	mMouseInside = bounds.ContainsPoint(x, y);
//
//	if (mMouseInside)
//	{
//		mIsOpen = not mIsOpen;
//		mControl->eClicked(mControl->GetID());
//	}
//	mMouseDown = false;
//
//	mControl->Invalidate();
//	mControl->UpdateNow();
//
//	return true;
//}

void MGtkExpanderImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	//	mDC = ::GetDC(GetWidget());
	//
	//	HTHEME hTheme = ::OpenThemeData(GetWidget(), VSCLASS_TASKDIALOG);
	//	if (hTheme != nullptr)
	//	{
	//		int w, h;
	//		RECT r;
	//
	//		wstring label(c2w(mLabel));
	//
	//		if (::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_WIDTH, &w) == S_OK and
	//			::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_HEIGHT, &h) == S_OK and
	//			::GetThemeTextExtent(hTheme, mDC, TDLG_EXPANDOTEXT, 0, label.c_str(), label.length(), 0, nullptr, &r) == S_OK)
	//		{
	//			MGtkProcMixin* parent;
	//			MRect bounds;
	//
	//			GetParentAndBounds(parent, bounds);
	//
	//			int lw = r.right - r.left + (r.bottom - r.top) / 2;
	//			mControl->ResizeFrame(w + lw - bounds.width, h - bounds.height);
	//		}
	//	}
}

MExpanderImpl *MExpanderImpl::Create(MExpander *inExpander, const string &inLabel)
{
	return new MGtkExpanderImpl(inExpander, inLabel);
}

// --------------------------------------------------------------------

MGtkScrollbarImpl::MGtkScrollbarImpl(MScrollbar *inScrollbar)
	: MGtkControlImpl(inScrollbar, "")
	, eValueChanged(this, &MGtkScrollbarImpl::ValueChanged)
{
}

void MGtkScrollbarImpl::CreateWidget()
{
	MRect bounds;
	mControl->GetBounds(bounds);

	GtkAdjustment *adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 1, 1, 1));

	if (bounds.width > bounds.height)
		SetWidget(gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, adjustment));
	else
		SetWidget(gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, adjustment));

	eValueChanged.Connect(GetWidget(), "value-changed");
}

int32_t MGtkScrollbarImpl::GetValue() const
{
	int32_t result = 0;

	if (GetWidget() != nullptr)
		result = gtk_range_get_value(GTK_RANGE(GetWidget()));

	return result;
}

void MGtkScrollbarImpl::SetValue(int32_t inValue)
{
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));

	if (adj != nullptr)
	{
		int32_t minValue = GetMinValue();

		if (inValue < minValue)
			inValue = minValue;

		int32_t maxValue = GetMaxValue();
		if (inValue > maxValue)
			inValue = maxValue;
	}

	gtk_range_set_value(GTK_RANGE(GetWidget()), inValue);
}

int32_t MGtkScrollbarImpl::GetTrackValue() const
{
	return GetValue();
}

void MGtkScrollbarImpl::SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
                                            int32_t inScrollUnit, int32_t inPageSize, int32_t inValue)
{
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));

	if (adj != nullptr)
	{
		if (inValue < inMinValue)
			inValue = inMinValue;

		int32_t maxValue = inMaxValue + 1 - inPageSize;
		if (inValue > maxValue)
			inValue = maxValue;

		// PRINT(("min: %d, max: %d, scrollunit: %d, page: %d, value: %d\n", inMinValue, inMaxValue, inScrollUnit, inPageSize, inValue));

		gtk_adjustment_set_lower(adj, inMinValue);
		gtk_adjustment_set_upper(adj, inMaxValue + 1);
		gtk_adjustment_set_step_increment(adj, inScrollUnit);
		gtk_adjustment_set_page_increment(adj, inPageSize);
		gtk_adjustment_set_page_size(adj, inPageSize);
		gtk_adjustment_set_value(adj, inValue);
	}
}

int32_t MGtkScrollbarImpl::GetMinValue() const
{
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));
	return adj == nullptr ? 0 : gtk_adjustment_get_lower(adj);
}

int32_t MGtkScrollbarImpl::GetMaxValue() const
{
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(GetWidget()));

	int32_t result = 0;
	if (adj != nullptr)
	{
		result = gtk_adjustment_get_upper(adj);
		if (gtk_adjustment_get_page_size(adj) > 1)
			result -= gtk_adjustment_get_page_size(adj);
	}

	return result;
}

void MGtkScrollbarImpl::ValueChanged()
{
	mControl->eScroll(kScrollToThumb);
}

MScrollbarImpl *MScrollbarImpl::Create(MScrollbar *inScrollbar)
{
	return new MGtkScrollbarImpl(inScrollbar);
}

// --------------------------------------------------------------------

MGtkStatusbarImpl::MGtkStatusbarImpl(MStatusbar *inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
	: MGtkControlImpl(inStatusbar, "")
	, mParts(inParts, inParts + inPartCount)
	, mClicked(this, &MGtkStatusbarImpl::Clicked)
{
}

void MGtkStatusbarImpl::CreateWidget()
{
	GtkWidget *statusBar = gtk_statusbar_new();

	GtkShadowType shadow_type = GTK_SHADOW_NONE;
	//	gtk_widget_style_get(statusBar, "shadow_type", &shadow_type, nullptr);

	for (auto part : mParts)
	{
		GtkWidget *frame = gtk_frame_new(nullptr);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), shadow_type);

		GtkWidget *label = gtk_label_new("");
		gtk_label_set_single_line_mode(GTK_LABEL(label), true);
		gtk_label_set_selectable(GTK_LABEL(label), true);
		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_label_set_yalign(GTK_LABEL(label), 0.5);

		gtk_container_add(GTK_CONTAINER(frame), label);

		if (part.packing == ePackStart)
			gtk_box_pack_start(GTK_BOX(statusBar), frame, part.expand, part.fill, part.padding);
		else
			gtk_box_pack_end(GTK_BOX(statusBar), frame, part.expand, part.fill, part.padding);

		if (part.width > 0)
			gtk_widget_set_size_request(frame, part.width, -1);

		mPanels.push_back(label);

		mClicked.Connect(label, "button-press-event");
	}

	SetWidget(statusBar);
}

void MGtkStatusbarImpl::AddedToWindow()
{
	CreateWidget();

	MGtkWidgetMixin *parent;
	MRect bounds;

	GetParentAndBounds(parent, bounds);

	MGtkWindowImpl *impl = dynamic_cast<MGtkWindowImpl *>(parent);
	assert(impl != nullptr);
	impl->AddStatusbarWidget(this);
}

void MGtkStatusbarImpl::SetStatusText(uint32_t inPartNr, const string &inText, bool inBorder)
{
	if (inPartNr < mPanels.size())
		gtk_label_set_text(GTK_LABEL(mPanels[inPartNr]), inText.c_str());
}

bool MGtkStatusbarImpl::Clicked(GdkEventButton *inEvent)
{
	GtkWidget *source = GTK_WIDGET(mClicked.GetSourceGObject());

	auto panel = find(mPanels.begin(), mPanels.end(), source);
	if (panel != mPanels.end())
		mControl->ePartClicked(panel - mPanels.begin(), MRect());

	return true;
}

MStatusbarImpl *MStatusbarImpl::Create(MStatusbar *inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
{
	return new MGtkStatusbarImpl(inStatusbar, inPartCount, inParts);
}

// --------------------------------------------------------------------

MGtkComboboxImpl::MGtkComboboxImpl(MCombobox *inCombobox)
	: MGtkControlImpl(inCombobox, "")
{
}

void MGtkComboboxImpl::CreateWidget()
{
	GtkTreeModel *list_store = GTK_TREE_MODEL(gtk_list_store_new(1, G_TYPE_STRING));
	THROW_IF_NIL(list_store);

	GtkWidget *wdgt = gtk_combo_box_new_with_model_and_entry(list_store);
	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(wdgt), 0);

	SetWidget(wdgt);
}

string MGtkComboboxImpl::GetText() const
{
	string result;

	//	char* text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(GetWidget()));
	const char *text = gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(GetWidget()))));
	if (text != nullptr)
		result = text;
	return result;
}

void MGtkComboboxImpl::SetText(const std::string &inText)
{
	auto i = find(mChoices.begin(), mChoices.end(), inText);
	if (i == mChoices.end())
	{
		mChoices.insert(mChoices.begin(), inText);
		i = mChoices.begin();

		SetChoices(mChoices);
	}

	GtkWidget *wdgt = GetWidget();

	if (not GTK_IS_COMBO_BOX(wdgt))
		THROW(("Item is not a combo box"));

	auto ix = i - mChoices.begin();
	if (ix != gtk_combo_box_get_active(GTK_COMBO_BOX(wdgt)))
		gtk_combo_box_set_active(GTK_COMBO_BOX(wdgt), ix);
}

void MGtkComboboxImpl::SetChoices(const std::vector<std::string> &inChoices)
{
	mChoices = inChoices;

	GtkWidget *wdgt = GetWidget();

	if (wdgt != nullptr)
	{
		mChanged.Disconnect(wdgt);
	
		if (not GTK_IS_COMBO_BOX(wdgt))
			THROW(("Item is not a combo box"));

		GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(wdgt)));
		THROW_IF_NIL(model);

		gtk_list_store_clear(model);

		for (auto s : inChoices)
		{
			GtkTreeIter iter;

			gtk_list_store_append(model, &iter);
			gtk_list_store_set(model, &iter,
			                   0, s.c_str(),
			                   -1);
		}

		gtk_combo_box_set_active(GTK_COMBO_BOX(wdgt), 0);

		// connect signal
		mChanged.Connect(wdgt, "changed");
	}
}

void MGtkComboboxImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	if (not mChoices.empty())
		SetChoices(mChoices);
}

bool MGtkComboboxImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = false;

	if (inKeyCode == kReturnKeyCode or
	    inKeyCode == kEnterKeyCode or
	    inKeyCode == kTabKeyCode or
	    inKeyCode == kEscapeKeyCode or
	    (inModifiers & ~kShiftKey) != 0)
	{
		//		result = MGtkControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
	}

	return result;
}

void MGtkComboboxImpl::OnChanged()
{
	mControl->eValueChanged(mControl->GetID(), GetText());
}

MComboboxImpl *MComboboxImpl::Create(MCombobox *inCombobox)
{
	return new MGtkComboboxImpl(inCombobox);
}

// --------------------------------------------------------------------

MGtkPopupImpl::MGtkPopupImpl(MPopup *inPopup)
	: MGtkControlImpl(inPopup, "")
{
}

void MGtkPopupImpl::CreateWidget()
{
	SetWidget(gtk_combo_box_text_new());
}

void MGtkPopupImpl::SetChoices(const std::vector<std::string> &inChoices)
{
	mChoices = inChoices;

	if (GetWidget() != nullptr)
	{
		for (auto s : inChoices)
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(GetWidget()), s.c_str());

		gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), 0);

		// connect signal
		mChanged.Connect(GetWidget(), "changed");
	}
}

void MGtkPopupImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	if (not mChoices.empty())
		SetChoices(mChoices);
}

int32_t MGtkPopupImpl::GetValue() const
{
	return gtk_combo_box_get_active(GTK_COMBO_BOX(GetWidget()));
}

void MGtkPopupImpl::SetValue(int32_t inValue)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), inValue);
}

void MGtkPopupImpl::SetText(const string &inText)
{
	auto i = find(mChoices.begin(), mChoices.end(), inText);
	if (i != mChoices.end())
		SetValue(i - mChoices.begin());
}

string MGtkPopupImpl::GetText() const
{
	const char *s = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(GetWidget()));
	return s ? s : "";
}

bool MGtkPopupImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = false;

	if (inKeyCode == kReturnKeyCode or
	    inKeyCode == kEnterKeyCode or
	    inKeyCode == kTabKeyCode or
	    inKeyCode == kEscapeKeyCode or
	    (inModifiers & ~kShiftKey) != 0)
	{
		//		result = MGtkControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
	}

	return result;
}

MPopupImpl *MPopupImpl::Create(MPopup *inPopup)
{
	return new MGtkPopupImpl(inPopup);
}

// --------------------------------------------------------------------

MGtkEdittextImpl::MGtkEdittextImpl(MEdittext *inEdittext, uint32_t inFlags)
	: MGtkControlImpl(inEdittext, "")
	, mFlags(inFlags)
{
}

void MGtkEdittextImpl::CreateWidget()
{
	SetWidget(gtk_entry_new());
}

void MGtkEdittextImpl::SetFocus()
{
	MGtkControlImpl::SetFocus();
	//
	//	::SendMessage(GetWidget(), EM_SETSEL, 0, -1);
}

string MGtkEdittextImpl::GetText() const
{
	const char *result = nullptr;
	if (GTK_IS_ENTRY(GetWidget()))
		result = gtk_entry_get_text(GTK_ENTRY(GetWidget()));
	return result ? result : "";
}

void MGtkEdittextImpl::SetText(const std::string &inText)
{
	if (GTK_IS_ENTRY(GetWidget()))
		gtk_entry_set_text(GTK_ENTRY(GetWidget()), inText.c_str());
}

void MGtkEdittextImpl::SetPasswordChar(uint32_t inUnicode)
{
	GtkWidget *wdgt = GetWidget();
	if (GTK_IS_ENTRY(wdgt))
	{
		gtk_entry_set_visibility(GTK_ENTRY(wdgt), false);
		gtk_entry_set_invisible_char(GTK_ENTRY(wdgt), inUnicode);
	}
	else
		THROW(("item is not an entry"));
}

bool MGtkEdittextImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = false;

	if (inKeyCode == kReturnKeyCode or
	    inKeyCode == kEnterKeyCode or
	    inKeyCode == kTabKeyCode or
	    inKeyCode == kEscapeKeyCode or
	    (inModifiers & ~kShiftKey) != 0)
	{
		//		result = MGtkControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
	}

	return result;
}

MEdittextImpl *MEdittextImpl::Create(MEdittext *inEdittext, uint32_t inFlags)
{
	return new MGtkEdittextImpl(inEdittext, inFlags);
}

// --------------------------------------------------------------------

MGtkCaptionImpl::MGtkCaptionImpl(MCaption *inControl, const string &inText)
	: MGtkControlImpl(inControl, inText)
{
}

void MGtkCaptionImpl::CreateWidget()
{
	GtkWidget *widget = gtk_label_new(mLabel.c_str());
	//	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_label_set_xalign(GTK_LABEL(widget), 0);
	SetWidget(widget);
}

void MGtkCaptionImpl::SetText(const string &inText)
{
	mLabel = inText;
	if (GTK_IS_LABEL(GetWidget()))
		gtk_label_set_text(GTK_LABEL(GetWidget()), inText.c_str());
}

MCaptionImpl *MCaptionImpl::Create(MCaption *inCaption, const std::string &inText)
{
	return new MGtkCaptionImpl(inCaption, inText);
}

// --------------------------------------------------------------------

MGtkSeparatorImpl::MGtkSeparatorImpl(MSeparator *inControl)
	: MGtkControlImpl(inControl, "")
{
}

void MGtkSeparatorImpl::CreateWidget()
{
	SetWidget(gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
}

MSeparatorImpl *MSeparatorImpl::Create(MSeparator *inSeparator)
{
	return new MGtkSeparatorImpl(inSeparator);
}

// --------------------------------------------------------------------

MGtkCheckboxImpl::MGtkCheckboxImpl(MCheckbox *inControl, const string &inText)
	: MGtkControlImpl(inControl, inText)
	, mChecked(false)
{
}

void MGtkCheckboxImpl::CreateWidget()
{
	SetWidget(gtk_check_button_new_with_label(mLabel.c_str()));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), mChecked);
}

bool MGtkCheckboxImpl::IsChecked() const
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GetWidget()));
}

void MGtkCheckboxImpl::SetChecked(bool inChecked)
{
	mChecked = inChecked;
	if (GetWidget())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), mChecked);
}

MCheckboxImpl *MCheckboxImpl::Create(MCheckbox *inCheckbox, const string &inText)
{
	return new MGtkCheckboxImpl(inCheckbox, inText);
}

// --------------------------------------------------------------------

MGtkRadiobuttonImpl::MGtkRadiobuttonImpl(MRadiobutton *inControl, const string &inText)
	: MGtkControlImpl(inControl, inText)
{
}

void MGtkRadiobuttonImpl::CreateWidget()
{
	if (mGroup.empty() or mGroup.front() == mControl)
		SetWidget(gtk_radio_button_new_with_label(nullptr, mLabel.c_str()));
	else if (not mGroup.empty())
	{
		MGtkRadiobuttonImpl *first = dynamic_cast<MGtkRadiobuttonImpl *>(mGroup.front()->GetImpl());

		SetWidget(gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(first->GetWidget()), mLabel.c_str()));
	}
}

bool MGtkRadiobuttonImpl::IsChecked() const
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GetWidget()));
}

void MGtkRadiobuttonImpl::SetChecked(bool inChecked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GetWidget()), inChecked);
}

void MGtkRadiobuttonImpl::SetGroup(const list<MRadiobutton *> &inButtons)
{
	mGroup = inButtons;
}

//bool MGtkRadiobuttonImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	bool result = false;
//
//	if (inUMsg == BN_CLICKED)
//	{
//		bool checked = not IsChecked();
//
//		SetChecked(checked);
//		mControl->eValueChanged(mControl->GetID(), checked);
//
//		outResult = 1;
//		result = true;
//	}
//
//	return result;
//}
//
MRadiobuttonImpl *MRadiobuttonImpl::Create(MRadiobutton *inRadiobutton, const std::string &inText)
{
	return new MGtkRadiobuttonImpl(inRadiobutton, inText);
}

// --------------------------------------------------------------------

MGtkListHeaderImpl::MGtkListHeaderImpl(MListHeader *inListHeader)
	: MGtkControlImpl(inListHeader, "")
{
}

void MGtkListHeaderImpl::CreateWidget()
{
	assert(false);
}

void MGtkListHeaderImpl::AppendColumn(const string &inLabel, int inWidth)
{
	//	HDITEM item = {};
	//
	//	wstring label = c2w(inLabel);
	//
	//	item.mask = HDI_FORMAT | HDI_TEXT;
	//	item.pszText = const_cast<wchar_t*>(label.c_str());
	//	item.cchTextMax = label.length();
	//	item.fmt = HDF_LEFT | HDF_STRING;
	//
	//	if (inWidth > 0)
	//	{
	//		item.mask |= HDI_WIDTH;
	//		item.cxy = inWidth;
	//	}
	//
	//	int insertAfter = ::SendMessage(GetWidget(), HDM_GETITEMCOUNT, 0, 0);
	//	::SendMessage(GetWidget(), HDM_INSERTITEM, (WPARAM)&insertAfter, (LPARAM)&item);
}

MListHeaderImpl *MListHeaderImpl::Create(MListHeader *inListHeader)
{
	return new MGtkListHeaderImpl(inListHeader);
}

// --------------------------------------------------------------------

MGtkNotebookImpl::MGtkNotebookImpl(MNotebook *inControl)
	: MGtkControlImpl(inControl, "")
{
}

void MGtkNotebookImpl::CreateWidget()
{
	assert(false);
}

void MGtkNotebookImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	// add pages
	if (not mPages.empty())
	{
		vector<MPage> pages(mPages);
		mPages.clear();

		for (MPage &page : pages)
			AddPage(page.mTitle, page.mPage);

		FrameResized();
	}

	//	MRect bounds;
	//	MGtkProcMixin* parent;
	//
	//	GetParentAndBounds(parent, bounds);
	//
	//	parent->AddNotify(TCN_SELCHANGE, GetWidget(),
	//		std::bind(&MGtkNotebookImpl::TCNSelChange, this, _1, _2, _3));
}

void MGtkNotebookImpl::FrameResized()
{
	MGtkControlImpl<MNotebook>::FrameResized();

	//	RECT rc;
	//	::GetClientRect(GetWidget(), &rc);
	//	TabCtrl_AdjustRect(GetWidget(), false, &rc);
	//
	//	for (MPage& page: mPages)
	//	{
	//		MRect frame;
	//		page.mPage->GetFrame(frame);
	//
	//		int32_t dx, dy, dw, dh;
	//		dx = rc.left - frame.x;
	//		dy = rc.top - frame.y;
	//		dw = rc.right - rc.left - frame.width;
	//		dh = rc.bottom - rc.top - frame.height;
	//
	//		page.mPage->MoveFrame(dx, dy);
	//		page.mPage->ResizeFrame(dw, dh);
	//	}
}

void MGtkNotebookImpl::AddPage(const string &inLabel, MView *inPage)
{
	MPage page = {inLabel, inPage};
	mPages.push_back(page);

	//	if (GetWidget())
	//	{
	//		wstring s(c2w(inLabel));
	//
	//		TCITEM tci = {};
	//		tci.mask = TCIF_TEXT;
	//		tci.pszText = const_cast<wchar_t*>(s.c_str());
	//
	//		::SendMessage(GetWidget(), TCM_INSERTITEM, (WPARAM)mPages.size(), (LPARAM)&tci);
	//
	//		RECT r;
	//		::GetClientRect(GetWidget(), &r);
	//		TabCtrl_AdjustRect(GetWidget(), false, &r);
	//
	//		MRect frame;
	//		inPage->GetFrame(frame);
	//
	//		if (frame.x != r.left or frame.y != r.top)
	//			inPage->MoveFrame(r.left - frame.x, r.top - frame.y);
	//
	//		if (frame.height != r.bottom - r.top or
	//			frame.width != r.right - r.left)
	//		{
	//			inPage->ResizeFrame((r.right - r.left) - frame.width, (r.bottom - r.top) - frame.height);
	//		}
	//
	//		mControl->AddChild(inPage);
	//
	//		if (mPages.size() > 1)
	//			inPage->Hide();
	//	}
}

void MGtkNotebookImpl::SelectPage(uint32_t inPage)
{
	//	if (inPage != ::SendMessage(GetWidget(), TCM_GETCURSEL, 0, 0))
	//	{
	//		::SendMessage(GetWidget(), TCM_SETCURSEL, (WPARAM)inPage, 0);
	//		::UpdateWindow(GetWidget());
	//	}
	//
	//	if (inPage < mPages.size())
	//	{
	//		for (MPage& page: mPages)
	//		{
	//			page.mPage->Hide();
	//			page.mPage->Disable();
	//		}
	//		mPages[inPage].mPage->Show();
	//		mPages[inPage].mPage->Enable();
	//		mControl->ePageSelected(inPage);
	//	}
}

uint32_t MGtkNotebookImpl::GetSelectedPage() const
{
	//	return ::SendMessage(GetWidget(), TCM_GETCURSEL, 0, 0);
	return 0;
}

MNotebookImpl *MNotebookImpl::Create(MNotebook *inNotebook)
{
	return new MGtkNotebookImpl(inNotebook);
}

// --------------------------------------------------------------------

MGtkColorSwatchImpl::MGtkColorSwatchImpl(MColorSwatch *inColorSwatch, MColor inColor)
	: MGtkControlImpl(inColorSwatch, "")
	, eSelectedColor(this, &MGtkColorSwatchImpl::SelectedColor)
	, ePreviewColor(this, &MGtkColorSwatchImpl::PreviewColor)
	, mColorSet(this, &MGtkColorSwatchImpl::OnColorSet)
	, mColor(inColor)
{
}

void MGtkColorSwatchImpl::CreateWidget()
{
	GdkRGBA color = {};
	color.red = mColor.red / 255.0;
	color.green = mColor.green / 255.0;
	color.blue = mColor.blue / 255.0;
	color.alpha = 1.0;

	SetWidget(gtk_color_button_new_with_rgba(&color));

	mColorSet.Connect(GetWidget(), "color-set");
}

void MGtkColorSwatchImpl::OnColorSet()
{
	GdkRGBA color = {};
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(GetWidget()), &color);

	mColor.red = static_cast<uint8_t>(255 * color.red);
	mColor.green = static_cast<uint8_t>(255 * color.green);
	mColor.blue = static_cast<uint8_t>(255 * color.blue);
}

void MGtkColorSwatchImpl::SelectedColor(MColor inColor)
{
	SetColor(inColor);
	mControl->eColorChanged(mControl->GetID(), mColor);
}

void MGtkColorSwatchImpl::PreviewColor(MColor inColor)
{
	mControl->eColorPreview(mControl->GetID(), inColor);
}

MColor MGtkColorSwatchImpl::GetColor() const
{
	return mColor;
}

void MGtkColorSwatchImpl::SetColor(MColor inColor)
{
	mColor = inColor;

	GdkRGBA color = {};
	color.red = mColor.red / 255.0;
	color.green = mColor.green / 255.0;
	color.blue = mColor.blue / 255.0;
	color.alpha = 1.0;
	if (GTK_IS_COLOR_BUTTON(GetWidget()))
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(GetWidget()), &color);
}

//void MGtkColorSwatchImpl::GetIdealSize(int32_t& outWidth, int32_t& outHeight)
//{
//	outWidth = 30;
//	outHeight = 23;
//
//	SIZE size;
//	if (GetWidget() != nullptr and ColorSwatch_GetIdealSize(GetWidget(), &size))
//	{
//		if (outWidth < size.cx + 20)
//			outWidth = size.cx + 20;
//
//		if (outHeight < size.cy + 2)
//			outHeight = size.cy + 2;
//	}
//}

MColorSwatchImpl *MColorSwatchImpl::Create(MColorSwatch *inColorSwatch, MColor inColor)
{
	return new MGtkColorSwatchImpl(inColorSwatch, inColor);
}

// --------------------------------------------------------------------

MGtkListBoxImpl::MGtkListBoxImpl(MListBox *inListBox)
	: MGtkControlImpl(inListBox, "")
	, mSelectionChanged(this, &MGtkListBoxImpl::OnSelectionChanged)
	, mStore(nullptr)
	, mNr(0)
{
}

void MGtkListBoxImpl::CreateWidget()
{
	mStore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	SetWidget(gtk_tree_view_new_with_model(GTK_TREE_MODEL(mStore)));

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(GetWidget()), false);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Author",
	                                                                     renderer,
	                                                                     "text", 0,
	                                                                     NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(GetWidget()), column);

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	mSelectionChanged.Connect(G_OBJECT(selection), "changed");
}

//void MGtkListBoxImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
//	wstring& outClassName, HMENU& outMenu)
//{
//	MGtkControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
//
//	outStyle = WS_CHILD | LBS_HASSTRINGS | LBS_NOTIFY;
//	outExStyle |= WS_EX_CLIENTEDGE;
//
//	outClassName = WC_LISTBOX;
//}

void MGtkListBoxImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	for (string &item : mItems)
		AddItem(item);

	mItems.clear();

	SetValue(0);
}

void MGtkListBoxImpl::AddItem(const string &inText)
{
	if (mStore == nullptr)
		mItems.push_back(inText);
	else
	{
		GtkTreeIter iter;

		gtk_list_store_append(mStore, &iter);
		gtk_list_store_set(mStore, &iter,
		                   0, inText.c_str(), 1, mNr++, -1);
	}
}

int32_t MGtkListBoxImpl::GetValue() const
{
	int32_t result = -1;

	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_tree_model_get(GTK_TREE_MODEL(mStore), &iter, 1, &result, -1);

	return result;
}

void MGtkListBoxImpl::SetValue(int32_t inValue)
{
	//	::SendMessage(GetWidget(), LB_SETCURSEL, inValue, 0);
}

void MGtkListBoxImpl::OnSelectionChanged()
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		int32_t selected;
		gtk_tree_model_get(GTK_TREE_MODEL(mStore), &iter, 1, &selected, -1);
		mControl->eValueChanged(mControl->GetID(), selected);
	}
}

//bool MGtkListBoxImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	switch (inUMsg)
//	{
//		case LBN_SELCHANGE:
//		case LBN_SELCANCEL:
//			mControl->eValueChanged(mControl->GetID(), ::SendMessage(GetWidget(), LB_GETCURSEL, 0, 0));
//			break;
//	}
//
//	return true;
//}
//
MListBoxImpl *MListBoxImpl::Create(MListBox *inListBox)
{
	return new MGtkListBoxImpl(inListBox);
}

// // --------------------------------------------------------------------

// MGtkListViewImpl::MGtkListViewImpl(MListView *inListView)
// 	: MGtkControlImpl(inListView, "")
// 	, mStore(nullptr)
// {
// }

// void MGtkListViewImpl::CreateWidget()
// {
// 	mStore = gtk_list_store_new(1, G_TYPE_STRING);

// 	for (string &s : mItems)
// 	{
// 		GtkTreeIter iter;

// 		gtk_list_store_append(mStore, &iter);
// 		gtk_list_store_set(mStore, &iter,
// 		                   0, s.c_str(),
// 		                   -1);
// 	}

// 	SetWidget(gtk_tree_view_new_with_model(GTK_TREE_MODEL(mStore)));

// 	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
// 	//	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Author",
// 	//                                                   renderer,
// 	//                                                   "text", 0,
// 	//                                                   NULL);

// 	GtkTreeViewColumn *column = gtk_tree_view_column_new();
// 	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
// 	gtk_tree_view_column_pack_start(column, renderer, true);
// 	gtk_tree_view_append_column(GTK_TREE_VIEW(GetWidget()), column);
// }

//void MGtkListViewImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
//	wstring& outClassName, HMENU& outMenu)
//{
//	MGtkControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
//
//	outStyle = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
//	outExStyle |= WS_EX_STATICEDGE;
//
//	outClassName = WC_LISTVIEW;
//}
//
//void MGtkListViewImpl::CreateHandle(MGtkProcMixin* inParent, MRect inBounds, const wstring& inTitle)
//{
//	MGtkControlImpl::CreateHandle(inParent, inBounds, inTitle);
//
//	// add a single column
//    LVCOLUMN lvc = {};
//
//    lvc.mask = LVCF_FMT | LVCF_SUBITEM;
//	lvc.cx = inBounds.width - 10;
//	lvc.fmt = LVCFMT_LEFT;
//	::SendMessage(GetWidget(), LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
//
//	if (inParent != nullptr)
//	{
//		inParent->AddNotify(LVN_ITEMACTIVATE, GetWidget(),
//			std::bind(&MGtkListViewImpl::LVMItemActivate, this, _1, _2, _3));
//		inParent->AddNotify(LVN_GETDISPINFO, GetWidget(),
//			std::bind(&MGtkListViewImpl::LVMGetDispInfo, this, _1, _2, _3));
//	}
//}
//
// void MGtkListViewImpl::AddedToWindow()
// {
// 	MGtkControlImpl::AddedToWindow();

// 	for (string &item : mItems)
// 		AddItem(item);

// 	mItems.clear();
// }

// void MGtkListViewImpl::AddItem(const string &inText)
// {
// 	if (GetWidget() != nullptr)
// 		mItems.push_back(inText);
// 	else
// 	{
// 		GtkTreeIter iter;

// 		gtk_list_store_append(mStore, &iter);
// 		gtk_list_store_set(mStore, &iter,
// 		                   0, inText.c_str(),
// 		                   -1);
// 	}
// }

//bool MGtkListViewImpl::LVMItemActivate(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	NMITEMACTIVATE* nmItemActivate = reinterpret_cast<NMITEMACTIVATE*>(inLParam);
//
//	mControl->eValueChanged(mControl->GetID(), nmItemActivate->iItem);
//
//	return true;
//}
//
//bool MGtkListViewImpl::LVMGetDispInfo(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(inLParam);
//
//
//	return true;
//}
//
// MListViewImpl *MListViewImpl::Create(MListView *inListView)
// {
// 	return new MGtkListViewImpl(inListView);
// }

// --------------------------------------------------------------------

MGtkBoxControlImpl::MGtkBoxControlImpl(MBoxControl *inControl, bool inHorizontal,
                                       bool inHomogeneous, bool inExpand, bool inFill, uint32_t inSpacing, uint32_t inPadding)
	: MGtkControlImpl(inControl, "")
	, mHorizontal(inHorizontal)
	, mHomogeneous(inHomogeneous)
	, mExpand(inExpand)
	, mFill(inFill)
	, mSpacing(inSpacing)
	, mPadding(inPadding)
{
}

void MGtkBoxControlImpl::CreateWidget()
{
	SetWidget(gtk_box_new(mHorizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, mSpacing));
}

MBoxControlImpl *MBoxControlImpl::Create(MBoxControl *inControl, bool inHorizontal,
                                         bool inHomogeneous, bool inExpand, bool inFill, uint32_t inSpacing, uint32_t inPadding)
{
	return new MGtkBoxControlImpl(inControl, inHorizontal, inHomogeneous, inExpand, inFill, inSpacing, inPadding);
}

void MGtkBoxControlImpl::Append(MGtkWidgetMixin *inChild, MControlPacking inPacking,
                                bool inExpand, bool inFill, uint32_t inPadding)
{
	assert(GTK_IS_BOX(GetWidget()));

	auto childWidget = inChild->GetWidget();
	gtk_widget_set_margin_top(childWidget, inPadding);
	gtk_widget_set_margin_bottom(childWidget, inPadding);
	gtk_widget_set_margin_start(childWidget, inPadding);
	gtk_widget_set_margin_end(childWidget, inPadding);

	if (inPacking == ePackStart)
		gtk_box_pack_start(GTK_BOX(GetWidget()), childWidget, inExpand, inFill, 0);
	else
		gtk_box_pack_end(GTK_BOX(GetWidget()), childWidget, inExpand, inFill, 0);
}
