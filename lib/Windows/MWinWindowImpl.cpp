//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include <filesystem>

#include <comdef.h>
#include <Dwmapi.h>

#pragma comment(lib, "dwmapi")

#include "zeep/xml/document.hpp"
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include "MWinWindowImpl.hpp"
#include "MWindow.hpp"
#include "MError.hpp"
#include "MWinApplicationImpl.hpp"
#include "MWinControlsImpl.hpp"
#include "MWinCanvasImpl.hpp"
#include "MUtils.hpp"
#include "MWinUtils.hpp"
#include "MWinMenuImpl.hpp"
#include "MAcceleratorTable.hpp"

using namespace std;
using namespace zeep;
namespace io = boost::iostreams;

const int BIT_COUNT = 32;

//namespace
//{
//
//class MDropTarget : public IDropTarget
//{
//  public:
//				MDropTarget(MWinWindowImpl* inImpl);
//	virtual		~MDropTarget();
//
//	STDMETHOD(DragEnter)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
//	STDMETHOD(DragLeave)();
//	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
//	STDMETHOD(Drop)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
//
//  public:
//	unsigned long STDMETHODCALLTYPE AddRef();
//	unsigned long STDMETHODCALLTYPE Release();
//	HRESULT STDMETHODCALLTYPE QueryInterface(
//		IID const& riid,
//		void** ppvObject
//	);
//
//  private:
//	MWinWindowImpl*			mWindowImpl;
//	unsigned long			mRefCount;
//	static UINT				sCFSTR_FILEDESCRIPTOR;
//};
//
//UINT MDropTarget::sCFSTR_FILEDESCRIPTOR = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
//
//MDropTarget::MDropTarget(MWinWindowImpl* inImpl)
//	: mWindowImpl(inImpl)
//	, mRefCount(1)
//{
//}
//
//MDropTarget::~MDropTarget()
//{
//}
//
//HRESULT STDMETHODCALLTYPE MDropTarget::DragEnter(
//	IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
//{
//	FORMATETC
//		fmt_text = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
//		fmt_file = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
////		fmt_file = { sCFSTR_FILEDESCRIPTOR, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
//
//	HRESULT result = S_FALSE;
//	if (pDataObj->QueryGetData(&fmt_text) == S_OK or
//		pDataObj->QueryGetData(&fmt_file) == S_OK)
//	{
//		if (*pdwEffect & DROPEFFECT_COPY)
//			result = S_OK;
//	}
//	
//	return result;
//}
//
//HRESULT STDMETHODCALLTYPE MDropTarget::DragLeave()
//{
//	return S_OK;
//}
//
//HRESULT STDMETHODCALLTYPE MDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
//{
//	return S_OK;
//}
//
//HRESULT STDMETHODCALLTYPE MDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
//{
//	return S_OK;
//}
//
//unsigned long STDMETHODCALLTYPE MDropTarget::AddRef()
//{
//    return InterlockedIncrement(&mRefCount);
//}
//
//unsigned long STDMETHODCALLTYPE MDropTarget::Release()
//{
//	unsigned long newCount = InterlockedDecrement(&mRefCount);
//
//    if (newCount == 0)
//    {
//        delete this;
//        return 0;
//    }
//
//    return newCount;
//}
//
//HRESULT STDMETHODCALLTYPE MDropTarget::QueryInterface(IID const& riid, void** ppvObject)
//{
//    if (__uuidof(IDropTarget) == riid)
//        *ppvObject = dynamic_cast<IDropTarget*>(this);
//    else if (__uuidof(IUnknown) == riid)
//        *ppvObject = dynamic_cast<IUnknown*>(this);
//    else
//    {
//        *ppvObject = NULL;
//        return E_FAIL;
//    }
//
//    return S_OK;
//}
//
//}

// --------------------------------------------------------------------

MWinWindowImpl::MWinWindowImpl(MWindowFlags inFlags, const string& inMenu,
		MWindow* inWindow)
	: MWindowImpl(inFlags, inWindow)
	, MWinProcMixin(inWindow)
	, mSizeBox(nullptr)
	, mStatus(nullptr)
	, mMinWidth(100)
	, mMinHeight(100)
	, mMenubar(nullptr)
	, mMousedView(nullptr)
	, mClickCount(0)
	, mLastClickTime(0)
	, mDropTarget(nullptr)
	, mCustomNonClient(false)
	, mCallDWP(false)
	, mCaptionHeight(0)
	, mHorizontalResizeMargin(0)
	, mVerticalResizeMargin(0)
{
	memset(&mNonClientOffset, 0, sizeof(mNonClientOffset));
	memset(&mNonClientMargin, 0, sizeof(mNonClientMargin));

	if (not inMenu.empty())
		mMenubar = MWindowImpl::CreateMenu(inMenu);
}

MWinWindowImpl::~MWinWindowImpl()
{
	if (mDropTarget)
		mDropTarget->Release();
	delete mMenubar;
}

void MWinWindowImpl::CreateParams(DWORD& outStyle,
	DWORD& outExStyle, wstring& outClassName, HMENU& outMenu)
{
	MWinProcMixin::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outClassName = L"MWinWindowImpl";
	outStyle = WS_OVERLAPPEDWINDOW;
	if (mFlags & kMFixedSize)
		outStyle &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
	outExStyle = 0;

	if (mMenubar != nullptr and (mFlags & kMCustomNonClient) == 0)
		outMenu = static_cast<MWinMenuImpl*>(mMenubar->impl())->GetHandle();
}

void MWinWindowImpl::RegisterParams(UINT& outStyle, int& outWndExtra,
	HCURSOR& outCursor, HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground)
{
	MWinProcMixin::RegisterParams(outStyle, outWndExtra,
		outCursor, outIcon, outSmallIcon, outBackground);
	
	HINSTANCE inst = MWinApplicationImpl::GetInstance()->GetHInstance();
	
	outStyle = 0;// CS_HREDRAW | CS_VREDRAW;
	outIcon = ::LoadIcon(inst, MAKEINTRESOURCE(1));
	outSmallIcon = ::LoadIcon(inst, MAKEINTRESOURCE(2));
	outCursor = ::LoadCursor(NULL, IDC_ARROW);
	
	if (mFlags & kMNoEraseOnUpdate)
		outBackground = 0;
	else if (mFlags & kMDialogBackground)
		outBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	else
		outBackground = (HBRUSH)(COLOR_WINDOW + 1);
}

LRESULT MWinWindowImpl::WinProc(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam)
{
	LRESULT lRet = 0;
	
	if (not ::DwmDefWindowProc(inHWnd, inUMsg, inWParam, inLParam, &lRet))
		lRet = MWinProcMixin::WinProc(inHWnd, inUMsg, inWParam, inLParam);
	
	return lRet;
}


