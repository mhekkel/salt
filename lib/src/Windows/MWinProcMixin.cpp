//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "MWinProcMixin.hpp"
#include "MError.hpp"
#include "MWinApplicationImpl.hpp"
#include "MWinControlsImpl.hpp"
#include "MUtils.hpp"
#include "MWinUtils.hpp"
#include "MUnicode.hpp"
#include "MAcceleratorTable.hpp"
#include "MAlerts.hpp"
#include "MError.hpp"

using namespace std;

MWinProcMixin::MWinProcMixin(MHandler* inHandler)
	: mHandle(nullptr)
	, mHandler(inHandler)
	, mOldWinProc(nullptr)
	, mHandledKeyDown(false)
{
	AddHandler(WM_DESTROY,		std::bind(&MWinProcMixin::WMDestroy, this, _1, _2, _3, _4, _5));
	AddHandler(WM_KEYDOWN,		std::bind(&MWinProcMixin::WMKeydown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_SYSKEYDOWN,	std::bind(&MWinProcMixin::WMKeydown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_CHAR,			std::bind(&MWinProcMixin::WMChar, this, _1, _2, _3, _4, _5));
	AddHandler(WM_UNICHAR,		std::bind(&MWinProcMixin::WMUniChar, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSEWHEEL,	std::bind(&MWinProcMixin::WMMouseWheel, this, _1, _2, _3, _4, _5));
	AddHandler(WM_SETFOCUS,		std::bind(&MWinProcMixin::WMSetFocus, this, _1, _2, _3, _4, _5));
	AddHandler(WM_KILLFOCUS,	std::bind(&MWinProcMixin::WMKillFocus, this, _1, _2, _3, _4, _5));
	AddHandler(WM_QUERYENDSESSION,
								std::bind(&MWinProcMixin::WMQueryEndSession, this, _1, _2, _3, _4, _5));
	AddHandler(WM_ENDSESSION,	std::bind(&MWinProcMixin::WMEndSession, this, _1, _2, _3, _4, _5));
}

MWinProcMixin::~MWinProcMixin()
{
	SetHandle(nullptr);
}

void MWinProcMixin::SetHandle(HWND inHandle)
{
	if (mHandle != inHandle)
	{
		if (mHandle != nullptr)
			RemovePropW(mHandle, L"m_window_imp");
		mHandle = inHandle;
		if (mHandle != nullptr)
			SetPropW(mHandle, L"m_window_imp", this);
	}
}

MWinProcMixin* MWinProcMixin::Fetch(HWND inHandle)
{
	MWinProcMixin* result = nullptr;
	if (inHandle != nullptr)
		result = reinterpret_cast<MWinProcMixin*>(::GetPropW(inHandle, L"m_window_imp"));
	return result;
}

void MWinProcMixin::CreateHandle(MWinProcMixin* inParent, MRect inBounds, const wstring& inTitle)
{
	DWORD		style = WS_CHILD | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;
	DWORD		exStyle = 0;
	HWND		parent = nullptr;
	HMENU		menu = nullptr;
	wstring		className;

	if (inParent != nullptr)
		parent = inParent->GetHandle();

	CreateParams(style, exStyle, className, menu);

	if (inBounds.x != CW_USEDEFAULT and inBounds.y != CW_USEDEFAULT)
	{
		RECT r = { inBounds.x, inBounds.y, inBounds.x + inBounds.width, inBounds.y + inBounds.height };
		::AdjustWindowRect(&r, style, menu != nullptr);
		inBounds = MRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
	}
	else
	{
		RECT r = { 0, 0, inBounds.width, inBounds.height };
		::AdjustWindowRect(&r, style, menu != nullptr);
		inBounds = MRect(CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top);
	}

	HINSTANCE instance = MWinApplicationImpl::GetInstance()->GetHInstance();
	WNDCLASSEXW lWndClass = { sizeof(WNDCLASSEXW) };
	lWndClass.lpszClassName = className.c_str();

	if (not ::GetClassInfoExW(instance, lWndClass.lpszClassName, &lWndClass))
	{
		lWndClass.lpfnWndProc = &MWinProcMixin::WinProcCallBack;
		RegisterParams(lWndClass.style, lWndClass.cbWndExtra,
			lWndClass.hCursor, lWndClass.hIcon,
			lWndClass.hIconSm, lWndClass.hbrBackground);

		ATOM a = ::RegisterClassExW(&lWndClass);
		if (a == 0)
			throw MException("Failed to register window class");
			//ThrowIfOSErr((::GetLastError()));
	}

	HWND handle = ::CreateWindowExW(exStyle,
		lWndClass.lpszClassName, inTitle.c_str(),
		style,
		inBounds.x, inBounds.y, inBounds.width, inBounds.height,
		parent, menu, instance, this);

	if (handle == nullptr)
		THROW_WIN_ERROR(("Error creating window"));

	SetHandle(handle);
}

void MWinProcMixin::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	outStyle = WS_CHILD | WS_CLIPSIBLINGS;
}

void MWinProcMixin::RegisterParams(UINT& outStyle, int& outWndExtra, HCURSOR& outCursor,
	HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground)
{
	outStyle = CS_VREDRAW | CS_HREDRAW;
	outCursor = ::LoadCursorW(0, IDC_ARROW);
}

void MWinProcMixin::SubClass()
{
	mOldWinProc = (WNDPROC)::GetWindowLongPtr(GetHandle(), GWLP_WNDPROC);
	if (mOldWinProc != &MWinProcMixin::WinProcCallBack)
		::SetWindowLongPtr(GetHandle(), GWLP_WNDPROC, (long)&MWinProcMixin::WinProcCallBack);
	else
		mOldWinProc = nullptr;
}

bool MWinProcMixin::WMDestroy(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	SetHandle(nullptr);
	outResult = 0;
	return true;
}

bool MWinProcMixin::WMKeydown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	uint32_t keyCode = 0, modifiers = 0, scanCode = static_cast<uint8_t>(inLParam >> 16);
	string text;

	GetModifierState(modifiers, false);
	if (inLParam & (1 << 24))
		modifiers ^= kNumPad;

	// We ignore the num lock flag here

	BYTE state[256] = {};
	if (::GetKeyboardState(state) and state[VK_NUMLOCK] == 0 and (inLParam & (1 << 24)) == 0)
	{
		switch (inWParam)
		{
			case VK_HOME:			keyCode = '7';					modifiers |= kNumPad;			break;
			case VK_UP:				keyCode = '8';					modifiers |= kNumPad;			break;
			case VK_PRIOR:			keyCode = '9';					modifiers |= kNumPad;			break;
			case VK_LEFT:			keyCode = '4';					modifiers |= kNumPad;			break;
			case VK_CLEAR:			keyCode = '5';					modifiers |= kNumPad;			break;
			case VK_RIGHT:			keyCode = '6';					modifiers |= kNumPad;			break;
			case VK_END:			keyCode = '1';					modifiers |= kNumPad;			break;
			case VK_DOWN:			keyCode = '2';					modifiers |= kNumPad;			break;
			case VK_NEXT:			keyCode = '3';					modifiers |= kNumPad;			break;
			case VK_INSERT:			keyCode = '0';					modifiers |= kNumPad;			break;
			case VK_DELETE:			keyCode = '.';					modifiers |= kNumPad;			break;
			//case VK_RETURN:			keyCode = kEnterKeyCode;		modifiers |= kNumPad;			break;
			//case VK_ADD:			keyCode = '+';					modifiers |= kNumPad;			break;
			//case VK_DIVIDE:			keyCode = kDivideKeyCode;		modifiers |= kNumPad;			break;
			//case VK_MULTIPLY:		keyCode = kMultiplyKeyCode;		modifiers |= kNumPad;			break;
			//case VK_SUBTRACT:		keyCode = kSubtractKeyCode;		modifiers |= kNumPad;			break;
		}
	}
	
	if (keyCode == 0)
	{
		switch (inWParam)
		{
			case VK_HOME:			keyCode = kHomeKeyCode;			modifiers ^= kNumPad;			break;
			case VK_ESCAPE:			keyCode = kEscapeKeyCode;										break;
			case VK_END:			keyCode = kEndKeyCode;			modifiers ^= kNumPad;			break;
			case VK_NEXT:			keyCode = kPageDownKeyCode;		modifiers ^= kNumPad;			break;
			case VK_PRIOR:			keyCode = kPageUpKeyCode;		modifiers ^= kNumPad;			break;
			case VK_LEFT:			keyCode = kLeftArrowKeyCode;	modifiers ^= kNumPad;			break;
			case VK_RIGHT:			keyCode = kRightArrowKeyCode;	modifiers ^= kNumPad;			break;
			case VK_UP:				keyCode = kUpArrowKeyCode;		modifiers ^= kNumPad;			break;
			case VK_DOWN:			keyCode = kDownArrowKeyCode;	modifiers ^= kNumPad;			break;
			case VK_DELETE:			keyCode = kDeleteKeyCode;		modifiers ^= kNumPad;			break;
			case VK_INSERT:			keyCode = kInsertKeyCode;		modifiers ^= kNumPad;			break;
			case VK_TAB:			keyCode = kTabKeyCode;											break;
			case VK_BACK:			keyCode = kBackspaceKeyCode;									break;
			case VK_RETURN:			keyCode = modifiers & kNumPad ? kEnterKeyCode : kReturnKeyCode;	break;
			case VK_PAUSE:			keyCode = kPauseKeyCode;										break;
			case VK_CANCEL:			keyCode = kCancelKeyCode;										break;

			case VK_NUMPAD0:		keyCode = '0';					modifiers |= kNumPad;			break;
			case VK_NUMPAD1:		keyCode = '1';					modifiers |= kNumPad;			break;
			case VK_NUMPAD2:		keyCode = '2';					modifiers |= kNumPad;			break;
			case VK_NUMPAD3:		keyCode = '3';					modifiers |= kNumPad;			break;
			case VK_NUMPAD4:		keyCode = '4';					modifiers |= kNumPad;			break;
			case VK_NUMPAD5:		keyCode = '5';					modifiers |= kNumPad;			break;
			case VK_NUMPAD6:		keyCode = '6';					modifiers |= kNumPad;			break;
			case VK_NUMPAD7:		keyCode = '7';					modifiers |= kNumPad;			break;
			case VK_NUMPAD8:		keyCode = '8';					modifiers |= kNumPad;			break;
			case VK_NUMPAD9:		keyCode = '9';					modifiers |= kNumPad;			break;
			case VK_NUMLOCK:		keyCode = kNumlockKeyCode;		modifiers |= kNumPad;			break;
			case VK_MULTIPLY:		keyCode = kMultiplyKeyCode;		modifiers |= kNumPad;			break;
			case VK_ADD:			keyCode = '+';					modifiers |= kNumPad;			break;
			//case VK_SEPARATOR:		keyCode = kEnterKeyCode;		modifiers |= kNumPad;			break;
			case VK_SUBTRACT:		keyCode = kSubtractKeyCode;		modifiers |= kNumPad;			break;
			case VK_DECIMAL:		keyCode = '.';					modifiers |= kNumPad;			break;
			case VK_DIVIDE:			keyCode = kDivideKeyCode;		modifiers |= kNumPad;			break;

			default:
				if (inWParam >= VK_F1 and inWParam <= VK_F24)
					keyCode = static_cast<unsigned short>(0x0101 + inWParam - VK_F1);
				else if (inWParam >= '0' and inWParam <= '9' and (modifiers & kOptionKey) == 0)
					keyCode = inWParam;
				//else 
				//	keyCode = static_cast<unsigned short>(::MapVirtualKeyW(inWParam, MAPVK_VK_TO_CHAR));
				break;
		}
	}
	
	if (keyCode == 0 and modifiers & kOptionKey)
	{
		WORD ch;
		if (::ToAscii(inWParam, inLParam, state, &ch, 0) == 1)
			keyCode = (ch & 0x00ff);
	}
	
	bool repeat = (inLParam & (1 << 30)) != 0;
	mHandledKeyDown = false;

	// some special cases
	if (modifiers == kControlKey)
	{
		mHandledKeyDown = true;
		switch (inWParam)
		{
			case VK_SPACE:		DispatchCharacter(string(1, '\x00'), repeat); break;
			case VK_OEM_4:		DispatchCharacter(string(1, '\x1b'), repeat); break;
			case VK_OEM_5:		DispatchCharacter(string(1, '\x1c'), repeat); break;
			case VK_OEM_6:		DispatchCharacter(string(1, '\x1d'), repeat); break;
			case VK_OEM_3:		DispatchCharacter(string(1, '\x1e'), repeat); break;
			case VK_OEM_2:		DispatchCharacter(string(1, '\x1f'), repeat); break;
			default:			mHandledKeyDown = false; break;
		}
	}

	if (not mHandledKeyDown and keyCode != 0 and DispatchKeyDown(keyCode, modifiers, repeat))
	{
		mHandledKeyDown = true;
		outResult = 0;
	}
	
	return mHandledKeyDown;
}

bool MWinProcMixin::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = false;
	
	MWinProcMixin* p = Fetch(::GetParent(GetHandle()));
	if (p != nullptr)
		result = p->DispatchKeyDown(inKeyCode, inModifiers, inRepeat);
	else
		result = gApp->HandleKeyDown(inKeyCode, inModifiers, inRepeat);
	
	return result;
}

