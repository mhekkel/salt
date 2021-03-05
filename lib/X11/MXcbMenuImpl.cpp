//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include "MXcbWindowImpl.hpp"
#include "MX11ApplicationImpl.hpp"
#include "MXcbMenuImpl.hpp"

#include "MError.hpp"

using namespace std;

// --------------------------------------------------------------------

namespace
{

struct MCommandToString
{
	char mCommandString[10];
	
	MCommandToString(
		uint32_t			inCommand)
	{
		strcpy(mCommandString, "MCmd_xxxx");
		
		mCommandString[5] = ((inCommand & 0xff000000) >> 24) & 0x000000ff;
		mCommandString[6] = ((inCommand & 0x00ff0000) >> 16) & 0x000000ff;
		mCommandString[7] = ((inCommand & 0x0000ff00) >>  8) & 0x000000ff;
		mCommandString[8] = ((inCommand & 0x000000ff) >>  0) & 0x000000ff;
	}
	
	operator const char*() const	{ return mCommandString; }
};

}

// --------------------------------------------------------------------

MXcbMenuImpl::MXcbMenuImpl(MMenu* inMenu)
	: MMenuImpl(inMenu)
	, mTarget(nullptr)
{
	PRINT(("MENU: %s", __func__));
}

MXcbMenuImpl::~MXcbMenuImpl()
{
	PRINT(("MENU: %s", __func__));
	for (auto mi: mItems)
		delete mi.mSubMenu;
}

void MXcbMenuImpl::SetTarget(MHandler* inHandler)
{
//	PRINT(("MENU: %s", __func__));
	mTarget = inHandler;
}

void MXcbMenuImpl::SetItemState(uint32_t inItem, bool inEnabled, bool inChecked)
{
//	PRINT(("MENU: %s", __func__));
	if (inItem < mItems.size())
	{
		mItems[inItem].mEnabled = inEnabled;
		mItems[inItem].mChecked = inChecked;
	}
}

void MXcbMenuImpl::AppendItem(const string& inLabel, uint32_t inCommand)
{
	PRINT(("MENU: %s(%s, %x)", __func__, inLabel.c_str(), inCommand));
	MMenuItem item = { inLabel, inCommand };
	mItems.push_back(item);
}

void MXcbMenuImpl::AppendSubmenu(MMenu* inSubmenu)
{
//	PRINT(("MENU: %s", __func__));
	MMenuItem item = { inSubmenu->GetLabel(), 0, inSubmenu };
	mItems.push_back(item);
}

void MXcbMenuImpl::AppendSeparator()
{
//	PRINT(("MENU: %s", __func__));
	MMenuItem item = { "-", 0 };
	mItems.push_back(item);
}

void MXcbMenuImpl::AppendCheckbox(const string& inLabel, uint32_t inCommand)
{
//	PRINT(("MENU: %s", __func__));
	MMenuItem item = { inLabel + "-checkbox", inCommand };
	mItems.push_back(item);
}

void MXcbMenuImpl::AppendRadiobutton(const string& inLabel, uint32_t inCommand)
{
//	PRINT(("MENU: %s", __func__));
	MMenuItem item = { inLabel + "-radiobutton", inCommand };
	mItems.push_back(item);
}

uint32_t MXcbMenuImpl::CountItems() const
{
//	PRINT(("MENU: %s", __func__));
	return mItems.size();
}

void MXcbMenuImpl::RemoveItems(uint32_t inFirstIndex, uint32_t inCount)
{
//	PRINT(("MENU: %s", __func__));
	if (inFirstIndex < mItems.size())
	{
		if (inCount > mItems.size() + inFirstIndex)
			inCount = mItems.size() + inFirstIndex;
		
		mItems.erase(mItems.begin() + inFirstIndex, mItems.begin() + inFirstIndex + inCount);
	}
}

string	MXcbMenuImpl::GetItemLabel(uint32_t inIndex) const
{
//	PRINT(("MENU: %s", __func__));
	string label;
	if (inIndex < mItems.size())
		label = mItems[inIndex].mLabel;
	return label;
}

void MXcbMenuImpl::SetItemCommand(uint32_t inIndex, uint32_t inCommand)
{
//	PRINT(("MENU: %s", __func__));
	if (inIndex < mItems.size())
		mItems[inIndex].mCommand = inCommand;
}

uint32_t MXcbMenuImpl::GetItemCommand(uint32_t inIndex) const
{
//	PRINT(("MENU: %s", __func__));
	uint32_t command = 0;
	if (inIndex < mItems.size())
		command = mItems[inIndex].mCommand;
	return command;
}

MMenu* MXcbMenuImpl::GetSubmenu(uint32_t inIndex) const
{
//	PRINT(("MENU: %s", __func__));
	MMenu* result = nullptr;
	if (inIndex < mItems.size())
		result = mItems[inIndex].mSubMenu;
	return result;
}

void MXcbMenuImpl::Popup(MWindow* inHandler, int32_t inX, int32_t inY, bool inBottomMenu)
{
	PRINT(("MENU: %s", __func__));
}

void MXcbMenuImpl::AddToWindow(MWindowImpl* inWindow)
{
	PRINT(("MENU: %s", __func__));
}

void MXcbMenuImpl::MenuUpdated()
{
	PRINT(("MENU: %s", __func__));
}

MMenuImpl* MMenuImpl::Create(MMenu* inMenu, bool inPopup)
{
	PRINT(("MENU: %s", __func__));
	return new MXcbMenuImpl(inMenu);
}

// --------------------------------------------------------------------

MXcbMenuBarImpl::MXcbMenuBarImpl(MMenu* inMenu)
	: MXcbMenuImpl(inMenu)
{
	PRINT(("MENU: %s", __func__));
}

// --------------------------------------------------------------------

MMenuImpl* MMenuImpl::CreateBar(MMenu* inMenu)
{
	PRINT(("MENU: %s", __func__));
	return new MXcbMenuBarImpl(inMenu);
}

