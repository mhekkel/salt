//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include <boost/algorithm/string.hpp>

#include "MWinWindowImpl.hpp"
#include "MWinControlsImpl.hpp"
#include "MWinCanvasImpl.hpp"
#include "MWinUtils.hpp"
#include "MUtils.hpp"
#include "MColorPicker.hpp"
#include "MError.hpp"

#pragma comment (lib, "uxtheme")
#pragma comment (linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;
namespace ba = boost::algorithm;

const int kScrollbarWidth = ::GetThemeSysSize(nullptr, SM_CXVSCROLL);

template<class CONTROL>
MWinControlImpl<CONTROL>::MWinControlImpl(CONTROL* inControl, const string& inLabel)
	: CONTROL::MImpl(inControl)
	, MWinProcMixin(inControl)
	, mLabel(inLabel)
{
}

template<class CONTROL>
MWinControlImpl<CONTROL>::~MWinControlImpl()
{
	if (GetHandle() != nullptr)
		::DestroyWindow(GetHandle());
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::SetFocus()
{
	if (GetHandle() != nullptr)
	{
		::SetFocus(GetHandle());
		::UpdateWindow(GetHandle());
	}
}

template<class CONTROL>
string MWinControlImpl<CONTROL>::GetText() const
{
	string result;

	if (GetHandle())
	{
		const int kBufferSize = 1024;
		vector<wchar_t> buf(kBufferSize);

		::GetWindowTextW(GetHandle(), &buf[0], kBufferSize);

		result = w2c(&buf[0]);
	}
	else
		result = mLabel;

	return result;
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::SetText(const std::string& inText)
{
	mLabel = inText;

	wstring s(c2w(inText));
	::SetWindowTextW(GetHandle(), s.c_str());
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::ActivateSelf()
{
	::EnableWindow(GetHandle(), mControl->IsActive() and mControl->IsEnabled());
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::DeactivateSelf()
{
	if (::IsWindowEnabled(GetHandle()))
		::EnableWindow(GetHandle(), false);
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::EnableSelf()
{
	::EnableWindow(GetHandle(), mControl->IsEnabled());
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::DisableSelf()
{
	if (::IsWindowEnabled(GetHandle()))
		::EnableWindow(GetHandle(), false);
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::ShowSelf()
{
	::ShowWindow(GetHandle(), SW_SHOW);
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::HideSelf()
{
	::ShowWindow(GetHandle(), SW_HIDE);
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::Draw(MRect inUpdate)
{
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::FrameMoved()
{
	FrameResized();
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::FrameResized()
{
	if (GetHandle() != nullptr)
	{
		MRect bounds;
		MWinProcMixin* parent;
		
		GetParentAndBounds(parent, bounds);
		
		::MoveWindow(GetHandle(), bounds.x, bounds.y,
			bounds.width, bounds.height, true);
	}
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::GetParentAndBounds(MWinProcMixin*& outParent, MRect& outBounds)
{
	MView* view = mControl;
	MView* parent = view->GetParent();
	
	view->GetBounds(outBounds);
	
	while (parent != nullptr)
	{
		view->ConvertToParent(outBounds.x, outBounds.y);
		
		MControlBase* ctl = dynamic_cast<MControlBase*>(parent);
		
		if (ctl != nullptr)
		{
			MControlImplBase* impl = ctl->GetControlImplBase();

			if (impl != nullptr and impl->GetWinProcMixin() != nullptr)
			{
				outParent = impl->GetWinProcMixin();
				break;
			}
		}
		
		MCanvas* canvas = dynamic_cast<MCanvas*>(parent);
		if (canvas != nullptr)
		{
			outParent = static_cast<MWinCanvasImpl*>(canvas->GetImpl());
			break;
		}
		
		MWindow* window = dynamic_cast<MWindow*>(parent);
		if (window != nullptr)
		{
			outParent = static_cast<MWinWindowImpl*>(window->GetImpl());
			break;
		}
		
		view = parent;
		parent = parent->GetParent();
	}
}

template<class CONTROL>
void MWinControlImpl<CONTROL>::AddedToWindow()
{
	MWinProcMixin* parent;
	MRect bounds;

	GetParentAndBounds(parent, bounds);

	CreateHandle(parent, bounds, c2w(GetText()));
	
	SubClass();

	// code to set the correct theme font
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
	HFONT hFont = ::CreateFontIndirect(&ncm.lfMessageFont);
	::SendMessage(GetHandle(), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

	RECT r;
	::GetClientRect(GetHandle(), &r);
	if (r.right - r.left != bounds.width or
		r.bottom - r.top != bounds.height)
	{
		::MapWindowPoints(GetHandle(), parent->GetHandle(), (LPPOINT)&r, 2);

		mControl->MoveFrame(
			r.left - bounds.x, r.top - bounds.y);
		mControl->ResizeFrame(
			(r.right - r.left) - bounds.width,
			(r.bottom - r.top) - bounds.height);
	}
	
	if (mControl->IsVisible())
		ShowSelf();
	else
		HideSelf();
}

template<class CONTROL>
MWinProcMixin* MWinControlImpl<CONTROL>::GetWinProcMixin()
{
	MWinProcMixin* mixin = this;
	return mixin;
}

// --------------------------------------------------------------------

MWinSimpleControlImpl::MWinSimpleControlImpl(MSimpleControl* inControl)
	: MWinControlImpl(inControl, "")
{
}

void MWinSimpleControlImpl::CreateParams(DWORD& outStyle,
	DWORD& outExStyle, wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = WC_STATIC;
	outStyle = WS_CHILD /*| SS_OWNERDRAW*/;
}

MSimpleControlImpl* MSimpleControlImpl::Create(MSimpleControl* inControl)
{
	return new MWinSimpleControlImpl(inControl);
}

// --------------------------------------------------------------------

MWinButtonImpl::MWinButtonImpl(MButton* inButton, const string& inLabel,
		MButtonFlags inFlags)
	: MWinControlImpl(inButton, inLabel)
	, mFlags(inFlags)
	, mDefault(false)
{
}

void MWinButtonImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outStyle = WS_CHILD | BS_PUSHBUTTON | BS_TEXT | WS_TABSTOP;
	if (mFlags & eBF_Split)
		outStyle |= BS_SPLITBUTTON;
	
	outClassName = L"BUTTON";
}

void MWinButtonImpl::CreateHandle(MWinProcMixin* inParent,
	MRect inBounds, const wstring& inTitle)
{
	using namespace std::placeholders;

	MWinControlImpl::CreateHandle(inParent, inBounds, inTitle);
	
	if (inParent != nullptr and mFlags & eBF_Split)
	{
		inParent->AddNotify(BCN_DROPDOWN, GetHandle(),
			std::bind(&MWinButtonImpl::BCMSetDropDownState, this, _1, _2, _3));
	}
}

bool MWinButtonImpl::WMCommand(HWND inHWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	if (inMsg == BN_CLICKED)
	{
		mControl->eClicked(mControl->GetID());
		outResult = 1;
		result = true;
	}

	return result;
}

bool MWinButtonImpl::WMGetDlgCode(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	outResult = DLGC_BUTTON;

	if (mDefault)
		outResult |= DLGC_DEFPUSHBUTTON;

	return true;
}

bool MWinButtonImpl::BCMSetDropDownState(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	Button_SetDropDownState(GetHandle(), true);
	
	RECT r;
	::GetWindowRect(GetHandle(), &r);
	
	mControl->eDropDown(mControl->GetID(), r.left, r.bottom);
	Button_SetDropDownState(GetHandle(), false);

	return true;
}

void MWinButtonImpl::SimulateClick()
{
	::SendMessage(GetHandle(), BM_SETSTATE, 1, 0);
	::UpdateWindow(GetHandle());
	::delay(12.0 / 60.0);
	::SendMessage(GetHandle(), BM_SETSTATE, 0, 0);
	::UpdateWindow(GetHandle());

	mControl->eClicked(mControl->GetID());
}

void MWinButtonImpl::MakeDefault(bool inDefault)
{
	mDefault = inDefault;

	if (GetHandle() != nullptr)
	{
		if (inDefault)
			::SendMessage(GetHandle(), BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, 0);
		else
			::SendMessage(GetHandle(), BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, 0);
		::UpdateWindow(GetHandle());
	}
}

void MWinButtonImpl::SetText(const std::string& inText)
{
	wstring text(c2w(inText));
	::SendMessage(GetHandle(), WM_SETTEXT, 0, (LPARAM)text.c_str());
}

void MWinButtonImpl::AddedToWindow()
{
	MWinControlImpl::AddedToWindow();
	if (mDefault)
		::SendMessage(GetHandle(), BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, 0);
	else
		::SendMessage(GetHandle(), BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, 0);
}

void MWinButtonImpl::GetIdealSize(int32_t& outWidth, int32_t& outHeight)
{
	outWidth = 75;
	outHeight = 23;

	SIZE size;
	if (GetHandle() != nullptr and Button_GetIdealSize(GetHandle(), &size))
	{
		if (outWidth < size.cx + 20)
			outWidth = size.cx + 20;

		if (outHeight < size.cy + 2)
			outHeight = size.cy + 2;
	}
}

MButtonImpl* MButtonImpl::Create(MButton* inButton, const string& inLabel,
	MButtonFlags inFlags)
{
	return new MWinButtonImpl(inButton, inLabel, inFlags);
}

// --------------------------------------------------------------------

MWinExpanderImpl::MWinExpanderImpl(MExpander* inExpander, const string& inLabel)
	: MWinControlImpl(inExpander, inLabel)
	, mIsOpen(false)
	, mMouseInside(false)
	, mMouseDown(false)
	, mMouseTracking(false)
	, mLastExit(0)
	, mDC(nullptr)
{
	using namespace std::placeholders;

	AddHandler(WM_PAINT,			std::bind(&MWinExpanderImpl::WMPaint, this, _1, _2, _3, _4, _5));
	AddHandler(WM_ACTIVATE,			std::bind(&MWinExpanderImpl::WMActivate, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONDOWN,		std::bind(&MWinExpanderImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONDBLCLK,	std::bind(&MWinExpanderImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONUP,		std::bind(&MWinExpanderImpl::WMMouseUp, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSEMOVE,		std::bind(&MWinExpanderImpl::WMMouseMove, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSELEAVE,		std::bind(&MWinExpanderImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
	AddHandler(WM_CAPTURECHANGED,	std::bind(&MWinExpanderImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
}

MWinExpanderImpl::~MWinExpanderImpl()
{
	if (mDC)
		::ReleaseDC(GetHandle(), mDC);
}

void MWinExpanderImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outStyle = WS_CHILD | WS_TABSTOP;
	outClassName = L"my-expander";
}

void MWinExpanderImpl::SetOpen(bool inOpen)
{
	if (inOpen != mIsOpen)
	{
		mIsOpen = inOpen;
		mControl->Invalidate();
		mControl->eClicked(mControl->GetID());
	}
}

bool MWinExpanderImpl::IsOpen() const
{
	return mIsOpen;
}

bool MWinExpanderImpl::WMActivate(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
{
	mControl->Invalidate();
	return false;
}

bool MWinExpanderImpl::WMPaint(HWND inHWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	HTHEME hTheme = ::OpenThemeData(inHWnd, VSCLASS_TASKDIALOG);
	if (hTheme != nullptr)
	{
		PAINTSTRUCT lPs;
		HDC hdc = ::BeginPaint(inHWnd, &lPs);
		THROW_IF_NIL(hdc);
		
//		::GetUpdateRect(inHWnd, &lPs.rcPaint, false);
		
		if (::IsThemeBackgroundPartiallyTransparent(hTheme, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL))
			::DrawThemeParentBackground(inHWnd, hdc, &lPs.rcPaint);

		RECT clientRect;
		::GetClientRect(inHWnd, &clientRect);

		RECT contentRect;
		::GetThemeBackgroundContentRect(hTheme, hdc, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, &clientRect, &contentRect);
		
		int w = contentRect.bottom - contentRect.top;
		::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_WIDTH, &w);
		int sr = contentRect.right;
		contentRect.right = contentRect.left + w;
		
		int state = TDLGEBS_NORMAL;
		if ((mMouseInside and not mMouseDown) or (not mMouseInside and mMouseDown))
			state = TDLGEBS_HOVER;
		else if (mMouseDown)
			state = TDLGEBS_PRESSED;

		if (mIsOpen)
			state += 3;		// expanded

		::DrawThemeBackground(hTheme, hdc, TDLG_EXPANDOBUTTON, state, &contentRect, 0);

	    ::CloseThemeData(hTheme);
	    hTheme = ::OpenThemeData(inHWnd, VSCLASS_TEXTSTYLE);
	    if (hTheme != nullptr)
	    {
			contentRect.left = contentRect.right;
			contentRect.right = sr;
			
			wstring label(c2w(mLabel));
	
			RECT r;
			::GetThemeTextExtent(hTheme, mDC, TEXT_BODYTEXT, 0, label.c_str(), label.length(), 0, nullptr, &r);
			contentRect.left += (r.bottom - r.top) / 2;
			contentRect.top += ((contentRect.bottom - contentRect.top) - (r.bottom - r.top)) / 2;

			int state = TS_CONTROLLABEL_NORMAL;
			if (not ::IsWindowEnabled(inHWnd))
				state = TS_CONTROLLABEL_DISABLED;
			
			::DrawThemeText(hTheme, hdc, TEXT_BODYTEXT, state, label.c_str(), label.length(),
				0, 0, &contentRect);

			::CloseThemeData(hTheme);
	    }

		::EndPaint (inHWnd, &lPs);

	    result = true;
	}

	return result;
}

bool MWinExpanderImpl::WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	::SetFocus(inHWnd);
	::SetCapture(inHWnd);

	mMouseInside = true;
	mMouseDown = true;
	mControl->Invalidate();
	mControl->UpdateNow();

	return true;
}

bool MWinExpanderImpl::WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	if (not mMouseTracking)
	{
		TRACKMOUSEEVENT me = { sizeof(TRACKMOUSEEVENT) };
		me.dwFlags = TME_LEAVE;
		me.hwndTrack = GetHandle();

		if (not mMouseDown)
			me.dwHoverTime = HOVER_DEFAULT;

		if (::TrackMouseEvent(&me))
			mMouseTracking = true;

		mControl->Invalidate();
		mControl->UpdateNow();
	}

	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));
	
	MRect bounds;
	mControl->GetBounds(bounds);
	
	if (mMouseInside != bounds.ContainsPoint(x, y))
	{
		mMouseInside = not mMouseInside;
		mControl->Invalidate();
		mControl->UpdateNow();
	}

	return true;
}

bool MWinExpanderImpl::WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	mMouseInside = false;
	mMouseTracking = false;
	mLastExit = GetLocalTime();
	
	mControl->Invalidate();
	mControl->UpdateNow();
	
	return true;
}

bool MWinExpanderImpl::WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	::ReleaseCapture();
	
	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));
	
	MRect bounds;
	mControl->GetBounds(bounds);
	mMouseInside = bounds.ContainsPoint(x, y);

	if (mMouseInside)
	{
		mIsOpen = not mIsOpen;
		mControl->eClicked(mControl->GetID());
	}
	mMouseDown = false;

	mControl->Invalidate();
	mControl->UpdateNow();

	return true;
}

void MWinExpanderImpl::AddedToWindow()
{
	MWinControlImpl::AddedToWindow();
	
	mDC = ::GetDC(GetHandle());
	
	HTHEME hTheme = ::OpenThemeData(GetHandle(), VSCLASS_TASKDIALOG);
	if (hTheme != nullptr)
	{
		int w, h;
		RECT r;

		wstring label(c2w(mLabel));
		
		if (::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_WIDTH, &w) == S_OK and
			::GetThemeMetric(hTheme, nullptr, TDLG_EXPANDOBUTTON, TDLGEBS_NORMAL, TMT_HEIGHT, &h) == S_OK and
			::GetThemeTextExtent(hTheme, mDC, TDLG_EXPANDOTEXT, 0, label.c_str(), label.length(), 0, nullptr, &r) == S_OK)
		{
			MWinProcMixin* parent;
			MRect bounds;
			
			GetParentAndBounds(parent, bounds);

			int lw = r.right - r.left + (r.bottom - r.top) / 2;
			mControl->ResizeFrame(w + lw - bounds.width, h - bounds.height);
		}
	}
}

MExpanderImpl* MExpanderImpl::Create(MExpander* inExpander, const string& inLabel)
{
	return new MWinExpanderImpl(inExpander, inLabel);
}

// --------------------------------------------------------------------

MWinScrollbarImpl::MWinScrollbarImpl(MScrollbar* inScrollbar)
	: MWinControlImpl(inScrollbar, "")
{
}

void MWinScrollbarImpl::ShowSelf()
{
	::ShowScrollBar(GetHandle(), SB_CTL, TRUE);
}

void MWinScrollbarImpl::HideSelf()
{
	::ShowScrollBar(GetHandle(), SB_CTL, FALSE);
}

void MWinScrollbarImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = L"SCROLLBAR";
	outStyle = WS_CHILD;

	MRect bounds;
	mControl->GetBounds(bounds);

	if (bounds.width > bounds.height)
		outStyle |= SBS_HORZ;
	else
		outStyle |= SBS_VERT;
}

int32_t MWinScrollbarImpl::GetValue() const
{
	int32_t result = 0;

	SCROLLINFO info = { sizeof(SCROLLINFO), SIF_POS };
	
	if (GetHandle() != nullptr and ::GetScrollInfo(GetHandle(), SB_CTL, &info))
		result = info.nPos;

	return result;
}

void MWinScrollbarImpl::SetValue(int32_t inValue)
{
	if (GetHandle() != nullptr)
	{
		SCROLLINFO info = { sizeof(SCROLLINFO), SIF_POS };
		info.nPos = inValue;
		
		::SetScrollInfo(GetHandle(), SB_CTL, &info, true);
	}
}

int32_t MWinScrollbarImpl::GetTrackValue() const
{
	SCROLLINFO info = { sizeof(SCROLLINFO), SIF_TRACKPOS };
	
	if (GetHandle() != nullptr)
		::GetScrollInfo(GetHandle(), SB_CTL, &info);

	return info.nTrackPos;
}

void MWinScrollbarImpl::SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
	int32_t inScrollUnit, int32_t inPageSize, int32_t inValue)
{
	if (GetHandle() != nullptr)
	{
		SCROLLINFO info = { sizeof(SCROLLINFO), SIF_ALL };
		info.nMin = inMinValue;
		info.nMax = inMaxValue;
		info.nPage = inPageSize;
		info.nPos = inValue;
		::SetScrollInfo(GetHandle(), SB_CTL, &info, true);
	}
}

int32_t MWinScrollbarImpl::GetMinValue() const
{
	SCROLLINFO info = { sizeof(SCROLLINFO), SIF_RANGE };
	
	if (GetHandle() != nullptr)
		::GetScrollInfo(GetHandle(), SB_CTL, &info);

	return info.nMin;
}

int32_t MWinScrollbarImpl::GetMaxValue() const
{
	SCROLLINFO info = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE };
	if (GetHandle() != nullptr)
		::GetScrollInfo(GetHandle(), SB_CTL, &info);

	int32_t result = info.nMax;
	if (info.nPage > 1)
		result -= info.nPage - 1;

	return result;
}

bool MWinScrollbarImpl::WMScroll(HWND inHandle, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	MScrollbar* scrollbar = dynamic_cast<MScrollbar*>(mControl);

	if (scrollbar != nullptr and (inUMsg == WM_HSCROLL or inUMsg == WM_VSCROLL))
	{
		outResult = 0;
		result = true;

		switch (LOWORD(inWParam))
		{
			case SB_LINEDOWN:
				scrollbar->eScroll(kScrollLineDown);
				break;
			case SB_LINEUP:
				scrollbar->eScroll(kScrollLineUp);
				break;
			case SB_PAGEDOWN:
				scrollbar->eScroll(kScrollPageDown);
				break;
			case SB_PAGEUP:
				scrollbar->eScroll(kScrollPageUp);
				break;
			case SB_THUMBTRACK:
			{
				SCROLLINFO info = { sizeof(info), SIF_TRACKPOS };
				::GetScrollInfo(GetHandle(), SB_CTL, &info);
				SetValue(info.nTrackPos);
				scrollbar->eScroll(kScrollToThumb);
				break;
			}
		}
//		mControl->GetWindow()->UpdateNow();
	}

	return result;
}

MScrollbarImpl* MScrollbarImpl::Create(MScrollbar* inScrollbar)
{
	return new MWinScrollbarImpl(inScrollbar);
}

// --------------------------------------------------------------------

MWinStatusbarImpl::MWinStatusbarImpl(MStatusbar* inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
	: MWinControlImpl(inStatusbar, "")
{
	int32_t offset = 0;
	for (uint32_t p = 0; p < inPartCount; ++p)
	{
		if (inParts[p].expand)
			mOffsets.push_back(-1);
		else
		{
			offset += inParts[p].width;
			mOffsets.push_back(offset);
		}
	}
}

void MWinStatusbarImpl::CreateParams(
	DWORD& outStyle, DWORD& outExStyle, wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outClassName = STATUSCLASSNAME;
	outStyle = WS_CHILD | WS_VISIBLE | /*SBARS_SIZEGRIP |*/ CCS_NOPARENTALIGN;
	outExStyle = 0;//WS_EX_CLIENTEDGE;
}

void MWinStatusbarImpl::CreateHandle(MWinProcMixin* inParent,
	MRect inBounds, const wstring& inTitle)
{
	using namespace std::placeholders;

	MWinControlImpl::CreateHandle(inParent, inBounds, inTitle);
	
	if (inParent != nullptr)
	{
		inParent->AddNotify(NM_CLICK, GetHandle(),
			std::bind(&MWinStatusbarImpl::NMClick, this, _1, _2, _3));
	}
}

void MWinStatusbarImpl::AddedToWindow()
{
	MWinControlImpl::AddedToWindow();

	::SendMessageW(GetHandle(), SB_SETPARTS, mOffsets.size(), (LPARAM)&mOffsets[0]);
}

void MWinStatusbarImpl::SetStatusText(uint32_t inPartNr, const string& inText, bool inBorder)
{
	if (inPartNr >= 0 and inPartNr < mOffsets.size())
	{
		if (inBorder == false)
			inPartNr |= SBT_NOBORDERS;
	
		wstring text(c2w(inText));
		::SendMessageW(GetHandle(), SB_SETTEXT, inPartNr, (LPARAM)text.c_str());
		::UpdateWindow(GetHandle());
	}
}

bool MWinStatusbarImpl::NMClick(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	NMMOUSE* nmMouse = reinterpret_cast<NMMOUSE*>(inLParam);
	
	uint32_t partNr = nmMouse->dwItemSpec;
	
	RECT r;
	::SendMessageW(GetHandle(), SB_GETRECT, partNr, (LPARAM)&r);
	::MapWindowPoints(GetHandle(), HWND_DESKTOP, (LPPOINT)&r, 2);
	
	mControl->ePartClicked(partNr, MRect(r.left, r.top, r.right - r.left, r.bottom - r.top));
	
	return true;
}

MStatusbarImpl* MStatusbarImpl::Create(MStatusbar* inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
{
	return new MWinStatusbarImpl(inStatusbar, inPartCount, inParts);
}

// --------------------------------------------------------------------

MWinComboboxImpl::MWinComboboxImpl(MCombobox* inCombobox)
	: MWinControlImpl(inCombobox, "")
	, mEditor(this)
{
}

void MWinComboboxImpl::SetChoices(const std::vector<std::string>& inChoices)
{
	mChoices = inChoices;
	if (GetHandle() != nullptr)
	{
		::SendMessage(GetHandle(), CB_RESETCONTENT, 0, 0);

		for (const string& choice: inChoices)
		{
			wstring s(c2w(choice));

			::SendMessage(GetHandle(), CB_INSERTSTRING, (WPARAM)-1, (long)s.c_str());
		}

		if (not inChoices.empty())
			SetText(inChoices.front());
		
		::UpdateWindow(GetHandle());
	}
}

void MWinComboboxImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = L"COMBOBOX";
	outStyle = WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL | WS_TABSTOP;
}

void MWinComboboxImpl::AddedToWindow()
{
	MWinControlImpl::AddedToWindow();
	
	HWND edit = ::GetWindow(GetHandle(), GW_CHILD);
	THROW_IF_NIL(edit);

	mEditor.SetHandle(edit);
	mEditor.SubClass();

	if (not mChoices.empty())
		SetChoices(mChoices);
}

bool MWinComboboxImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = false;

	if (inKeyCode == kReturnKeyCode or
		inKeyCode == kEnterKeyCode or
		inKeyCode == kTabKeyCode or
		inKeyCode == kEscapeKeyCode or
		(inModifiers & ~kShiftKey) != 0)
	{
		result = MWinControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
	}

	return result;
}

bool MWinComboboxImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = true;
	
	switch (inUMsg)
	{
//		case CBN_SELENDOK:
//			mControl->eValueChanged(mControl->GetID(), GetText());
//			break;
		
		case CBN_EDITUPDATE:
			mControl->eValueChanged(mControl->GetID(), GetText());
			break;
		
		case CBN_SELCHANGE:
		{
			uint32_t selected = ::SendMessage(GetHandle(), CB_GETCURSEL, 0, 0);
			if (selected < mChoices.size())
				mControl->eValueChanged(mControl->GetID(), mChoices[selected]);
			break;
		}
		
		case CBN_DROPDOWN:
		{
			int count = ::SendMessage(GetHandle(), CB_GETCOUNT, 0, 0) + 1;
			if (count < 1) count = 1;
			if (count > 8) count = 8;
			
			MRect bounds;
			mControl->GetBounds(bounds);
			
			int itemHeight = ::SendMessage(GetHandle(), CB_GETITEMHEIGHT, 0, 0);
			::SetWindowPos(GetHandle(), 0, 0, 0, bounds.width,
				count * itemHeight + bounds.height + 2,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_HIDEWINDOW);
			::SetWindowPos(GetHandle(), 0, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_SHOWWINDOW);
			break;
		}
		
		default:
			result = false;
			break;
	}
	
	return result;
}

bool MWinComboboxImpl::WMMouseWheel(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult)
{
	return false;
}

MComboboxImpl* MComboboxImpl::Create(MCombobox* inCombobox)
{
	return new MWinComboboxImpl(inCombobox);
}

// --------------------------------------------------------------------

MWinPopupImpl::MWinPopupImpl(MPopup* inPopup)
	: MWinControlImpl(inPopup, "")
{
}

void MWinPopupImpl::SetChoices(const std::vector<std::string>& inChoices)
{
	if (GetHandle() == nullptr)
		mChoices = inChoices;
	else
	{
		::SendMessage(GetHandle(), CB_RESETCONTENT, 0, 0);

		for (const string& choice: inChoices)
		{
			wstring s(c2w(choice));

			::SendMessage(GetHandle(), CB_INSERTSTRING, (WPARAM)-1, (long)s.c_str());
		}

		if (not inChoices.empty())
			SetValue(0);
		
		::UpdateWindow(GetHandle());
	}
}

void MWinPopupImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = L"COMBOBOX";
	outStyle = WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP;
}

void MWinPopupImpl::AddedToWindow()
{
	MWinControlImpl::AddedToWindow();
	
//	AddHandler(WM_MOUSEWHEEL, std::bind(&MWinPopupImpl::WMMouseWheel, this, _1, _2, _3, _4, _5));

	if (not mChoices.empty())
		SetChoices(mChoices);
}

int32_t MWinPopupImpl::GetValue() const
{
	return ::SendMessage(GetHandle(), CB_GETCURSEL, 0, 0) + 1;
}

void MWinPopupImpl::SetValue(int32_t inValue)
{
	::SendMessage(GetHandle(), CB_SETCURSEL, (WPARAM)(inValue - 1), 0);
	::UpdateWindow(GetHandle());
}

void MWinPopupImpl::SetText(const string& inText)
{
	wstring s(c2w(inText));	
	int r = ::SendMessageW(GetHandle(), CB_FINDSTRINGEXACT, (WPARAM)(-1), (LPARAM)(s.c_str()));
	if (r == CB_ERR)
		THROW(("Could not find text in popup menu"));
	::SendMessage(GetHandle(), CB_SETCURSEL, (WPARAM)r, 0);
	::UpdateWindow(GetHandle());
}

bool MWinPopupImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = false;

	if (inKeyCode == kReturnKeyCode or
		inKeyCode == kEnterKeyCode or
		inKeyCode == kTabKeyCode or
		inKeyCode == kEscapeKeyCode or
		(inModifiers & ~kShiftKey) != 0)
	{
		result = MWinControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
	}

	return result;
}

bool MWinPopupImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = true;
	
	switch (inUMsg)
	{
		case CBN_SELENDOK:
			mControl->eValueChanged(mControl->GetID(), GetValue());
			break;
		
//		case CBN_EDITUPDATE:
//			mControl->eValueChanged(mControl->GetID(), GetValue());
//			break;
		
		case CBN_DROPDOWN:
		{
			int count = ::SendMessage(GetHandle(), CB_GETCOUNT, 0, 0) + 1;
			if (count < 1) count = 1;
			if (count > 8) count = 8;
			
			MRect bounds;
			mControl->GetBounds(bounds);
			
			int itemHeight = ::SendMessage(GetHandle(), CB_GETITEMHEIGHT, 0, 0);
			::SetWindowPos(GetHandle(), 0, 0, 0, bounds.width,
				count * itemHeight + bounds.height + 2,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_HIDEWINDOW);
			::SetWindowPos(GetHandle(), 0, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_SHOWWINDOW);
			break;
		}
		
		default:
			result = false;
			break;
	}
	
	return result;
}

bool MWinPopupImpl::WMMouseWheel(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult)
{
	return false;
}

MPopupImpl* MPopupImpl::Create(MPopup* inPopup)
{
	return new MWinPopupImpl(inPopup);
}

// --------------------------------------------------------------------

MWinEdittextImpl::MWinEdittextImpl(MEdittext* inEdittext, uint32_t inFlags)
	: MWinControlImpl(inEdittext, "")
	, mFlags(inFlags)
{
}

void MWinEdittextImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outClassName = L"EDIT";
	outStyle = WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL;
	outExStyle = WS_EX_CLIENTEDGE;

	if (mFlags & eMEditTextAlignRight)
		outStyle |= ES_RIGHT;
	
	if (mFlags & eMEditTextNumbers)
		outStyle |= ES_NUMBER | ES_RIGHT;

	if (mFlags & eMEditTextMultiLine)
		outStyle |= ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL;
	
	if (mFlags & eMEditTextReadOnly)
		outStyle |= ES_READONLY;

//	else if (fPassword)
//		ioParams.fStyle |= ES_PASSWORD;
}

void MWinEdittextImpl::SetFocus()
{
	MWinControlImpl::SetFocus();
	
	::SendMessage(GetHandle(), EM_SETSEL, 0, -1);
}

string MWinEdittextImpl::GetText() const
{
	int l = ::SendMessage(GetHandle(), WM_GETTEXTLENGTH, 0, 0);
	vector<wchar_t> buffer(l + 1);
	l = ::SendMessage(GetHandle(), WM_GETTEXT, (WPARAM)(l + 1), (LPARAM)&buffer[0]);

	string text(w2c(&buffer[0]));
	ba::replace_all(text, "\r\n", "\n");
	return text;
}

void MWinEdittextImpl::SetText(const std::string& inText)
{
	if (inText != GetText())
	{
		wstring text(c2w(inText));
		ba::replace_all(text, L"\n", L"\r\n");
		::SendMessage(GetHandle(), WM_SETTEXT, 0, (LPARAM)text.c_str());
		::UpdateWindow(GetHandle());
	}
}

void MWinEdittextImpl::SetPasswordChar(uint32_t inUnicode)
{
	::SendMessage(GetHandle(), EM_SETPASSWORDCHAR, (WPARAM)inUnicode, 0);
}

bool MWinEdittextImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = false;

	if (inKeyCode == kReturnKeyCode or
		inKeyCode == kEnterKeyCode or
		inKeyCode == kTabKeyCode or
		inKeyCode == kEscapeKeyCode or
		(inModifiers & ~kShiftKey) != 0)
	{
		result = MWinControlImpl::DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
	}

	return result;
}

bool MWinEdittextImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = true;
	
	switch (inUMsg)
	{
		case EN_CHANGE:
			mControl->eValueChanged(mControl->GetID(), GetText());
			break;
		
		default:
			result = false;
			break;
	}
	
	return result;
}

MEdittextImpl* MEdittextImpl::Create(MEdittext* inEdittext, uint32_t inFlags)
{
	return new MWinEdittextImpl(inEdittext, inFlags);
}

// --------------------------------------------------------------------

MWinCaptionImpl::MWinCaptionImpl(MCaption* inControl, const string& inText)
	: MWinControlImpl(inControl, inText)
	, mText(c2w(inText))
{
}

void MWinCaptionImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = L"STATIC";
	outStyle = WS_CHILD /*| SS_OWNERDRAW*/;
}