void MWinWindowImpl::Create(MRect inBounds, const wstring& inTitle)
{
	using namespace std::placeholders;

	AddHandler(WM_CREATE,			std::bind(&MWinWindowImpl::WMCreate, this, _1, _2, _3, _4, _5));
	AddHandler(WM_CLOSE,			std::bind(&MWinWindowImpl::WMClose, this, _1, _2, _3, _4, _5));
	AddHandler(WM_ACTIVATE,			std::bind(&MWinWindowImpl::WMActivate, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSEACTIVATE,	std::bind(&MWinWindowImpl::WMMouseActivate, this, _1, _2, _3, _4, _5));
	AddHandler(WM_SIZE,				std::bind(&MWinWindowImpl::WMSize, this, _1, _2, _3, _4, _5));
	AddHandler(WM_SIZING,			std::bind(&MWinWindowImpl::WMSizing, this, _1, _2, _3, _4, _5));
	AddHandler(WM_PAINT,			std::bind(&MWinWindowImpl::WMPaint, this, _1, _2, _3, _4, _5));
	if (mFlags & (kMDialogBackground | kMNoEraseOnUpdate))
		AddHandler(WM_ERASEBKGND,	std::bind(&MWinWindowImpl::WMEraseBkgnd, this, _1, _2, _3, _4, _5));
	AddHandler(WM_INITMENU,			std::bind(&MWinWindowImpl::WMInitMenu, this, _1, _2, _3, _4, _5));
//	AddHandler(WM_MENUCHAR,			std::bind(&MWinWindowImpl::WMMenuChar, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MENUCOMMAND,		std::bind(&MWinWindowImpl::WMMenuCommand, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONDOWN,		std::bind(&MWinWindowImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONDBLCLK,	std::bind(&MWinWindowImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONUP,		std::bind(&MWinWindowImpl::WMMouseUp, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSEMOVE,		std::bind(&MWinWindowImpl::WMMouseMove, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSELEAVE,		std::bind(&MWinWindowImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
	AddHandler(WM_CAPTURECHANGED,	std::bind(&MWinWindowImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
	AddHandler(WM_CONTEXTMENU,		std::bind(&MWinWindowImpl::WMContextMenu, this, _1, _2, _3, _4, _5));
	AddHandler(WM_SETCURSOR,		std::bind(&MWinWindowImpl::WMSetCursor, this, _1, _2, _3, _4, _5));
//	AddHandler(WM_IME_COMPOSITION,	std::bind(&MWinWindowImpl::WMImeComposition, this, _1, _2, _3, _4, _5));
//	AddHandler(WM_IME_STARTCOMPOSITION,
//									std::bind(&MWinWindowImpl::WMImeStartComposition, this, _1, _2, _3, _4, _5));
//	AddHandler(WM_IME_REQUEST,		std::bind(&MWinWindowImpl::WMImeRequest, this, _1, _2, _3, _4, _5));
	AddHandler(WM_DROPFILES,		std::bind(&MWinWindowImpl::WMDropFiles, this, _1, _2, _3, _4, _5));

	AddHandler(WM_THEMECHANGED,		std::bind(&MWinWindowImpl::WMThemeChanged, this, _1, _2, _3, _4, _5));
	AddHandler(WM_DWMCOMPOSITIONCHANGED,
									std::bind(&MWinWindowImpl::WMDwmCompositionChanged, this, _1, _2, _3, _4, _5));
	AddHandler(WM_NCCALCSIZE,		std::bind(&MWinWindowImpl::WMNCCalcSize, this, _1, _2, _3, _4, _5));
	AddHandler(WM_NCHITTEST,		std::bind(&MWinWindowImpl::WMNCHitTest, this, _1, _2, _3, _4, _5));

	if (mFlags & kMPostionDefault)
		inBounds.x = inBounds.y = CW_USEDEFAULT;

	MWinProcMixin::CreateHandle(nullptr, inBounds, inTitle);

	if (not (mFlags & (kMFixedSize | kMNoSizeBox)))
	{
		MRect r;
		mWindow->GetBounds(r);
		mSizeBox = ::CreateWindowExW(0, L"SCROLLBAR", nullptr,
			WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN,
			r.x + r.width - kScrollbarWidth, r.y + r.height - kScrollbarWidth,
			kScrollbarWidth, kScrollbarWidth, GetHandle(),
			nullptr, MWinApplicationImpl::GetInstance()->GetHInstance(),
			nullptr);
	}
	
	if (mFlags & kMAcceptFileDrops)
		::DragAcceptFiles(GetHandle(), true);
	
//	if (mFlags & kMAcceptDragAndDrop)
//	{
//		mDropTarget = new MDropTarget(this);
//		::RegisterDragDrop(GetHandle(), mDropTarget);
//	}
	
	RECT clientArea;
	::GetClientRect(GetHandle(), &clientArea);

	mWindow->SetFrame(MRect{
		clientArea.left,
		clientArea.top,
		clientArea.right - clientArea.left,
		clientArea.bottom - clientArea.top});

	if (mMenubar != nullptr)
		mMenubar->SetTarget(mWindow);
}

void MWinWindowImpl::SetTransparency(float inAlpha)
{
	LONG style = ::GetWindowLong(GetHandle(), GWL_EXSTYLE);
	
	if (inAlpha == 1.0f)
		::SetWindowLong(GetHandle(), GWL_EXSTYLE, style & ~WS_EX_LAYERED);
	else
	{
		::SetWindowLong(GetHandle(), GWL_EXSTYLE, style | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(GetHandle(), 0, static_cast<uint8_t>(inAlpha * 255), LWA_ALPHA);
	}

	//DWM_BLURBEHIND bb = {0};

 //   // Specify blur-behind and blur region.
 //   bb.dwFlags = DWM_BB_ENABLE;
 //   bb.fEnable = true;
 //   bb.hRgnBlur = NULL;

 //   // Enable blur-behind.
 //   HRESULT hr = DwmEnableBlurBehindWindow(GetHandle(), &bb);

}

bool MWinWindowImpl::IsDialogMessage(MSG& inMessage)
{
	return false;
}

// --------------------------------------------------------------------
// overrides for MWindowImlp

void MWinWindowImpl::SetTitle(string inTitle)
{
	::SetWindowTextW(GetHandle(), c2w(inTitle).c_str());

	DWORD flags = RDW_INVALIDATE | RDW_FRAME;
	::RedrawWindow(GetHandle(), nullptr, nullptr, flags);
}

//string MWinWindowImpl::GetTitle() const
//{
//	return mTitle;
//}

void MWinWindowImpl::Show()
{
	::ShowWindow(GetHandle(), SW_RESTORE);
}

void MWinWindowImpl::Hide()
{
	::ShowWindow(GetHandle(), SW_HIDE);
}

bool MWinWindowImpl::Visible() const
{
	return ::IsWindowVisible(GetHandle()) != 0;
}

void MWinWindowImpl::Select()
{
	WINDOWPLACEMENT pl = { sizeof(WINDOWPLACEMENT) };

	if (not Visible())
		Show();

	if (::GetWindowPlacement(GetHandle(), &pl) and ::IsIconic(GetHandle()))
	{
		pl.showCmd = SW_RESTORE;
		::SetWindowPlacement(GetHandle(), &pl);
	}

	::SetActiveWindow(GetHandle());
	::SetForegroundWindow(GetHandle());
	mWindow->Activate();
}

void MWinWindowImpl::Close()
{
	if (GetHandle() != nullptr)
	{
		if (not ::DestroyWindow(GetHandle()))
			THROW_WIN_ERROR(("Error destroying window"));
	}
}

//virtual void	ActivateSelf();
//virtual void	DeactivateSelf();

//void MWinWindowImpl::SetFocus(MView* inFocus)
//{
//	MWinProcMixin* mixin = nullptr;
//	
//	if (dynamic_cast<MControlBase*>(inFocus) != nullptr)
//		mixin = dynamic_cast<MWinProcMixin*>(static_cast<MControlBase*>(inFocus)->GetImplBase());
//	else if (dynamic_cast<MCanvas*>(inFocus) != nullptr)
//		mixin = dynamic_cast<MWinCanvasImpl*>(static_cast<MCanvas*>(inFocus)->GetImpl());
//	
//	if (mixin != nullptr)
//	{
//		::SetFocus(mixin->GetHandle());
//	}
//}

MHandler* MWinWindowImpl::GetFocus()
{
	MHandler* result = nullptr;
	
	MWinProcMixin* mixin = Fetch(::GetFocus());
	if (mixin != nullptr)
		result = mixin->GetHandler();
	
	return result;
}
	
void MWinWindowImpl::ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta)
{
	if (GetHandle() != nullptr)
	{
		MRect wr;
		GetWindowPosition(wr);
		::MoveWindow(GetHandle(), wr.x, wr.y,
			wr.width + inWidthDelta, wr.height + inHeightDelta, true);
	}
}

void MWinWindowImpl::SetWindowPosition(MRect inBounds, bool inTransition)
{
	MRect main;
	MWindow::GetMainScreenBounds(main);

	if (main.Intersects(inBounds))
		inBounds = main & inBounds;
	else
	{
#pragma message("Handle placement of windows outside the main area")
	}
	
	// only sensible sizes are allowed
	if (inBounds and inBounds.width > 20 and inBounds.height > 20)
		::MoveWindow(GetHandle(), inBounds.x,
			inBounds.y, inBounds.width, inBounds.height, true);
}

void MWinWindowImpl::GetWindowPosition(MRect& outBounds) const
{
	RECT r;
	::GetWindowRect(GetHandle(), &r);
	outBounds = MRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}
	
void MWinWindowImpl::Invalidate(MRect inRect)
{
	RECT r = { inRect.x, inRect.y, inRect.x + inRect.width, inRect.y + inRect.height };
	::InvalidateRect(GetHandle(), &r, false);
}

void MWinWindowImpl::Validate(MRect inRect)
{
	RECT r = { inRect.x, inRect.y, inRect.x + inRect.width, inRect.y + inRect.height };
	::ValidateRect(GetHandle(), &r);
}

void MWinWindowImpl::UpdateNow()
{
	::UpdateWindow(GetHandle());
}

void MWinWindowImpl::ScrollRect(MRect inRect, int32_t inDeltaH, int32_t inDeltaV)
{
	RECT r = { inRect.x, inRect.y, inRect.x + inRect.width, inRect.y + inRect.height };
//	::ScrollWindowEx(GetHandle(), inDeltaH, inDeltaV, &r, &r, nullptr, nullptr, SW_INVALIDATE);
	::InvalidateRect(GetHandle(), &r, false);
}
	
bool MWinWindowImpl::GetMouse(int32_t& outX, int32_t& outY, uint32_t& outModifiers)
{
	POINT lPoint;
	::GetCursorPos(&lPoint);
	::ScreenToClient(GetHandle(), &lPoint);

	int button = VK_LBUTTON;
	if (::GetSystemMetrics(SM_SWAPBUTTON))
		button = VK_RBUTTON;

	bool result = (::GetAsyncKeyState(button) & 0x8000) != 0;
	
	if (result and
		mLastGetMouseX == lPoint.x and
		mLastGetMouseY == lPoint.y)
	{
		::delay(0.02);
		::GetCursorPos(&lPoint);
		::ScreenToClient(GetHandle(), &lPoint);
		
		result = (::GetAsyncKeyState(button) & 0x8000) != 0;
	}
	
	outX = lPoint.x;
	outY = lPoint.y;
	
	mLastGetMouseX = lPoint.x;
	mLastGetMouseY = lPoint.y;

	::GetModifierState(outModifiers, true);

	return result;
}

bool MWinWindowImpl::WaitMouseMoved(int32_t inX, int32_t inY)
{
	bool result = false;

	if (mWindow->IsActive())
	{
		POINT w = { inX, inY };
		result = ::DragDetect(GetHandle(), w) != 0;
	}
	else if (MWindow::GetFirstWindow() and MWindow::GetFirstWindow()->IsActive())
	{
		double test = GetLocalTime() + 0.5;
		
		for (;;)
		{
			if (GetLocalTime() > test)
			{
				result = true;
				break;
			}
			
			int32_t x, y;
			uint32_t mod;
			
			if (not GetMouse(x, y, mod))
				break;
			
			if (std::abs(x - inX) > 2 or
				std::abs(y - inY) > 2)
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

uint32_t MWinWindowImpl::GetModifiers() const
{
	uint32_t modifiers = 0;
	if (::GetKeyState(VK_SHIFT) & 0x8000)
		modifiers |= kShiftKey;
	if (::GetKeyState(VK_CONTROL) & 0x8000)
		modifiers |= kControlKey;
	if (::GetKeyState(VK_MENU) & 0x8000)
		modifiers |= kOptionKey;
	return modifiers;
}

void MWinWindowImpl::SetCursor(MCursor inCursor)
{
	static HCURSOR sArrow = ::LoadCursor(NULL, IDC_ARROW);
	static HCURSOR sIBeam = ::LoadCursor(NULL, IDC_IBEAM);
	static HCURSOR sRightArrow = nullptr;

	switch (inCursor)
	{
		case eNormalCursor:
			if (sArrow != nullptr)
				::SetCursor(sArrow);
			break;

		case eIBeamCursor:
			if (sIBeam != nullptr)
				::SetCursor(sIBeam);
			break;

		case eRightCursor:
		{
			if (sRightArrow == nullptr)
			{
				ICONINFO info;
				if (::GetIconInfo(sArrow, &info))
				{
					BITMAP color = {}, mask = {};

					HDC dc = ::GetDC(GetHandle());

					::GetObjectW(info.hbmColor, sizeof(BITMAP), &color);
					::GetObjectW(info.hbmMask, sizeof(BITMAP), &mask);

					BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER) };
					bi.biWidth = color.bmWidth;    
					bi.biHeight = color.bmHeight;  
					bi.biPlanes = 1;    
					bi.biBitCount = 32;    
					bi.biCompression = BI_RGB;

					vector<uint32_t> rgba(bi.biWidth);

					for (int row = 0; row < bi.biHeight; ++row)
					{
						if (::GetDIBits(dc, info.hbmColor, row, 1, &rgba[0], (BITMAPINFO*)&bi, DIB_RGB_COLORS) == 1)
						{
							reverse(rgba.begin(), rgba.end());
							(void)::SetDIBits(dc, info.hbmColor, row, 1, &rgba[0], (BITMAPINFO*)&bi, DIB_RGB_COLORS);
						}

						if (::GetDIBits(dc, info.hbmMask, row, 1, &rgba[0], (BITMAPINFO*)&bi, DIB_RGB_COLORS) == 1)
						{
							reverse(rgba.begin(), rgba.end());
							(void)::SetDIBits(dc, info.hbmMask, row, 1, &rgba[0], (BITMAPINFO*)&bi, DIB_RGB_COLORS);
						}
					}
					
					::ReleaseDC(GetHandle(), dc);
					
					info.xHotspot = 32 - info.xHotspot;
					sRightArrow = ::CreateIconIndirect(&info);
				}
			}

			if (sRightArrow != nullptr)
				::SetCursor(sRightArrow);
			break;
		}

		default:
			break;
	}
}

void MWinWindowImpl::ObscureCursor()
{
	::SetCursor(nullptr);
}

void MWinWindowImpl::ConvertToScreen(int32_t& ioX, int32_t& ioY) const
{
	POINT p = { ioX, ioY };
	::ClientToScreen(GetHandle(), &p);
	ioX = p.x;
	ioY = p.y;
}

void MWinWindowImpl::ConvertFromScreen(int32_t& ioX, int32_t& ioY) const
{
	POINT p = { ioX, ioY };
	::ScreenToClient(GetHandle(), &p);
	ioX = p.x;
	ioY = p.y;
}

bool MWinWindowImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	MHandler* focus = mWindow->GetFocus();
	if (focus == nullptr)
		focus = mWindow;
	return focus->HandleKeyDown(inKeyCode, inModifiers, inRepeat);
}

bool MWinWindowImpl::DispatchCharacter(const string& inText, bool inRepeat)
{
	MHandler* focus = mWindow->GetFocus();
	if (focus == nullptr)
		focus = mWindow;
	return focus->HandleCharacter(inText, inRepeat);
}

bool MWinWindowImpl::DispatchCommand(uint32_t inCommand, uint32_t inModifiers)
{
	MHandler* focus = mWindow->GetFocus();
	if (focus == nullptr)
		focus = mWindow;

	bool result = false, enabled = false, checked = false;

	if (focus->UpdateCommandStatus(inCommand, nullptr, 0, enabled, checked) and enabled)
		result = focus->ProcessCommand(inCommand, nullptr, 0, inModifiers);

	return result;
}

// --------------------------------------------------------------------
// Windows Message handling

bool MWinWindowImpl::WMCreate(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
{
//	if (mFlags & kMCustomNonClient)
//	{
////	    RECT rcClient;
////	    ::GetWindowRect(GetHandle(), &rcClient);
////	
////	    // Inform application of the frame change.
////	    ::SetWindowPos(GetHandle(),  nullptr, rcClient.left, rcClient.top,
////			rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);
//		UpdateNonClientMargins();
//	}

	return false;
}

bool MWinWindowImpl::WMClose(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
{
	if (mWindow->AllowClose(false))
		mWindow->Close();
	return true;
}

// Destroy (delete) myself and notify some others,
bool MWinWindowImpl::WMDestroy(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = MWinProcMixin::WMDestroy(inHWnd, inUMsg, inWParam, inLParam, outResult);
	if (result)
	{
		MWindow* w = mWindow;
		mWindow = nullptr;
		delete w;
	}
	return result;
}

bool MWinWindowImpl::WMActivate(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
{
	if (LOWORD(inWParam) == WA_INACTIVE)
		mWindow->Deactivate ();
	else if (mWindow->IsEnabled())
		mWindow->Activate();

	if (UpdateNonClientMargins() != mCustomNonClient)
	{
		mCustomNonClient = not mCustomNonClient;
		
		if (mCustomNonClient)
		{
			RECT rcClient;
	        GetWindowRect(GetHandle(), &rcClient);
	
	        // Inform application of the frame change.
	        SetWindowPos(GetHandle(), 
	                     NULL, 
	                     rcClient.left, rcClient.top,
	                     rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
	                     SWP_FRAMECHANGED);
	
			DWORD flags = RDW_INVALIDATE | RDW_FRAME;
			::RedrawWindow(GetHandle(), nullptr, nullptr, flags);
		}
	}

	return false;
}

bool MWinWindowImpl::WMMouseActivate(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM inLParam, LRESULT& outResult)
{
	outResult = MA_ACTIVATE;
	
	if (not mWindow->IsEnabled())
	{
		outResult = MA_NOACTIVATEANDEAT;
	}
	else if (LOWORD(inLParam) == HTCLIENT and not mWindow->IsActive())
	{
		uint32_t modifiers;
		GetModifierState(modifiers, false);
		
		POINT lPoint;
		::GetCursorPos(&lPoint);
		::ScreenToClient(GetHandle(), &lPoint);

		MView* view;
		
		//if (MView::GetGrabbingNode())
		//	node = HNode::GetGrabbingNode();
		//else
			view = mWindow->FindSubView(lPoint.x, lPoint.y);
		assert(view != nullptr);
		
		int32_t x = lPoint.x;
		int32_t y = lPoint.y;
		
		view->ConvertFromWindow(x, y);
		
		if (view->ActivateOnClick(x, y, modifiers))
			outResult = MA_ACTIVATEANDEAT;
		else
			outResult = MA_NOACTIVATEANDEAT;
	}
	
	return true;
}

bool MWinWindowImpl::WMSize(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	if (inWParam != SIZE_MINIMIZED)
	{
		MRect newBounds(0, 0, LOWORD(inLParam), HIWORD(inLParam));
		MRect oldBounds;
		mWindow->GetBounds(oldBounds);

		if (mSizeBox != nullptr)
		{
			//int kScrollbarWidth = HScrollBarNode::GetScrollBarWidth();
			int kScrollbarWidth = 16;

			MRect r(newBounds);
			r.x += r.width - kScrollbarWidth;
			r.y += r.height- kScrollbarWidth;
			
			::MoveWindow(mSizeBox, r.x, r.y, r.width, r.height, true);
//			::SetWindowPos(fSizeBox, GetHandle(), lNewBounds.right - kScrollbarWidth,
//				lNewBounds.bottom - kScrollbarWidth, 0, 0,
//				SWP_NOZORDER | SWP_NOZORDER);
		}

		mWindow->SetMargins(mNonClientMargin.left, mNonClientMargin.top, mNonClientMargin.right, mNonClientMargin.bottom);
		
		MRect bounds;
		mWindow->GetBounds(bounds);
		
		int32_t dw = newBounds.width - mNonClientMargin.left - mNonClientMargin.right - bounds.width;
		int32_t dh = newBounds.height - mNonClientMargin.top - mNonClientMargin.bottom - bounds.height;
		
		if (dw or dh)
			mWindow->MView::ResizeFrame(dw, dh);
	}

	return true;
//	return false;
}

bool MWinWindowImpl::WMSizing(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = true;

	RECT& r = *reinterpret_cast<RECT*>(inLParam);

	switch (inWParam)
	{
		case WMSZ_LEFT:
			if (r.right - r.left < mMinWidth)
				r.left = r.right - mMinWidth;
			break;
		case WMSZ_RIGHT:
			if (r.right - r.left < mMinWidth)
				r.right = r.left + mMinWidth;
			break;
		case WMSZ_TOP:
			if (r.bottom - r.top < mMinHeight)
				r.top = r.bottom - mMinHeight;
			break;
		case WMSZ_TOPLEFT:
			if (r.right - r.left < mMinWidth)
				r.left = r.right - mMinWidth;
			if (r.bottom - r.top < mMinHeight)
				r.top = r.bottom - mMinHeight;
			break;
		case WMSZ_TOPRIGHT:
			if (r.right - r.left < mMinWidth)
				r.right = r.left + mMinWidth;
			if (r.bottom - r.top < mMinHeight)
				r.top = r.bottom - mMinHeight;
			break;
		case WMSZ_BOTTOM:
			if (r.bottom - r.top < mMinHeight)
				r.bottom = r.top + mMinHeight;
			break;
		case WMSZ_BOTTOMLEFT:
			if (r.right - r.left < mMinWidth)
				r.left = r.right - mMinWidth;
			if (r.bottom - r.top < mMinHeight)
				r.bottom = r.top + mMinHeight;
			break;
		case WMSZ_BOTTOMRIGHT:
			if (r.right - r.left < mMinWidth)
				r.right = r.left + mMinWidth;
			if (r.bottom - r.top < mMinHeight)
				r.bottom = r.top + mMinHeight;
			break;
	}

	outResult = 1;
	return result;
}

bool MWinWindowImpl::WMPaint(HWND inHWnd, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM /*inLParam*/, LRESULT& outResult)
{
	RECT lUpdateRect;
	if (mCustomNonClient and ::GetUpdateRect(inHWnd, &lUpdateRect, FALSE) == TRUE)
	{
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(inHWnd, &ps);
		if (hdc)
		{
			PaintCustomCaption(hdc);
			::EndPaint(inHWnd, &ps);
		}
	}
	
	return false;
}

bool MWinWindowImpl::WMEraseBkgnd(HWND inHWnd, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM /*inLParam*/, LRESULT& outResult)
{
	outResult = 0;
	bool result = false;
	
	if (mFlags & kMDialogBackground)
	{
		/* Get the 'dirty' rect */
		RECT lUpdateRect;
		if ((mFlags & kMDialogBackground) and ::GetUpdateRect(inHWnd, &lUpdateRect, FALSE) == TRUE)
		{
			PAINTSTRUCT lPs;
			HDC hdc = ::BeginPaint(inHWnd, &lPs);
			if (hdc)
			{
				::FillRect(hdc, &lUpdateRect, ::GetSysColorBrush(COLOR_BTNFACE));
				::EndPaint(inHWnd, &lPs);
			}
		
			outResult = 1;
			result = true;
		}
	}
	else if (mFlags & kMNoEraseOnUpdate)
	{
		RECT lUpdateRect;
		if (mCustomNonClient and ::GetUpdateRect(inHWnd, &lUpdateRect, FALSE) == TRUE)
		{
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(inHWnd, &ps);
			if (hdc)
			{
				RECT bounds;
				::GetClientRect(GetHandle(), &bounds);
				
				RECT rl = { 0, 0, mHorizontalResizeMargin, bounds.bottom };
				::FillRect(hdc, &rl, (HBRUSH)GetStockObject(BLACK_BRUSH));				

				RECT rt = { 0, 0, bounds.right, mCaptionHeight };
				::FillRect(hdc, &rt, (HBRUSH)GetStockObject(BLACK_BRUSH));				

				RECT rr = { bounds.right - mHorizontalResizeMargin, 0, bounds.right, bounds.bottom };
				::FillRect(hdc, &rr, (HBRUSH)GetStockObject(BLACK_BRUSH));				

				RECT rb = { 0, bounds.bottom - mVerticalResizeMargin, bounds.right, bounds.bottom };
				::FillRect(hdc, &rb, (HBRUSH)GetStockObject(BLACK_BRUSH));				
				
				::EndPaint(inHWnd, &ps);
			}
		}
		
		outResult = 1;
		result = true;
	}
	
	return result;
}

bool MWinWindowImpl::WMInitMenu(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
{
	if (mMenubar != nullptr)
		mMenubar->UpdateCommandStatus();
	return false;
}

bool MWinWindowImpl::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	outResult = 1;
	bool result = false;

	if (inLParam == 0)
	{
		uint32_t modifiers;
		GetModifierState(modifiers, false);
		result = mWindow->GetFocus()->ProcessCommand(inWParam, nullptr, 0, modifiers);
	}
	else
		result = MWinProcMixin::WMCommand(inHWnd, inUMsg, inWParam, inLParam, outResult);
	
	return result;
}

//bool MWinWindowImpl::WMMenuChar(HWND inHWnd, UINT /*inUMsg*/, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	outResult = MNC_CLOSE;
//	return true;
//}

bool MWinWindowImpl::WMMenuCommand(HWND inHWnd, UINT /*inUMsg*/, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	outResult = 1;
	
	uint32_t index = inWParam;
	MMenu* menu = MWinMenuImpl::Lookup((HMENU)inLParam);

	return mWindow->GetFocus()->ProcessCommand(menu->GetItemCommand(index), menu, index, 0);
}

bool MWinWindowImpl::WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	::SetFocus(inHWnd);
	::SetCapture(inHWnd);

	uint32_t modifiers;
	::GetModifierState(modifiers, false);
	
	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));

	MView* mousedView = mWindow->FindSubView(x, y);
	if (mousedView == mMousedView)
	{
		if (mLastClickTime + GetDblClickTime() > GetLocalTime())
			mClickCount = mClickCount % 3 + 1;
		else
			mClickCount = 1;
	}
	else
	{
		mClickCount = 1;
		mMousedView = mousedView;
	}

	mLastClickTime = GetLocalTime();

	if (mMousedView != nullptr)
	{
		mMousedView->ConvertFromWindow(x, y);
		mMousedView->MouseDown(x, y, mClickCount, modifiers);
	}

	return true;
}

bool MWinWindowImpl::WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));

	MView* mousedView = mWindow->FindSubView(x, y);
	if (mousedView == mMousedView)
	{
		uint32_t modifiers;
		::GetModifierState(modifiers, false);

		mMousedView->ConvertFromWindow(x, y);
		mMousedView->MouseMove(x, y, modifiers);
	}

	return true;
}