bool MWinProcMixin::WMChar(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	if (mHandledKeyDown)
		result = true;
	else
	{
		wchar_t ch = inWParam;
		wstring text(&ch, 1);
		
		outResult = 1;
		bool repeat = (inLParam & (1 << 30)) != 0;
	
		if (DispatchCharacter(w2c(text), repeat))
		{
			result = true;
			outResult = 0;
		}
	}
	
	return result;
}

bool MWinProcMixin::WMUniChar(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;

	if (mHandledKeyDown)
		result = true;
	else if (inWParam != UNICODE_NOCHAR)
	{
		wchar_t ch = inWParam;
		wstring text(&ch, 1);

		outResult = 1;
		bool repeat = (inLParam & (1 << 30)) != 0;

		if (DispatchCharacter(w2c(text), repeat))
		{
			result = true;
			outResult = 0;
		}
	}
	
	return result;
}

bool MWinProcMixin::DispatchCharacter(const string& inText, bool inRepeat)
{
	bool result = false;
	
	MWinProcMixin* p = Fetch(::GetParent(GetHandle()));
	if (p != nullptr)
		result = p->DispatchCharacter(inText, inRepeat);
	
	return result;
}

bool MWinProcMixin::DispatchCommand(uint32_t inCommand, uint32_t inModifiers)
{
	return false;
}

