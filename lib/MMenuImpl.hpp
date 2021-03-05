//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MMenu.hpp"

class MMenuImpl
{
public:
						MMenuImpl(MMenu* inMenu)
							: mMenu(inMenu) {}

	virtual				~MMenuImpl() {}

	virtual void		SetTarget(MHandler* inHandler) = 0;

	virtual void		SetItemState(uint32_t inItem, bool inEnabled, bool inChecked) = 0;

	virtual void		AppendItem(const std::string& inLabel, uint32_t inCommand) = 0;
	virtual void		AppendSubmenu(MMenu* inSubmenu) = 0;
	virtual void		AppendSeparator() = 0;
	virtual void		AppendCheckbox(const std::string& inLabel, uint32_t inCommand) = 0;
	virtual void		AppendRadiobutton(const std::string& inLabel, uint32_t inCommand) = 0;
	virtual uint32_t		CountItems() const = 0;
	virtual void		RemoveItems(uint32_t inFirstIndex, uint32_t inCount) = 0;

	virtual std::string	GetItemLabel(uint32_t inIndex) const = 0;
	virtual void		SetItemCommand(uint32_t inIndex, uint32_t inCommand) = 0;
	virtual uint32_t		GetItemCommand(uint32_t inIndex) const = 0;
	virtual MMenu*		GetSubmenu(uint32_t inIndex) const = 0;

	virtual void		Popup(MWindow* inHandler, int32_t inX, int32_t inY, bool inBottomMenu) = 0;
	virtual void		AddToWindow(MWindowImpl* inWindow) = 0;
	
	virtual void		MenuUpdated() = 0;

	static MMenuImpl*	Create(MMenu* inMenu, bool inPopup);
	static MMenuImpl*	CreateBar(MMenu* inMenu);

protected:
	MMenu*				mMenu;
};