bool MWinWindowImpl::WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	if (mMousedView)
		mMousedView->MouseExit();

	return true;
}

bool MWinWindowImpl::WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	::ReleaseCapture();

	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));

	MView* mousedView = mWindow->FindSubView(x, y);
	if (mousedView == mMousedView)
	{
		uint32_t modifiers;
		::GetModifierState(modifiers, false);
	
		mMousedView->ConvertFromWindow(x, y);
		mMousedView->MouseUp(x, y, modifiers);
	}

	return true;
}

bool MWinWindowImpl::WMMouseWheel(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	short deltay = static_cast<short>(HIWORD(inWParam));
	deltay /= WHEEL_DELTA;
	short deltax = static_cast<short>(LOWORD(inWParam));
	deltax /= WHEEL_DELTA;
	
	uint32_t modifiers;
	::GetModifierState(modifiers, false);
	
	int32_t x = LOWORD(inLParam);
	int32_t y = HIWORD(inLParam);
	mWindow->ConvertFromScreen(x, y);
	
	MView* view = mWindow->FindSubView(x, y);
	
	if (view != nullptr)
	{
		view->ConvertFromWindow(x, y);
		view->MouseWheel(x, y, deltax, deltay, modifiers);
	}

	outResult = 0;
	return true;
}

bool MWinWindowImpl::WMContextMenu(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM inLParam, LRESULT& /*outResult*/)
{
	try
	{
		int32_t x = LOWORD(inLParam);
		int32_t y = HIWORD(inLParam);

		ConvertFromScreen(x, y);
		
		MView* view = mWindow->FindSubView(x, y);
		if (view != nullptr)
		{
			view->ConvertFromWindow(x, y);
			view->ShowContextMenu(x, y);
		}
	}
	catch (...)
	{
	}

	return true;
}

