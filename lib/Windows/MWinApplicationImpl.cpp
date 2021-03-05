//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "MUtils.hpp"

#include "MWinApplicationImpl.hpp"
#include "MWinUtils.hpp"
#include "MWinWindowImpl.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MAcceleratorTable.hpp"
#include "MAlerts.hpp"

#pragma comment (lib, "comctl32")

using namespace std;
namespace fs = boost::filesystem;

COLORREF kDialogBackgroundColorRef((COLORREF)::GetSysColor(COLOR_BTNFACE));
const MColor kDialogBackgroundColor(
			static_cast<uint8>((kDialogBackgroundColorRef >>  0) & 0x000000FF),
			static_cast<uint8>((kDialogBackgroundColorRef >>  8) & 0x000000FF),
			static_cast<uint8>((kDialogBackgroundColorRef >> 16) & 0x000000FF));

#ifdef UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

MWinApplicationImpl* MWinApplicationImpl::sInstance;

MWinApplicationImpl::MWinApplicationImpl(
	HINSTANCE			inInstance)
	: mInstance(inInstance)
{
	sInstance = this;
}

void MWinApplicationImpl::Initialise()
{
	wchar_t path[MAX_PATH] = {};

	if (::GetModuleFileName(NULL, path, MAX_PATH) > 0)
		gExecutablePath = w2c(path);

	gPrefsDir = fs::path(GetPrefsDirectory());

//    HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	// use OleInitialize since we're going to do drag&drop
	HRESULT hr = ::OleInitialize(nullptr);

	INITCOMMONCONTROLSEX info = {
		sizeof(INITCOMMONCONTROLSEX),

		//ICC_ANIMATE_CLASS |
		ICC_BAR_CLASSES |
		ICC_COOL_CLASSES |
		//ICC_DATE_CLASSES |
		//ICC_HOTKEY_CLASS |
		//ICC_INTERNET_CLASSES |
		//ICC_LINK_CLASS |
		ICC_LISTVIEW_CLASSES |
		ICC_NATIVEFNTCTL_CLASS |
		ICC_PAGESCROLLER_CLASS |
		ICC_PROGRESS_CLASS |
		ICC_STANDARD_CLASSES |
		ICC_TAB_CLASSES |
		ICC_TREEVIEW_CLASSES |
		ICC_UPDOWN_CLASS |
		ICC_USEREX_CLASSES |
		ICC_WIN95_CLASSES
	};

	::InitCommonControlsEx(&info);
}

MWinApplicationImpl::~MWinApplicationImpl()
{
}

int MWinApplicationImpl::RunEventLoop()
{
	UINT_PTR t = ::SetTimer(NULL, 0, 10, nullptr);

	MSG message;
	int result = 0;

	// Main message loop:
	for (;;)
	{
		result = ::GetMessage(&message, NULL, 0, 0);
		
		if (result <= 0)
		{
			if (result < 0)
				result = message.wParam;
			break;
		}

		if (message.message == WM_TIMER and message.wParam == t)
		{
			Pulse();
			continue;
		}

		HWND front = ::GetActiveWindow();
		if (front != nullptr)
		{
			MWinWindowImpl* impl = dynamic_cast<MWinWindowImpl*>(MWinProcMixin::Fetch(front));
			if (impl != nullptr and impl->IsDialogMessage(message))
				continue;
		}
		
		if (message.message == WM_KEYDOWN and IsAcceleratorKeyDown(message))
			continue;

		TranslateAndDispatch(message);
	}

	sInstance = nullptr;

	return result;
}

void MWinApplicationImpl::TranslateAndDispatch(MSG& inMessage)
{
	bool handled = false;
	
	if (inMessage.message == WM_SYSKEYDOWN)
	{
		MWinProcMixin* mixin = MWinProcMixin::Fetch(inMessage.hwnd);
		
		LRESULT result;
		if (mixin != nullptr and mixin->WMKeydown(inMessage.hwnd, inMessage.message, inMessage.wParam, inMessage.lParam, result))
			handled = true;
	}
	
	if (not handled)
	{
		::TranslateMessage(&inMessage);
		::DispatchMessage(&inMessage);
	}
}

void MWinApplicationImpl::Quit()
{
	::PostQuitMessage(0);
}