bool MWinProcMixin::WMDrawItem(DRAWITEMSTRUCT* inDrawItemStruct)
{
	return false;
}

bool MWinProcMixin::WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	return false;
}

bool MWinProcMixin::WMMouseWheel(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool result = false;
	
	MWinProcMixin* parent = Fetch(::GetParent(GetHandle()));
	if (parent != nullptr)
		result = parent->WMMouseWheel(inHWnd, inUMsg, inWParam, inLParam, outResult);
	
	return result;
}

bool MWinProcMixin::WMSetFocus(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
	LPARAM inLParam, LRESULT& outResult)
{
	if (mHandler != nullptr)
	{
		mHandler->BeFocus();
		outResult = 0;
	}

	return false;
}

bool MWinProcMixin::WMKillFocus(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
	LPARAM inLParam, LRESULT& outResult)
{
	if (mHandler != nullptr)
	{
		mHandler->DontBeFocus();
		outResult = 0;
	}

	return false;
}

bool MWinProcMixin::WMQueryEndSession(HWND inHWnd, UINT /*inUMsg*/, WPARAM /*inWParam*/, LPARAM /*inLParam*/, LRESULT& outResult)
{
//	PRINT(("QueryEndSession, %p", inHWnd));

	outResult = gApp->AllowQuit(true) ? TRUE : FALSE;

	return true;
}