bool MWinWindowImpl::WMSetCursor(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM /*inLParam*/, LRESULT& /*outResult*/)
{
	bool handled = false;
	try
	{
		int32_t x, y;
		uint32_t modifiers;
		
		GetMouse(x, y, modifiers);
		
		MView* view;
		//if (HNode::GetGrabbingNode())
		//	node = HNode::GetGrabbingNode();
		//else
			view = mWindow->FindSubView(x, y);

			// if node == mWindow defproc should handle setcursor
		if (view != nullptr and view != mWindow and view->IsActive())
		{
			view->ConvertFromWindow(x, y);
			view->AdjustCursor(x, y, modifiers);
			handled = true;
		}
	}
	catch (...)
	{
	}

	return handled;
}

//bool MWinWindowImpl::WMImeRequest(HWND /*inHWnd*/, UINT /*inUMsg*/, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	bool result = false;
//	//switch (inWParam)
//	//{
//	//	case IMR_COMPOSITIONWINDOW:
//	//	{
//	//		COMPOSITIONFORM* cf = reinterpret_cast<COMPOSITIONFORM*>(inLParam);
//	//		cf->dwStyle = CFS_POINT;
//
//	//		HPoint pt;
//	//		HHandler::GetFocus()->
//	//			OffsetToPosition(-1, pt);
//
//	//		cf->ptCurrentPos = HNativePoint(pt);
//	//		outResult = 1;
//	//		break;
//	//	}
//	//	
//	//	default:
//	//		break;
//	//}
//	
//	return result;
//}