void MWinCaptionImpl::SetText(const string& inText)
{
	wstring s(c2w(inText));
	::SetWindowTextW(GetHandle(), s.c_str());
//	mText = c2w(inText);
//	
//	RECT r;
//	::GetClientRect(GetHandle(), &r);
//	::InvalidateRect(GetHandle(), &r, false);
//	::UpdateWindow(GetHandle());
}

bool MWinCaptionImpl::WMDrawItem(DRAWITEMSTRUCT* inDrawItemStruct)
{
//	return false;
	HTHEME hTheme = ::OpenThemeData(inDrawItemStruct->hwndItem, VSCLASS_TEXTSTYLE);
	if (hTheme != nullptr)
	{
		RECT r;
		::GetClientRect(GetHandle(), &r);
		
		int state = 0; //TS_CONTROLLABEL_NORMAL;
//		//if (inDrawItemStruct->itemState & ODS_DISABLED)
//		//	state = TS_CONTROLLABEL_DISABLED;
//
//		DTTOPTS opts = { sizeof(DTTOPTS), DTT_TEXTCOLOR };
//		opts.crText = RGB(0, 0, 0);		// sjeez... really?
//
//		::DrawThemeParentBackground(GetHandle(), inDrawItemStruct->hDC, &r);
//		::DrawThemeTextEx(hTheme, inDrawItemStruct->hDC,
//			TEXT_CONTROLLABEL, state, mText.c_str(), mText.length(), 0, &r, &opts);
//

		::DrawThemeText(hTheme, inDrawItemStruct->hDC,
			TEXT_BODYTEXT, state, mText.c_str(), mText.length(), DT_SINGLELINE, 0, &r);

		::CloseThemeData(hTheme);
	}
	
	return true;
}

