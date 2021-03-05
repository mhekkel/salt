//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include <fstream>
#include <boost/bind.hpp>

#include "zeep/xml/document.hpp"

#include "MWinMenuImpl.hpp"
#include "MWinWindowImpl.hpp"
#include "MError.hpp"
#include "MWinUtils.hpp"
#include "MAcceleratorTable.hpp"

using namespace std;
using namespace zeep;

MWinMenuImpl::MWinMenuImpl(MMenu* inMenu, bool inPopup)
	: MMenuImpl(inMenu)
{
	if (inPopup)
		mMenuHandle = ::CreatePopupMenu();
	else
		mMenuHandle = ::CreateMenu();

	MENUINFO info = { sizeof(MENUINFO) };
	::GetMenuInfo(mMenuHandle, &info);
	info.fMask |= MIM_STYLE | MIM_MENUDATA;
	info.dwStyle |= MNS_NOTIFYBYPOS;
	info.dwMenuData = (ULONG_PTR)this;
	THROW_IF_WIN_ERROR(::SetMenuInfo(mMenuHandle, &info));
}

MWinMenuImpl::~MWinMenuImpl()
{
	// destroy all sub menus as well
	
	try
	{
		uint32_t cnt = ::GetMenuItemCount(mMenuHandle);
		for (uint32_t i = 0; i < cnt; ++i)
		{
			MENUITEMINFOW info = { sizeof(MENUITEMINFO), MIIM_SUBMENU };
			if (::GetMenuItemInfoW(mMenuHandle, i, true, &info))
			{
				if (info.hSubMenu != nullptr)
				{
					MMenu* menu = Lookup(info.hSubMenu);
					if (menu != nullptr)
						delete menu;
				}
			}
		}
	}
	catch (...) {}	
	
	::DestroyMenu(mMenuHandle);
}

void MWinMenuImpl::SetTarget(
	MHandler*		inHandler)
{
}

void MWinMenuImpl::SetItemState(
	uint32_t				inIndex,
	bool				inEnabled,
	bool				inChecked)
{
	MENUITEMINFOW info = { sizeof(MENUITEMINFOW), MIIM_STATE };
	THROW_IF_WIN_ERROR(::GetMenuItemInfoW(mMenuHandle, inIndex, true, &info));
	
	if (inEnabled and inChecked)
		info.fState = MFS_ENABLED | MFS_CHECKED;
	else if (inEnabled)
		info.fState = MFS_ENABLED | MFS_UNCHECKED;
	else if (inChecked)
		info.fState = MFS_DISABLED | MFS_CHECKED;
	else
		info.fState = MFS_DISABLED | MFS_UNCHECKED;

	THROW_IF_WIN_ERROR(::SetMenuItemInfoW(mMenuHandle, inIndex, true, &info));
}