//bool MWinWindowImpl::WMImeStartComposition(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	return true;
//}
//
//bool MWinWindowImpl::WMImeComposition(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{//
//	bool handled = false;
//	
//	HTextInputAreaInfo info = { 0 };
//	
//	if (inLParam == 0)
//	{
//		HHandler::GetFocus()->
//			UpdateActiveInputArea(nullptr, 0, 0, info);
//		handled = true;
//	}
//	else
//	{
//		HAutoBuf<char> text(nullptr);
//		unsigned long size = 0;
//		
//		if (inLParam & CS_INSERTCHAR)
//		{
//			beep();
//		}
//		if (inLParam & GCS_RESULTSTR) 	
//		{
//			HIMC hIMC = ::ImmGetContext(GetHandle());
//			ThrowIfNil((void*)hIMC);
//	
//			// Get the size of the result string.
//			DWORD dwSize = ::ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
//	
//			// increase buffer size for NULL terminator, 
//			//	 maybe it is in UNICODE
//			dwSize += sizeof(WCHAR);
//	
//			HANDLE hstr = ::GlobalAlloc(GHND,dwSize);
//			ThrowIfNil(hstr);
//	
//			void* lpstr = ::GlobalLock(hstr);
//			ThrowIfNil(lpstr);
//	
//			// Get the result strings that is generated by IME into lpstr.
//			::ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);
//			::ImmReleaseContext(GetHandle(), hIMC);
//	
//			unsigned long s1 = dwSize;
//			unsigned long s2 = 2 * s1;
//			
//			text.reset(new char[s2]);
//	
//			HEncoder::FetchEncoder(enc_UTF16LE)->
//				EncodeToUTF8((char*)lpstr, s1, text.get(), s2);
//			size = std::strlen(text.get());
//	
//			::GlobalUnlock(hstr);
//			::GlobalFree(hstr);	
//			handled = true;
//		}
//		
//		HHandler::GetFocus()->
//			UpdateActiveInputArea(text.get(), size, 0, info);
//	}
//	return handled;
//}