MCaptionImpl* MCaptionImpl::Create(MCaption* inCaption, const std::string& inText)
{
	return new MWinCaptionImpl(inCaption, inText);
}

// --------------------------------------------------------------------

MWinSeparatorImpl::MWinSeparatorImpl(MSeparator* inControl)
	: MWinControlImpl(inControl, "")
{
}

void MWinSeparatorImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = L"STATIC";
	outStyle = WS_CHILD | SS_SUNKEN;
}

MSeparatorImpl* MSeparatorImpl::Create(MSeparator* inSeparator)
{
	return new MWinSeparatorImpl(inSeparator);
}

// --------------------------------------------------------------------

MWinCheckboxImpl::MWinCheckboxImpl(MCheckbox* inControl, const string& inText)
	: MWinControlImpl(inControl, inText)
{
}

void MWinCheckboxImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = L"BUTTON";
	outStyle = WS_CHILD | BS_CHECKBOX | BS_TEXT | WS_TABSTOP;
}

bool MWinCheckboxImpl::IsChecked() const
{
	return ::SendMessage(GetHandle(), BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void MWinCheckboxImpl::SetChecked(bool inChecked)
{
	if (inChecked)
		::SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	else
		::SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	::UpdateWindow(GetHandle());
}

bool MWinCheckboxImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	if (inUMsg == BN_CLICKED)
	{
		bool checked = not IsChecked();

		SetChecked(checked);
		mControl->eValueChanged(mControl->GetID(), checked);

		outResult = 1;
		result = true;
	}

	return result;
}

MCheckboxImpl* MCheckboxImpl::Create(MCheckbox* inCheckbox, const string& inText)
{
	return new MWinCheckboxImpl(inCheckbox, inText);
}

// --------------------------------------------------------------------

MWinRadiobuttonImpl::MWinRadiobuttonImpl(MRadiobutton* inControl, const string& inText)
	: MWinControlImpl(inControl, inText)
{
}

void MWinRadiobuttonImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = L"BUTTON";
	outStyle = WS_CHILD | BS_RADIOBUTTON | BS_TEXT | WS_TABSTOP;
}