void MWinMenuImpl::AppendItem(
	const string&	inLabel,
	uint32_t			inCommand)
{
	/* Add it to the fMenuHandle */
	MENUITEMINFOW lWinItem = { sizeof(MENUITEMINFOW) };
	wstring label = c2w(inLabel);

	uint32_t keyCode, modifiers;
	if (MAcceleratorTable::Instance().GetAcceleratorKeyForCommand(inCommand, keyCode, modifiers))
	{
		wstring shortcut;

		if (modifiers & kControlKey)
			shortcut += L"Ctrl";

		if (modifiers & kOptionKey)
		{
			if (not shortcut.empty())
				shortcut += '+';
			shortcut += L"Alt";
		}

		if (modifiers & kShiftKey)
		{
			if (not shortcut.empty())
				shortcut += '+';
			shortcut += L"Shift";
		}

		wstring key;

		switch (keyCode)
		{
			case kHomeKeyCode:				key = L"Home"; break;
			case kCancelKeyCode:			key = L"Break"; break;
			case kEndKeyCode:				key = L"End"; break;
			case kInsertKeyCode:			key = L"Insert"; break;
			//case kBellKeyCode:				key = L"Bell"; break;
			case kBackspaceKeyCode:			key = L"Backspace"; break;
			case kTabKeyCode:				key = L"Tab"; break;
			//case kLineFeedKeyCode:			key = L"break;
			//case kVerticalTabKeyCode:break;
			case kPageUpKeyCode:			key = L"PageUp"; break;
			//case kFormFeedKeyCode:			break;
			case kPageDownKeyCode:			key = L"PageDown"; break;
			case kReturnKeyCode:			key = L"Return"; break;
			//case kFunctionKeyKeyCode:		break;
			case kPauseKeyCode:				key = L"Pause"; break;
			case kEscapeKeyCode:			key = L"Esc"; break;
			//case kClearKeyCode:				break;
			case kLeftArrowKeyCode:			key = L"Left"; break;
			case kRightArrowKeyCode:		key = L"Right"; break;
			case kUpArrowKeyCode:			key = L"Up"; break;
			case kDownArrowKeyCode:			key = L"Down"; break;
			case kSpaceKeyCode:				key = L"Space"; break;
			case kDeleteKeyCode:			key = L"Del"; break;
			case kF1KeyCode:				key = L"F1"; break;
			case kF2KeyCode:				key = L"F2"; break;
			case kF3KeyCode:				key = L"F3"; break;
			case kF4KeyCode:				key = L"F4"; break;
			case kF5KeyCode:				key = L"F5"; break;
			case kF6KeyCode:				key = L"F6"; break;
			case kF7KeyCode:				key = L"F7"; break;
			case kF8KeyCode:				key = L"F8"; break;
			case kF9KeyCode:				key = L"F9"; break;
			case kF10KeyCode:				key = L"F10"; break;
			case kF11KeyCode:				key = L"F11"; break;
			case kF12KeyCode:				key = L"F12"; break;
			case kF13KeyCode:				key = L"F13"; break;
			case kF14KeyCode:				key = L"F14"; break;
			case kF15KeyCode:				key = L"F15"; break;
			default:
				wchar_t ch = ::MapVirtualKeyW(keyCode, MAPVK_VK_TO_CHAR);
				if (ch == 0 and keyCode < 127 and isprint(static_cast<char>(keyCode)))
					ch = keyCode;
				key = ch;
				break;
		}

		if (not key.empty())
		{
			if (shortcut.empty())
				label = label + wchar_t('\t') + key;
			else
				label = label + wchar_t('\t') + shortcut + wchar_t('+') + key;
		}
	}

	lWinItem.fMask = MIIM_ID | MIIM_TYPE;
	lWinItem.fType = MFT_STRING;
	lWinItem.wID = inCommand;
	lWinItem.dwTypeData = (LPWSTR)label.c_str();

	THROW_IF_WIN_ERROR(::InsertMenuItemW(mMenuHandle, ::GetMenuItemCount(mMenuHandle) + 1, TRUE, &lWinItem));
}

void MWinMenuImpl::AppendSubmenu(
	MMenu*			inSubmenu)
{
	/* Add it to the fMenuHandle */
	MENUITEMINFOW lWinItem = { sizeof(MENUITEMINFOW) };
	wstring label = c2w(inSubmenu->GetLabel());
	
	lWinItem.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
	lWinItem.fType = MFT_STRING;
	//lWinItem.wID = inCommand;
	lWinItem.hSubMenu = dynamic_cast<MWinMenuImpl*>(inSubmenu->impl())->GetHandle();
	lWinItem.dwTypeData = (LPWSTR)label.c_str();

	THROW_IF_WIN_ERROR(::InsertMenuItemW(mMenuHandle, ::GetMenuItemCount(mMenuHandle) + 1, TRUE, &lWinItem));
}

void MWinMenuImpl::AppendSeparator()
{
	/* Add it to the fMenuHandle */
	MENUITEMINFOW lWinItem = { sizeof(MENUITEMINFOW) };
	
	lWinItem.fMask = MIIM_TYPE;
	lWinItem.fType = MFT_SEPARATOR;

	THROW_IF_WIN_ERROR(::InsertMenuItemW(mMenuHandle, ::GetMenuItemCount(mMenuHandle) + 1, TRUE, &lWinItem));
}