bool MWinWindowImpl::WMDropFiles(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	//HDROP drop = (HDROP)inWParam;
	//
	//unsigned int cnt = ::DragQueryFile(drop, 0xFFFFFFFF, nullptr, 0);
	//HAutoBuf<HUrl>	urls(new HUrl[cnt]);
	//HUrl* url = urls.get();
	//
	//for (unsigned int i = 0; i < cnt; ++i)
	//{
	//	wchar_t path[MAX_PATH];
	//	::DragQueryFileW(drop, i, path, MAX_PATH);
	//	
	//	HFileSpec sp(path);
	//	url[i].SetSpecifier(sp);
	//}

	//::DragFinish(drop);
	//
	//HDragThing drag(static_cast<long>(cnt), url);
	//
	//unsigned long modifiers;
	//GetModifierState(modifiers, false);
	//
	//HPoint where;
	//HNativePoint pt(where);
	//if (::DragQueryPoint(drop, &pt))
	//	where.Set(pt.x, pt.y);
	//
	//HNode* node = mWindow->FindSubPane(where);
	//assert(node != nullptr);
	//if (node->CanAccept(drag))
	//{
	//	node->ConvertFromWindow(where);
	//	node->Receive(drag, where, modifiers);
	//}
	//
	//outResult = 0;
	return true;
}