bool MWinRadiobuttonImpl::IsChecked() const
{
	return ::SendMessage(GetHandle(), BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void MWinRadiobuttonImpl::SetChecked(bool inChecked)
{
	if (inChecked)
	{
		::SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		
		for (MRadiobutton* button: mGroup)
		{
			if (button == mControl)
				continue;
			
			button->SetChecked(false);
		}
	}
	else
		::SendMessage(GetHandle(), BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

	::UpdateWindow(GetHandle());
}

void MWinRadiobuttonImpl::SetGroup(const list<MRadiobutton*>& inButtons)
{
	mGroup = inButtons;
}

bool MWinRadiobuttonImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	if (inUMsg == BN_CLICKED)
	{
		bool checked = not IsChecked();

		SetChecked(checked);
		mControl->eValueChanged(mControl->GetID(), checked);

		outResult = 1;
		result = true;
	}

	return result;
}

MRadiobuttonImpl* MRadiobuttonImpl::Create(MRadiobutton* inRadiobutton, const std::string& inText)
{
	return new MWinRadiobuttonImpl(inRadiobutton, inText);
}

// --------------------------------------------------------------------

MWinListHeaderImpl::MWinListHeaderImpl(MListHeader* inListHeader)
	: MWinControlImpl(inListHeader, "")
{
}

void MWinListHeaderImpl::CreateHandle(MWinProcMixin* inParent, MRect inBounds,
	const wstring& inTitle)
{
	using namespace std::placeholders;

	MWinControlImpl::CreateHandle(inParent, inBounds, inTitle);

	if (inParent != nullptr)
	{
        // Retrieve the bounding rectangle of the parent window's 
        // client area, and then request size and position values 
        // from the header control. 

//		MRect bounds;
//		mControl->GetBounds(bounds);
//		mControl->ConvertToWindow(bounds.x, bounds.y);

		RECT rc = { inBounds.x, inBounds.y, inBounds.x + inBounds.width, inBounds.y + inBounds.height };
 
        HDLAYOUT hdl;
        WINDOWPOS wp; 

		hdl.prc = &rc;
        hdl.pwpos = &wp; 

		if (::SendMessage(GetHandle(), HDM_LAYOUT, 0, (LPARAM) &hdl))
		{
			// Set the size, position, and visibility of the header control.
			::SetWindowPos(GetHandle(), wp.hwndInsertAfter, wp.x, wp.y, 
				wp.cx, wp.cy, wp.flags | SWP_SHOWWINDOW);
			
			MRect frame;
			mControl->GetFrame(frame);
			frame.width = wp.cx;
			frame.height = wp.cy;
			mControl->SetFrame(frame);
		}

		inParent->AddNotify(HDN_TRACK, GetHandle(),
			std::bind(&MWinListHeaderImpl::HDNTrack, this, _1, _2, _3));
		inParent->AddNotify(HDN_BEGINTRACK, GetHandle(),
			std::bind(&MWinListHeaderImpl::HDNBeginTrack, this, _1, _2, _3));
		inParent->AddNotify(HDN_BEGINDRAG, GetHandle(),
			std::bind(&MWinListHeaderImpl::HDNBeginDrag, this, _1, _2, _3));
	}
}

void MWinListHeaderImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outStyle = WS_CHILD | HDS_HORZ;
	outClassName = WC_HEADER;
}

void MWinListHeaderImpl::AppendColumn(const string& inLabel, int inWidth)
{
	HDITEM item = {};
	
	wstring label = c2w(inLabel);
	
	item.mask = HDI_FORMAT | HDI_TEXT;
	item.pszText = const_cast<wchar_t*>(label.c_str());
	item.cchTextMax = label.length();
	item.fmt = HDF_LEFT | HDF_STRING;

	if (inWidth > 0)
	{
		item.mask |= HDI_WIDTH;
		item.cxy = inWidth;
	}
	
	int insertAfter = ::SendMessage(GetHandle(), HDM_GETITEMCOUNT, 0, 0);
	::SendMessage(GetHandle(), HDM_INSERTITEM, (WPARAM)&insertAfter, (LPARAM)&item);
}

bool MWinListHeaderImpl::HDNBeginTrack(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	outResult = 1;
	return true;
}

bool MWinListHeaderImpl::HDNBeginDrag(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	outResult = 1;
	return true;
}

bool MWinListHeaderImpl::HDNTrack(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	NMHEADER& nmh = *(NMHEADER*)inLParam;

	mControl->eColumnResized(nmh.iItem, nmh.pitem->cxy);

	return true;
}

MListHeaderImpl* MListHeaderImpl::Create(MListHeader* inListHeader)
{
	return new MWinListHeaderImpl(inListHeader);
}

// --------------------------------------------------------------------

MWinNotebookImpl::MWinNotebookImpl(MNotebook* inControl)
	: MWinControlImpl(inControl, "")
{
}

void MWinNotebookImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outClassName = WC_TABCONTROL;
	outStyle = WS_CHILD | /*BS_Notebook | BS_TEXT | */WS_TABSTOP;
}