void MWinMenuImpl::AppendCheckbox(
	const string&	inLabel,
	uint32_t			inCommand)
{
	/* Add it to the fMenuHandle */
	MENUITEMINFOW lWinItem = { sizeof(MENUITEMINFOW) };
	wstring label = c2w(inLabel);
	
	lWinItem.fMask = MIIM_ID | MIIM_TYPE | MIIM_CHECKMARKS;
	lWinItem.fType = MFT_STRING;
	lWinItem.wID = inCommand;
	lWinItem.dwTypeData = (LPWSTR)label.c_str();

	THROW_IF_WIN_ERROR(::InsertMenuItemW(mMenuHandle, ::GetMenuItemCount(mMenuHandle) + 1, TRUE, &lWinItem));
}

void MWinMenuImpl::AppendRadiobutton(
	const string&	inLabel,
	uint32_t			inCommand)
{
	/* Add it to the fMenuHandle */
	MENUITEMINFOW lWinItem = { sizeof(MENUITEMINFOW) };
	wstring label = c2w(inLabel);
	
	lWinItem.fMask = MIIM_ID | MIIM_TYPE | MIIM_CHECKMARKS;
	lWinItem.fType = MFT_STRING | MFT_RADIOCHECK;
	lWinItem.wID = inCommand;
	lWinItem.dwTypeData = (LPWSTR)label.c_str();

	THROW_IF_WIN_ERROR(::InsertMenuItemW(mMenuHandle, ::GetMenuItemCount(mMenuHandle) + 1, TRUE, &lWinItem));
}

uint32_t MWinMenuImpl::CountItems() const
{
	return ::GetMenuItemCount(mMenuHandle);
}

void MWinMenuImpl::RemoveItems(
	uint32_t				inFirstIndex,
	uint32_t				inCount)
{
	for (uint32_t i = inFirstIndex; i < inFirstIndex + inCount; ++i)
		::RemoveMenu(mMenuHandle, inFirstIndex, MF_BYPOSITION);
}

string MWinMenuImpl::GetItemLabel(
	uint32_t			inIndex) const
{
	MENUITEMINFOW info = { sizeof(MENUITEMINFOW), MIIM_STRING };
	THROW_IF_WIN_ERROR(::GetMenuItemInfoW(mMenuHandle, inIndex, true, &info));

	vector<wchar_t> s(info.cch + 1);
	info.dwTypeData = &s[0];
	info.cch += 1;

	THROW_IF_WIN_ERROR(::GetMenuItemInfoW(mMenuHandle, inIndex, true, &info));
	
	return w2c(info.dwTypeData);
}

void MWinMenuImpl::SetItemCommand(uint32_t inIndex, uint32_t inCommand)
{
	MENUITEMINFOW info = { sizeof(MENUITEMINFOW), MIIM_ID };
	::GetMenuItemInfoW(mMenuHandle, inIndex, true, &info);
	info.wID = inCommand;
	::SetMenuItemInfoW(mMenuHandle, inIndex, true, &info);
}

uint32_t MWinMenuImpl::GetItemCommand(
	uint32_t			inIndex) const
{
	MENUITEMINFOW info = { sizeof(MENUITEMINFOW), MIIM_ID };
	::GetMenuItemInfoW(mMenuHandle, inIndex, true, &info);
	
	return info.wID;
}

MMenu* MWinMenuImpl::GetSubmenu(
	uint32_t			inIndex) const
{
	MENUITEMINFOW info = { sizeof(MENUITEMINFOW), MIIM_SUBMENU };
	THROW_IF_WIN_ERROR(::GetMenuItemInfoW(mMenuHandle, inIndex, true, &info));
	
	MMenu* result = nullptr;
	if (info.hSubMenu != nullptr)
		result = Lookup(info.hSubMenu);
	return result;
}

void MWinMenuImpl::Popup(
	MWindow*		inWindow,
	int32_t			inX,
	int32_t			inY,
	bool			inBottomMenu)
{
	MWinWindowImpl* impl = dynamic_cast<MWinWindowImpl*>(inWindow->GetImpl());
	::TrackPopupMenuEx(mMenuHandle, TPM_NONOTIFY, inX, inY, impl->GetHandle(), nullptr);
}

MMenu* MWinMenuImpl::Lookup(
	HMENU			inMenuHandle)
{
	MENUINFO info = { sizeof(MENUINFO), MIM_MENUDATA };
	MMenu* result = nullptr;

	if (::GetMenuInfo(inMenuHandle, &info) and info.dwMenuData != 0)
		result = reinterpret_cast<MWinMenuImpl*>(info.dwMenuData)->mMenu;

	return result;
}