// --------------------------------------------------------------------

bool MWinWindowImpl::WMThemeChanged(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
//	UpdateNonClientMargins();
//	
//	DWORD flags = RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN;
//	::RedrawWindow(GetHandle(), nullptr, nullptr, flags);
	
	return false;
}

bool MWinWindowImpl::WMDwmCompositionChanged(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
//	UpdateNonClientMargins();
//
//	DWORD flags = RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN;
//	::RedrawWindow(GetHandle(), nullptr, nullptr, flags);
	
	return false;
}

bool MWinWindowImpl::WMNCCalcSize(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;
	
	if (mCustomNonClient)
	{
        RECT *clientRect = inWParam
                         ? &(reinterpret_cast<NCCALCSIZE_PARAMS*>(inLParam))->rgrc[0]
                         : (reinterpret_cast<RECT*>(inLParam));

        clientRect->top      += (mCaptionHeight - mNonClientMargin.top);
        clientRect->left     += (mHorizontalResizeMargin - mNonClientMargin.left);
        clientRect->right    -= (mHorizontalResizeMargin - mNonClientMargin.right);
        clientRect->bottom   -= (mVerticalResizeMargin - mNonClientMargin.bottom);

        result = true;
        outResult = 0;
	}
	
	return result;
}

bool MWinWindowImpl::WMNCHitTest(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;
	
	if (mCustomNonClient)
	{
	    // Get the point coordinates for the hit test.
	    POINT ptMouse = { LOWORD(inLParam), HIWORD(inLParam)};
	
	    // Get the window rectangle.
	    RECT rcWindow;
	    ::GetWindowRect(GetHandle(), &rcWindow);
	
	    // Get the frame rectangle, adjusted for the style without a caption.
	    RECT rcFrame = { 0 };
	    ::AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);
	
	    // Determine if the hit test is for resizing. Default middle (1,1).
	    USHORT uRow = 1;
	    USHORT uCol = 1;
	    bool fOnResizeBorder = false;
	
	    // Determine if the point is at the top or bottom of the window.
	    if (ptMouse.y >= rcWindow.top and ptMouse.y < rcWindow.top + mVerticalResizeMargin + mCaptionHeight)
	    {
	        fOnResizeBorder = (ptMouse.y < (rcWindow.top - rcFrame.top));
	        uRow = 0;
	    }
	    else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - mVerticalResizeMargin)
	    {
	        uRow = 2;
	    }
	
	    // Determine if the point is at the left or right of the window.
	    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + mHorizontalResizeMargin)
	    {
	        uCol = 0; // left side
	    }
	    else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - mHorizontalResizeMargin)
	    {
	        uCol = 2; // right side
	    }
	
	    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
	    LRESULT hitTests[3][3] = 
	    {
	        { HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
	        { HTLEFT,       HTNOWHERE,     HTRIGHT },
	        { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
	    };
	
	    outResult = hitTests[uRow][uCol];
	    result = true;
	}

	return result;
}

bool MWinWindowImpl::UpdateNonClientMargins()
{
	BOOL compositionEnabled;
	if (::DwmIsCompositionEnabled(&compositionEnabled) != S_OK)
		compositionEnabled = false;

	bool result;

	if (mFlags & kMCustomNonClient and compositionEnabled)
	{
		WINDOWPLACEMENT pl = {};
		::GetWindowPlacement(GetHandle(), &pl);
		
		mCaptionHeight = ::GetSystemMetrics(SM_CYFRAME) +
			::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CXPADDEDBORDER);
		
		mHorizontalResizeMargin = ::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);
		mVerticalResizeMargin = ::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);
	
		memset(&mNonClientOffset, 0, sizeof(mNonClientOffset));
	
		if (pl.showCmd == SW_SHOWMAXIMIZED)
		{
			mNonClientOffset.top = mCaptionHeight;
	
			APPBARDATA appBarData = { sizeof(appBarData) };
			UINT taskbarState = ::SHAppBarMessage(ABM_GETSTATE, &appBarData);
			if (ABS_AUTOHIDE & taskbarState)
			{
				UINT edge = -1;
				appBarData.hWnd = ::FindWindow(L"Shell_TrayWnd", NULL);
				if (appBarData.hWnd)
				{
					HMONITOR taskbarMonitor = ::MonitorFromWindow(appBarData.hWnd, MONITOR_DEFAULTTOPRIMARY);
					HMONITOR windowMonitor = ::MonitorFromWindow(GetHandle(), MONITOR_DEFAULTTONEAREST);
					if (taskbarMonitor == windowMonitor)
					{
						::SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData);
						edge = appBarData.uEdge;
					}
				}
				
				if (ABE_LEFT == edge)
					mNonClientOffset.left -= 1;
				else if (ABE_RIGHT == edge)
					mNonClientOffset.right -= 1;
				else if (ABE_BOTTOM == edge or ABE_TOP == edge)
					mNonClientOffset.bottom -= 1;
			}
		}
		else if (pl.showCmd != SW_SHOWMINIMIZED)
		{
			if (mNonClientMargin.top and compositionEnabled)
				mNonClientMargin.top = std::min(mCaptionHeight, mNonClientMargin.top);
			else if (mNonClientMargin.top == 0)
				mNonClientMargin.top = mCaptionHeight;
			else	
				mNonClientMargin.top = 0;
			
			if (mNonClientMargin.bottom > 0 and compositionEnabled)
				mNonClientMargin.bottom = std::min(mVerticalResizeMargin, mNonClientMargin.bottom);
			else if (mNonClientMargin.bottom == 0)
				mNonClientMargin.bottom = mVerticalResizeMargin;
			else
				mNonClientMargin.bottom = 0;
			
			if (mNonClientMargin.left > 0 and compositionEnabled)
				mNonClientMargin.left = std::min(mHorizontalResizeMargin, mNonClientMargin.left);
			else if (mNonClientMargin.left == 0)
				mNonClientMargin.left = mHorizontalResizeMargin;
			else
				mNonClientMargin.left = 0;
			
			if (mNonClientMargin.right > 0 and compositionEnabled)
				mNonClientMargin.right = std::min(mHorizontalResizeMargin, mNonClientMargin.right);
			else if (mNonClientMargin.right == 0)
				mNonClientMargin.right = mHorizontalResizeMargin;
			else
				mNonClientMargin.right = 0;
		}

		MARGINS margins;
		margins.cxLeftWidth = mHorizontalResizeMargin;
		margins.cyTopHeight = mCaptionHeight;
		margins.cxRightWidth = mHorizontalResizeMargin;
		margins.cyBottomHeight = mVerticalResizeMargin;
	
		mCallDWP = false;
		if (::DwmExtendFrameIntoClientArea(GetHandle(), &margins) == S_OK)
			mCallDWP = true;
		
		DWMNCRENDERINGPOLICY policy = DWMNCRP_USEWINDOWSTYLE;
		::DwmSetWindowAttribute(GetHandle(), DWMWA_NCRENDERING_POLICY, &policy, sizeof policy);

		result = true;
	}
	else
	{
		mCaptionHeight = mHorizontalResizeMargin = mVerticalResizeMargin = 0;
		result = false;
	}

	return result;
}