void MWinNotebookImpl::AddedToWindow()
{
	MWinControlImpl::AddedToWindow();

	// add pages
	if (not mPages.empty())
	{
		vector<MPage> pages(mPages);
		mPages.clear();
		
		for (MPage& page: pages)
			AddPage(page.mTitle, page.mPage);
		
		FrameResized();
	}

	MRect bounds;
	MWinProcMixin* parent;

	GetParentAndBounds(parent, bounds);
	
	parent->AddNotify(TCN_SELCHANGE, GetHandle(),
		std::bind(&MWinNotebookImpl::TCNSelChange, this, _1, _2, _3));
}

void MWinNotebookImpl::FrameResized()
{
	MWinControlImpl<MNotebook>::FrameResized();

	RECT rc;
	::GetClientRect(GetHandle(), &rc);
	TabCtrl_AdjustRect(GetHandle(), false, &rc);

	for (MPage& page: mPages)
	{
		MRect frame;
		page.mPage->GetFrame(frame);

		int32_t dx, dy, dw, dh;
		dx = rc.left - frame.x;
		dy = rc.top - frame.y;
		dw = rc.right - rc.left - frame.width;
		dh = rc.bottom - rc.top - frame.height;

		page.mPage->MoveFrame(dx, dy);
		page.mPage->ResizeFrame(dw, dh);
	}
}

