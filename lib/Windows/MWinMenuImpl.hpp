//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MMenuImpl.hpp"

class MWinMenuImpl : public MMenuImpl
{
public:
						MWinMenuImpl(MMenu* inMenu, bool inPopup);

						~MWinMenuImpl();

	virtual void		SetTarget(MHandler* inHandler);

	virtual void		SetItemState(uint32_t inItem, bool inEnabled, bool inChecked);

	virtual void		AppendItem(const std::string& inLabel, uint32_t inCommand);

	virtual void		AppendSubmenu(MMenu* inSubmenu);

	virtual void		AppendSeparator();

	virtual void		AppendCheckbox(const std::string& inLabel, uint32_t inCommand);

	virtual void		AppendRadiobutton(const std::string& inLabel, uint32_t inCommand);

	virtual uint32_t		CountItems() const;

	virtual void		RemoveItems(uint32_t inFirstIndex, uint32_t inCount);

	virtual std::string	GetItemLabel(uint32_t inIndex) const;

	virtual void		SetItemCommand(uint32_t inIndex, uint32_t inCommand);

	virtual uint32_t		GetItemCommand(uint32_t inIndex) const;

	virtual MMenu*		GetSubmenu(uint32_t inIndex) const;

	virtual void		Popup(MWindow* inHandler, int32_t inX, int32_t inY, bool inBottomMenu);

	HMENU				GetHandle() const		{ return mMenuHandle; }

	static MMenu*		Lookup(HMENU inHandle);

	virtual void		AddToWindow(MWindowImpl* inWindow);

private:
	HMENU				mMenuHandle;
};


//class MWinMenubar : public MWinProcMixin
//{
//  public:
//					MWinMenubar(// MWinWindowImpl* inWindowImpl, // const char* inMenuResource);
//
//					~MWinMenubar();
//
//  protected:
//	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
//						std::wstring& outClassName, HMENU& outMenu);
//	virtual void	RegisterParams(UINT& outStyle, HCURSOR& outCursor,
//						HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground);
//
//	bool			NDropDown(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
//
//	MWinWindowImpl*	mWindowImpl;
//	std::vector<MMenu*>
//					mMenus;
//};