void MWinWindowImpl::PaintCustomCaption(HDC inHdc)
{
    wchar_t title[1024]; 
    if (::GetWindowTextW(GetHandle(), title, sizeof(title) / sizeof(wchar_t)) <= 0)
		return;

    RECT rcClient;
    ::GetClientRect(GetHandle(), &rcClient);

    HTHEME hTheme = ::OpenThemeData(NULL, L"CompositedWindow::Window");
    if (hTheme)
    {
        RECT btnRect = rcClient;

        HDC hdcPaint = ::CreateCompatibleDC(inHdc);
        if (hdcPaint)
        {
            int cx = rcClient.right - rcClient.left;
//            int cy = rcClient.bottom - rcClient.top;
			int cy = mCaptionHeight;

            // Define the BITMAPINFO structure used to draw text.
            // Note that biHeight is negative. This is done because
            // DrawThemeTextEx() needs the bitmap to be in top-to-bottom
            // order.
            BITMAPINFO dib = { 0 };
            dib.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
            dib.bmiHeader.biWidth           = cx;
            dib.bmiHeader.biHeight          = -cy;
            dib.bmiHeader.biPlanes          = 1;
            dib.bmiHeader.biBitCount        = BIT_COUNT;
            dib.bmiHeader.biCompression     = BI_RGB;

			void* ppvBits = nullptr;

            HBITMAP hbm = ::CreateDIBSection(inHdc, &dib, DIB_RGB_COLORS, &ppvBits, NULL, 0);
            if (hbm)
            {
                HBITMAP hbmOld = (HBITMAP)::SelectObject(hdcPaint, hbm);

                // Setup the theme drawing options.
                DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
                DttOpts.iGlowSize = 15;

                // Select a font.
                LOGFONT lgFont;
                HFONT hFontOld = NULL, hFont = NULL;
                if (SUCCEEDED(::GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
                {
                    hFont = ::CreateFontIndirect(&lgFont);
                    if (hFont)
	                    hFontOld = (HFONT)::SelectObject(hdcPaint, hFont);
                }
                
                // Draw the button
                
                btnRect.left += mHorizontalResizeMargin;
                btnRect.bottom = btnRect.top + 20;
                btnRect.right = btnRect.left + 75;
                
//                ::DrawThemeEdge(hTheme, hdcPaint, WP_CLOSEBUTTON, 0, &btnRect,
//                	BDR_SUNKENOUTER,
//                	BF_LEFT | BF_BOTTOM | BF_RIGHT, nullptr);
                
                // Draw the title.
                RECT rcPaint = rcClient;
                rcPaint.top += mVerticalResizeMargin;
                rcPaint.right -= 125;
//                rcPaint.left += mHorizontalResizeMargin;
                rcPaint.left = btnRect.right + 2 * mHorizontalResizeMargin;
                rcPaint.bottom = mCaptionHeight;
                ::DrawThemeTextEx(hTheme, hdcPaint, 0, 0, title, -1, 
                	DT_LEFT | DT_WORD_ELLIPSIS, &rcPaint, &DttOpts);

                // Blit text to the frame.
                ::BitBlt(inHdc, 0, 0, cx, cy, hdcPaint, 0, 0, SRCCOPY);

                ::SelectObject(hdcPaint, hbmOld);
                if (hFont)
                {
                    ::SelectObject(hdcPaint, hFontOld);
                    ::DeleteObject(hFont);
                }
                
                ::SelectObject(hdcPaint, hbmOld);
                ::DeleteObject(hbm);
            }
            ::DeleteDC(hdcPaint);
        }

//		HRESULT hr = ::DrawThemeEdge(hTheme, inHdc, WP_MINBUTTON, MINBS_NORMAL, &btnRect, EDGE_ETCHED,
//			BF_ADJUST, &btnRect);

	
//		::DrawThemeParentBackground(GetHandle(), inHdc, &btnRect);
		::DrawThemeBackground(hTheme, inHdc, WP_CAPTION, 0, &btnRect, nullptr);
        
        ::CloseThemeData(hTheme);
    }
}

// --------------------------------------------------------------------

MWindowImpl* MWindowImpl::Create(const string& inTitle, MRect inBounds,
	MWindowFlags inFlags, const string& inMenu, MWindow* inWindow)
{
	MWinWindowImpl* result = new MWinWindowImpl(inFlags, inMenu, inWindow);
	result->Create(inBounds, c2w(inTitle));
	return result;
}

// --------------------------------------------------------------------

void MWindow::GetMainScreenBounds(
	MRect&			outBounds)
{
	RECT bounds;

	if (::SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&bounds, 0))
		outBounds = MRect{bounds.left, bounds.top,
			bounds.right - bounds.left, bounds.bottom - bounds.top};
	else
	{
		/* Get the root DC */
		HDC lDC = ::GetDC(nullptr);
	
		/* Set the outBounds */
		outBounds.x= 0;
		outBounds.width = ::GetDeviceCaps(lDC, HORZRES);
		outBounds.y= 0;
		outBounds.height = ::GetDeviceCaps(lDC, VERTRES);
	
		/* Release the DC. Don't know if thats needed with the screen DC */
		::ReleaseDC(nullptr, lDC);
	}
}