void MWinNotebookImpl::AddPage(const string& inLabel, MView* inPage)
{
	MPage page = { inLabel, inPage };
	mPages.push_back(page);
	
	if (GetHandle())
	{
		wstring s(c2w(inLabel));
		
		TCITEM tci = {};
		tci.mask = TCIF_TEXT;
		tci.pszText = const_cast<wchar_t*>(s.c_str());
		
		::SendMessage(GetHandle(), TCM_INSERTITEM, (WPARAM)mPages.size(), (LPARAM)&tci);

		RECT r;
		::GetClientRect(GetHandle(), &r);
		TabCtrl_AdjustRect(GetHandle(), false, &r);

		MRect frame;
		inPage->GetFrame(frame);

		if (frame.x != r.left or frame.y != r.top)
			inPage->MoveFrame(r.left - frame.x, r.top - frame.y);

		if (frame.height != r.bottom - r.top or
			frame.width != r.right - r.left)
		{
			inPage->ResizeFrame((r.right - r.left) - frame.width, (r.bottom - r.top) - frame.height);
		}

		mControl->AddChild(inPage);
		
		if (mPages.size() > 1)
			inPage->Hide();
	}
}

void MWinNotebookImpl::SelectPage(uint32_t inPage)
{
	if (inPage != ::SendMessage(GetHandle(), TCM_GETCURSEL, 0, 0))
	{
		::SendMessage(GetHandle(), TCM_SETCURSEL, (WPARAM)inPage, 0);
		::UpdateWindow(GetHandle());
	}
	
	if (inPage < mPages.size())
	{
		for (MPage& page: mPages)
		{
			page.mPage->Hide();
			page.mPage->Disable();
		}
		mPages[inPage].mPage->Show();
		mPages[inPage].mPage->Enable();
		mControl->ePageSelected(inPage);
	}
}