bool MWinProcMixin::WMEndSession(HWND inHWnd, UINT /*inUMsg*/, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
//	PRINT(("EndSession, %p", inHWnd));

	if (inWParam)
	{
		gApp->DoQuit();
		_exit(0);
	}
	else
		gApp->CancelQuit();

	outResult = 1;
	return true;
}

LRESULT MWinProcMixin::WinProc(HWND inHandle, UINT inMsg, WPARAM inWParam, LPARAM inLParam)
{
	LRESULT result = 0;

//	LogWinMsg("WinProc", inMsg);
	assert(mHandle);

	try
	{
		MWMCall call;
		{
			MHandlerTable::iterator i = mHandlers.find(inMsg);
			if (i != mHandlers.end())
				call = i->second;
		}

		if (not call or not call(inHandle, inMsg, inWParam, inLParam, result))
			result = DefProc(inHandle, inMsg, inWParam, inLParam);
	}
	catch (exception& e)
	{
		DisplayError(e);
	}

	return result;
}

LRESULT MWinProcMixin::DefProc(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam)
{
	LRESULT result = 0;
	bool handled = false;
	
	switch (inUMsg)
	{
		case WM_NOTIFY:
		{
			LPNMHDR msg = reinterpret_cast<LPNMHDR>(inLParam);

			MNotifyHandler h = { msg->hwndFrom, msg->code };
			MNotificationTable::iterator i = mNotificationHandlers.find(h);

			if (i != mNotificationHandlers.end())
				handled = i->second(inWParam, inLParam, result);
			break;
		}

		case WM_COMMAND:
			if (inLParam != 0)
			{
				MWinProcMixin* imp = MWinProcMixin::Fetch((HWND)inLParam);
				if (imp != nullptr)
					handled = imp->WMCommand(inHWnd, HIWORD(inWParam), inWParam, inLParam, result);
			}
			break;

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)inLParam;
			MWinProcMixin* m = Fetch(dis->hwndItem);
			if (m != nullptr)
			{
				handled = m->WMDrawItem(dis);
				result = 1;
			}
			break;
		}

		case WM_HSCROLL:
		case WM_VSCROLL:
		{
			MWinScrollbarImpl* scrollbarImpl = dynamic_cast<MWinScrollbarImpl*>(MWinProcMixin::Fetch((HWND)inLParam));
			if (scrollbarImpl != nullptr)
				handled = scrollbarImpl->WMScroll(inHWnd, inUMsg, inWParam, inLParam, result);
			break;
		}

		default:
			break;
	}

	if (not handled)
	{
		if (mOldWinProc != nullptr and mOldWinProc != &MWinProcMixin::WinProcCallBack)
			result = ::CallWindowProcW(mOldWinProc, inHWnd, inUMsg, inWParam, inLParam);
		else
			result = ::DefWindowProcW(inHWnd, inUMsg, inWParam, inLParam);
	}

	return result;
}