void MWinMenuImpl::AddToWindow(MWindowImpl* inWindow)
{
}

MMenuImpl* MMenuImpl::Create(MMenu* inMenu, bool inPopup)
{
	return new MWinMenuImpl(inMenu, inPopup);
}

MMenuImpl* MMenuImpl::CreateBar(MMenu* inMenu)
{
	return new MWinMenuImpl(inMenu, false);
}

//// --------------------------------------------------------------------
//
//MWinMenubar::MWinMenubar(MWinWindowImpl* inWindowImpl, const char* inMenuResource)
//	: mWindowImpl(inWindowImpl)
//{
//	MRect r(0, 0, 10, 10);
//	//mWindow->GetBounds(r);
//	Create(inWindowImpl, r, L"menubar");
//
//	inWindowImpl->AddNotify(TBN_DROPDOWN, GetHandle(), boost::bind(&MWinMenubar::NDropDown, this, _1, _2, _3));
//
//	vector<TBBUTTON> buttons;
//	list<wstring> labels;
//
//	//mrsrc::rsrc rsrc(string("Menus/") + inResourceName + ".xml");
//	//
//	//if (not rsrc)
//	//	THROW(("Menu resource not found: %s", inResourceName));
//
//	//io::stream<io::array_source> data(rsrc.data(), rsrc.size());
//	ifstream data("C:\\Users\\maarten\\projects\\japi\\Resources\\Menus\\" + string(inMenuResource) + ".xml");
//	xml::document doc(data);
//	
//	// build a menubar from the resource XML
//	for (xml::element* menu: doc.find("/menubar/menu"))
//	{
//		//MMenu* obj = CreateMenu(menu);
//
//		labels.push_back(c2w(menu->get_attribute("label")));
//
//		TBBUTTON btn = {
//			I_IMAGENONE,
//			buttons.size(),
//			TBSTATE_ENABLED,
//			BTNS_DROPDOWN | BTNS_AUTOSIZE,
//			{0},
//			0,
//			(INT_PTR)labels.back().c_str()
//		};
//
//		buttons.push_back(btn);
//
//		mMenus.push_back(MMenu::Create(menu));
//	}
//	
//    // Add buttons.
//    ::SendMessage(GetHandle(), TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
//    ::SendMessage(GetHandle(), TB_ADDBUTTONS, buttons.size(), (LPARAM)&buttons[0]);
//
//    // Tell the toolbar to resize itself, and show it.
//    ::SendMessage(GetHandle(), TB_AUTOSIZE, 0, 0);
//    ::ShowWindow(GetHandle(), TRUE);
//}
//
//MWinMenubar::~MWinMenubar()
//{
//	for (MMenu* menu: mMenus)
//		delete menu;
//}
//
//void MWinMenubar::CreateParams(DWORD& outStyle, DWORD& outExStyle,
//	wstring& outClassName, HMENU& outMenu)
//{
//	MWinProcMixin::CreateParams(outStyle, outExStyle, outClassName, outMenu);
//
//	outClassName = TOOLBARCLASSNAME;
//	outStyle = WS_CHILD | TBSTYLE_LIST | TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_WRAPABLE;
//}
//
//void MWinMenubar::RegisterParams(UINT& outStyle, HCURSOR& outCursor,
//	HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground)
//{
//	MWinProcMixin::RegisterParams(outStyle, outCursor, outIcon, outSmallIcon, outBackground);
//}
//
//bool MWinMenubar::NDropDown(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
//{
//	LPNMTOOLBAR toolbar = reinterpret_cast<LPNMTOOLBAR>(inLParam);
//
//	// Get the coordinates of the button.
//	RECT rc;
//	::SendMessage(GetHandle(), TB_GETRECT, (WPARAM)toolbar->iItem, (LPARAM)&rc);
//
//	// Convert to screen coordinates.            
//	::MapWindowPoints(GetHandle(), HWND_DESKTOP, (LPPOINT)&rc, 2);                         
//    
//	MMenu* menu = mMenus[toolbar->iItem];
//
//	if (menu != nullptr)
//		menu->Popup(mWindowImpl->GetWindow(), rc.left, rc.bottom, false);
//
//	outResult = 0;
//	return true;
//}