uint32_t MWinNotebookImpl::GetSelectedPage() const
{
	return ::SendMessage(GetHandle(), TCM_GETCURSEL, 0, 0);
}

bool MWinNotebookImpl::TCNSelChange(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	SelectPage(::SendMessage(GetHandle(), TCM_GETCURSEL, 0, 0));


	outResult = 0;
	return true;
}

MNotebookImpl* MNotebookImpl::Create(MNotebook* inNotebook)
{
	return new MWinNotebookImpl(inNotebook);
}

// --------------------------------------------------------------------

MWinColorSwatchImpl::MWinColorSwatchImpl(MColorSwatch* inColorSwatch, MColor inColor)
	: MWinControlImpl(inColorSwatch, "")
	, eSelectedColor(this, &MWinColorSwatchImpl::SelectedColor)
	, ePreviewColor(this, &MWinColorSwatchImpl::PreviewColor)
	, mColor(inColor)
{
}

void MWinColorSwatchImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outStyle = WS_CHILD | BS_PUSHBUTTON | BS_BITMAP | WS_TABSTOP;
	
	outClassName = L"BUTTON";
}

bool MWinColorSwatchImpl::WMCommand(HWND inHWnd, UINT inMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	if (inMsg == BN_CLICKED)
	{
		MColorPicker* dlog = new MColorPicker(mControl->GetWindow(), mColor);
		AddRoute(dlog->eSelectedColor, eSelectedColor);
		AddRoute(dlog->eChangedColor, ePreviewColor);

		outResult = 1;
		result = true;
	}

	return result;
}