void MWinApplicationImpl::Pulse()
{
	try
	{
		if (gApp != nullptr)
			gApp->Pulse();
	}
	catch (exception& e)
	{
		DisplayError(e);
	}
}

bool MWinApplicationImpl::IsAcceleratorKeyDown(MSG& inMessage)
{
	uint32 keyCode = 0, modifiers = 0;
	string text;

	GetModifierState(modifiers, false);
	if (inMessage.lParam & (1 << 24))
		modifiers |= kNumPad;
	
	switch (inMessage.wParam)
	{
		case VK_HOME:
			keyCode = kHomeKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_ESCAPE:
			keyCode = kEscapeKeyCode;
			break;
		case VK_END:
			keyCode = kEndKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_NEXT:
			keyCode = kPageDownKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_PRIOR:
			keyCode = kPageUpKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_LEFT:
			keyCode = kLeftArrowKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_RIGHT:
			keyCode = kRightArrowKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_UP:
			keyCode = kUpArrowKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_DOWN:
			keyCode = kDownArrowKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_DELETE:
			keyCode = kDeleteKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_INSERT:
			keyCode = kInsertKeyCode;
			modifiers ^= kNumPad;
			break;
		case VK_CONTROL:
		case VK_SHIFT:
			keyCode = 0;
			break;
		case VK_TAB:
			if (modifiers & kControlKey)
				keyCode = kTabKeyCode;
			break;
		case VK_BACK:
			if (modifiers & kControlKey)
				keyCode = kBackspaceKeyCode;
			break;
		case VK_RETURN:
			if (modifiers & kControlKey)
				keyCode = kReturnKeyCode;
			break;
		case VK_PAUSE:
			keyCode = kPauseKeyCode;
			break;
		case VK_CANCEL:
			keyCode = kCancelKeyCode;
			break;
		case VK_NUMLOCK:
			keyCode = kNumlockKeyCode;
			break;
		case VK_DIVIDE:
			keyCode = kDivideKeyCode;
			modifiers |= kNumPad;
			break;
		case VK_MULTIPLY:
			keyCode = kMultiplyKeyCode;
			modifiers |= kNumPad;
			break;
		case VK_SUBTRACT:
			keyCode = kSubtractKeyCode;
			modifiers |= kNumPad;
			break;
		default:
			if (inMessage.wParam >= VK_NUMPAD0 and inMessage.wParam <= VK_DIVIDE)
				modifiers |= kNumPad;
			if (inMessage.wParam >= VK_F1 and inMessage.wParam <= VK_F24)
				keyCode = static_cast<unsigned short>(
					0x0101 + inMessage.wParam - VK_F1);
			else 
				keyCode = static_cast<unsigned short>(
					::MapVirtualKeyW(inMessage.wParam, MAPVK_VK_TO_CHAR));
			break;
	}
	
	//// fetch the original character in case we have control-shift combinations:
	//if (modifiers & kControlKey and modifiers & kShiftKey)
	//{
	//	WORD ch = 0;
	//	uint8 keystate[256] = {};
	//	keystate[VK_SHIFT] = 0x80;
	//	uint32 vsc = ::MapVirtualKeyW(inMessage.wParam, MAPVK_VK_TO_VSC);
	//	if (::ToAscii(inMessage.wParam, vsc, keystate, &ch, 0) == 1)
	//		keyCode = ch;
	//}

	bool result = false;
	uint32 cmd;

	if (MAcceleratorTable::Instance().IsAcceleratorKey(keyCode, modifiers, cmd))
	{
		result = true;
		
		// so this is an accelerator keydown, now find a target
		
		MWinProcMixin* mixin = nullptr;
		HWND hwnd = inMessage.hwnd;
		
		bool handled = false;
		while (hwnd != nullptr)
		{
			mixin = MWinProcMixin::Fetch(hwnd);
			if (mixin != nullptr and mixin->DispatchCommand(cmd, modifiers))
			{
				handled = true;
				break;
			}
			hwnd = ::GetParent(hwnd);
		}
		
		if (not handled)
		{
			bool enabled = false, checked = false;
			if (gApp->UpdateCommandStatus(cmd, nullptr, 0, enabled, checked) and enabled)
				gApp->ProcessCommand(cmd, nullptr, 0, modifiers);
		}
	}

	return result;
}