LRESULT CALLBACK MWinProcMixin::WinProcCallBack(HWND hwnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 1;
	
	try
	{
		if (uMsg == WM_CREATE)
		{
			CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
			MWinProcMixin* impl = reinterpret_cast<MWinProcMixin*>(cs->lpCreateParams);
			impl->SetHandle(hwnd);
		}

		MWinProcMixin* impl = MWinProcMixin::Fetch(hwnd);

		if (impl != nullptr and impl->mHandle == hwnd)
			result = impl->WinProc(hwnd, uMsg, wParam, lParam);
		else
			result = ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	catch (std::exception& e)
	{
		DisplayError(e);
	}
	catch (...)
	{
		DisplayError(MException("ouch"));
	}
	return result;
}

/*

Just a help when debugging! Cannot find it in the MSDN help :)

#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005
#define WM_ACTIVATE                     0x0006
#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_SETREDRAW                    0x000B
#define WM_SETTEXT                      0x000C
#define WM_GETTEXT                      0x000D
#define WM_GETTEXTLENGTH                0x000E
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#define WM_QUERYENDSESSION              0x0011
#define WM_QUIT                         0x0012
#define WM_QUERYOPEN                    0x0013
#define WM_ERASEBKGND                   0x0014
#define WM_SYSCOLORCHANGE               0x0015
#define WM_ENDSESSION                   0x0016
#define WM_SHOWWINDOW                   0x0018
#define WM_WININICHANGE                 0x001A
#define WM_SETTINGCHANGE                WM_WININICHANGE
#define WM_DEVMODECHANGE                0x001B
#define WM_ACTIVATEAPP                  0x001C
#define WM_FONTCHANGE                   0x001D
#define WM_TIMECHANGE                   0x001E
#define WM_CANCELMODE                   0x001F
#define WM_SETCURSOR                    0x0020
#define WM_MOUSEACTIVATE                0x0021
#define WM_CHILDACTIVATE                0x0022
#define WM_QUEUESYNC                    0x0023
#define WM_GETMINMAXINFO                0x0024
#define WM_PAINTICON                    0x0026
#define WM_ICONERASEBKGND               0x0027
#define WM_NEXTDLGCTL                   0x0028
#define WM_SPOOLERSTATUS                0x002A
#define WM_DRAWITEM                     0x002B
#define WM_MEASUREITEM                  0x002C
#define WM_DELETEITEM                   0x002D
#define WM_VKEYTOITEM                   0x002E
#define WM_CHARTOITEM                   0x002F
#define WM_SETFONT                      0x0030
#define WM_GETFONT                      0x0031
#define WM_SETHOTKEY                    0x0032
#define WM_GETHOTKEY                    0x0033
#define WM_QUERYDRAGICON                0x0037
#define WM_COMPAREITEM                  0x0039
#define WM_GETOBJECT                    0x003D
#define WM_COMPACTING                   0x0041
#define WM_COMMNOTIFY                   0x0044  
#define WM_WINDOWPOSCHANGING            0x0046
#define WM_WINDOWPOSCHANGED             0x0047
#define WM_POWER                        0x0048
#define WM_COPYDATA                     0x004A
#define WM_CANCELJOURNAL                0x004B
#define WM_NOTIFY                       0x004E
#define WM_INPUTLANGCHANGEREQUEST       0x0050
#define WM_INPUTLANGCHANGE              0x0051
#define WM_TCARD                        0x0052
#define WM_HELP                         0x0053
#define WM_USERCHANGED                  0x0054
#define WM_NOTIFYFORMAT                 0x0055
#define WM_CONTEXTMENU                  0x007B
#define WM_STYLECHANGING                0x007C
#define WM_STYLECHANGED                 0x007D
#define WM_DISPLAYCHANGE                0x007E
#define WM_GETICON                      0x007F
#define WM_SETICON                      0x0080
#define WM_NCCREATE                     0x0081
#define WM_NCDESTROY                    0x0082
#define WM_NCCALCSIZE                   0x0083
#define WM_NCHITTEST                    0x0084
#define WM_NCPAINT                      0x0085
#define WM_NCACTIVATE                   0x0086
#define WM_GETDLGCODE                   0x0087
#define WM_SYNCPAINT                    0x0088
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCLBUTTONDOWN                0x00A1
#define WM_NCLBUTTONUP                  0x00A2
#define WM_NCLBUTTONDBLCLK              0x00A3
#define WM_NCRBUTTONDOWN                0x00A4
#define WM_NCRBUTTONUP                  0x00A5
#define WM_NCRBUTTONDBLCLK              0x00A6
#define WM_NCMBUTTONDOWN                0x00A7
#define WM_NCMBUTTONUP                  0x00A8
#define WM_NCMBUTTONDBLCLK              0x00A9
#define WM_KEYFIRST                     0x0100
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_DEADCHAR                     0x0103
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_SYSDEADCHAR                  0x0107
#define WM_KEYLAST                      0x0108
#define WM_IME_STARTCOMPOSITION         0x010D
#define WM_IME_ENDCOMPOSITION           0x010E
#define WM_IME_COMPOSITION              0x010F
#define WM_IME_KEYLAST                  0x010F
#define WM_INITDIALOG                   0x0110
#define WM_COMMAND                      0x0111
#define WM_SYSCOMMAND                   0x0112
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115
#define WM_INITMENU                     0x0116
#define WM_INITMENUPOPUP                0x0117
#define WM_MENUSELECT                   0x011F
#define WM_MENUCHAR                     0x0120
#define WM_ENTERIDLE                    0x0121
#define WM_MENURBUTTONUP                0x0122
#define WM_MENUDRAG                     0x0123
#define WM_MENUGETOBJECT                0x0124
#define WM_UNINITMENUPOPUP              0x0125
#define WM_MENUCOMMAND                  0x0126
#define WM_CTLCOLORMSGBOX               0x0132
#define WM_CTLCOLOREDIT                 0x0133
#define WM_CTLCOLORLISTBOX              0x0134
#define WM_CTLCOLORBTN                  0x0135
#define WM_CTLCOLORDLG                  0x0136
#define WM_CTLCOLORSCROLLBAR            0x0137
#define WM_CTLCOLORSTATIC               0x0138
#define WM_MOUSEFIRST                   0x0200
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A
#define WM_MOUSELAST                    0x020A
#define WM_MOUSELAST                    0x0209
#define WM_PARENTNOTIFY                 0x0210
#define WM_ENTERMENULOOP                0x0211
#define WM_EXITMENULOOP                 0x0212
#define WM_NEXTMENU                     0x0213
#define WM_SIZING                       0x0214
#define WM_CAPTURECHANGED               0x0215
#define WM_MOVING                       0x0216
#define WM_POWERBROADCAST               0x0218      // r_winuser pbt
#define WM_DEVICECHANGE                 0x0219
#define WM_MDICREATE                    0x0220
#define WM_MDIDESTROY                   0x0221
#define WM_MDIACTIVATE                  0x0222
#define WM_MDIRESTORE                   0x0223
#define WM_MDINEXT                      0x0224
#define WM_MDIMAXIMIZE                  0x0225
#define WM_MDITILE                      0x0226
#define WM_MDICASCADE                   0x0227
#define WM_MDIICONARRANGE               0x0228
#define WM_MDIGETACTIVE                 0x0229
#define WM_MDISETMENU                   0x0230
#define WM_ENTERSIZEMOVE                0x0231
#define WM_EXITSIZEMOVE                 0x0232
#define WM_DROPFILES                    0x0233
#define WM_MDIREFRESHMENU               0x0234
#define WM_IME_SETCONTEXT               0x0281
#define WM_IME_NOTIFY                   0x0282
#define WM_IME_CONTROL                  0x0283
#define WM_IME_COMPOSITIONFULL          0x0284
#define WM_IME_SELECT                   0x0285
#define WM_IME_CHAR                     0x0286
#define WM_IME_REQUEST                  0x0288
#define WM_IME_KEYDOWN                  0x0290
#define WM_IME_KEYUP                    0x0291
#define WM_MOUSEHOVER                   0x02A1
#define WM_MOUSELEAVE                   0x02A3
#define WM_CUT                          0x0300
#define WM_COPY                         0x0301
#define WM_PASTE                        0x0302
#define WM_CLEAR                        0x0303
#define WM_UNDO                         0x0304
#define WM_RENDERFORMAT                 0x0305
#define WM_RENDERALLFORMATS             0x0306
#define WM_DESTROYCLIPBOARD             0x0307
#define WM_DRAWCLIPBOARD                0x0308
#define WM_PAINTCLIPBOARD               0x0309
#define WM_VSCROLLCLIPBOARD             0x030A
#define WM_SIZECLIPBOARD                0x030B
#define WM_ASKCBFORMATNAME              0x030C
#define WM_CHANGECBCHAIN                0x030D
#define WM_HSCROLLCLIPBOARD             0x030E
#define WM_QUERYNEWPALETTE              0x030F
#define WM_PALETTEISCHANGING            0x0310
#define WM_PALETTECHANGED               0x0311
#define WM_HOTKEY                       0x0312
#define WM_PRINT                        0x0317
#define WM_PRINTCLIENT                  0x0318
#define WM_HANDHELDFIRST                0x0358
#define WM_HANDHELDLAST                 0x035F
#define WM_AFXFIRST                     0x0360
#define WM_AFXLAST                      0x037F
#define WM_PENWINFIRST                  0x0380
#define WM_PENWINLAST                   0x038F

*/