void MWinColorSwatchImpl::SelectedColor(MColor inColor)
{
	SetColor(inColor);
	mControl->eColorChanged(mControl->GetID(), mColor);
}

void MWinColorSwatchImpl::PreviewColor(MColor inColor)
{
	mControl->eColorPreview(mControl->GetID(), inColor);
}

MColor MWinColorSwatchImpl::GetColor() const
{
	return mColor;
}

void MWinColorSwatchImpl::SetColor(MColor inColor)
{
	mColor = inColor;

	if (GetHandle())
	{
		RECT r = { 0, 0, 20, 14 };
		::GetClientRect(GetHandle(), &r);
		r.right -= 12;
		r.bottom -= 12;

		HDC dc = ::GetDC(GetHandle());
		HDC mdc = ::CreateCompatibleDC(dc);
		HBITMAP bitmap = ::CreateCompatibleBitmap(dc, r.right, r.bottom);
		::SelectObject(mdc, bitmap);
		
		HBRUSH brush = ::CreateSolidBrush(RGB(mColor.red, mColor.green, mColor.blue));
		::FillRect(mdc, &r, brush);
		::DeleteObject(brush);

		::DeleteDC(mdc);
		::ReleaseDC(GetHandle(), dc);
		
		bitmap = (HBITMAP)::SendMessage(GetHandle(), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
		if (bitmap != nullptr)
			::DeleteObject(bitmap);
	}
}

//void MWinColorSwatchImpl::GetIdealSize(int32_t& outWidth, int32_t& outHeight)
//{
//	outWidth = 30;
//	outHeight = 23;
//
//	SIZE size;
//	if (GetHandle() != nullptr and ColorSwatch_GetIdealSize(GetHandle(), &size))
//	{
//		if (outWidth < size.cx + 20)
//			outWidth = size.cx + 20;
//
//		if (outHeight < size.cy + 2)
//			outHeight = size.cy + 2;
//	}
//}

MColorSwatchImpl* MColorSwatchImpl::Create(MColorSwatch* inColorSwatch, MColor inColor)
{
	return new MWinColorSwatchImpl(inColorSwatch, inColor);
}

// --------------------------------------------------------------------

MWinListBoxImpl::MWinListBoxImpl(MListBox* inListBox)
	: MWinControlImpl(inListBox, "")
{
}

void MWinListBoxImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outStyle = WS_CHILD | LBS_HASSTRINGS | LBS_NOTIFY;
	outExStyle |= WS_EX_CLIENTEDGE;
	
	outClassName = WC_LISTBOX;
}

void MWinListBoxImpl::AddedToWindow()
{
	MWinControlImpl::AddedToWindow();
	
	for (string& item: mItems)
		AddItem(item);
	
	mItems.clear();
	
	SetValue(0);
}

void MWinListBoxImpl::AddItem(const string& inText)
{
	if (GetHandle() == nullptr)
		mItems.push_back(inText);
	else
	{
		wstring text(c2w(inText));
		::SendMessage(GetHandle(), LB_ADDSTRING, 0, (LPARAM)text.c_str());
	}
}

int32_t MWinListBoxImpl::GetValue() const
{
	return ::SendMessage(GetHandle(), LB_GETCURSEL, 0, 0);
}

void MWinListBoxImpl::SetValue(int32_t inValue)
{
	::SendMessage(GetHandle(), LB_SETCURSEL, inValue, 0);
}

bool MWinListBoxImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	switch (inUMsg)
	{
		case LBN_SELCHANGE:
		case LBN_SELCANCEL:
			mControl->eValueChanged(mControl->GetID(), ::SendMessage(GetHandle(), LB_GETCURSEL, 0, 0));
			break;
	}
	
	return true;
}

MListBoxImpl* MListBoxImpl::Create(MListBox* inListBox)
{
	return new MWinListBoxImpl(inListBox);
}

// // --------------------------------------------------------------------

// MWinListViewImpl::MWinListViewImpl(MListView* inListView)
// 	: MWinControlImpl(inListView, "")
// {
// }

// void MWinListViewImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
// 	wstring& outClassName, HMENU& outMenu)
// {
// 	MWinControlImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

// 	outStyle = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER;
// 	outExStyle |= WS_EX_STATICEDGE;
	
// 	outClassName = WC_LISTVIEW;
// }

// void MWinListViewImpl::CreateHandle(MWinProcMixin* inParent, MRect inBounds, const wstring& inTitle)
// {
// 	using namespace std::placeholders;

// 	MWinControlImpl::CreateHandle(inParent, inBounds, inTitle);
	
// 	// add a single column
//     LVCOLUMN lvc = {};

//     lvc.mask = LVCF_FMT | LVCF_SUBITEM;
// 	lvc.cx = inBounds.width - 10;
// 	lvc.fmt = LVCFMT_LEFT;
// 	::SendMessage(GetHandle(), LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

// 	if (inParent != nullptr)
// 	{
// 		inParent->AddNotify(LVN_ITEMACTIVATE, GetHandle(),
// 			std::bind(&MWinListViewImpl::LVMItemActivate, this, _1, _2, _3));
// 		inParent->AddNotify(LVN_GETDISPINFO, GetHandle(),
// 			std::bind(&MWinListViewImpl::LVMGetDispInfo, this, _1, _2, _3));
// 	}
// }

// void MWinListViewImpl::AddedToWindow()
// {
// 	MWinControlImpl::AddedToWindow();
	
// 	for (string& item: mItems)
// 		AddItem(item);
	
// 	mItems.clear();
// }

// void MWinListViewImpl::AddItem(const string& inText)
// {
// 	if (GetHandle() != nullptr)
// 		mItems.push_back(inText);
// 	else
// 	{
// 		wstring text(c2w(inText));

// 		LVITEM item = {};
		
// 		item.mask = LVIF_TEXT | LVIF_STATE;
// 		item.pszText = const_cast<wchar_t*>(text.c_str());
// 		item.iItem = ::SendMessage(GetHandle(), LVM_GETITEMCOUNT, 0, 0);
		
// 		::SendMessage(GetHandle(), LVM_INSERTITEM, 0, (LPARAM)&item);
// 	}
// }

// bool MWinListViewImpl::LVMItemActivate(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
// {
// 	NMITEMACTIVATE* nmItemActivate = reinterpret_cast<NMITEMACTIVATE*>(inLParam);
	
// 	mControl->eValueChanged(mControl->GetID(), nmItemActivate->iItem);
	
// 	return true;
// }

// bool MWinListViewImpl::LVMGetDispInfo(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
// {
// 	NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(inLParam);
	
	
// 	return true;
// }

// MListViewImpl* MListViewImpl::Create(MListView* inListView)
// {
// 	return new MWinListViewImpl(inListView);
// }

