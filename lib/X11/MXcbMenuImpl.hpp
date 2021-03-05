//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MMenuImpl.hpp"
#include "MXcbWinMixin.hpp"

// --------------------------------------------------------------------

struct MMenuItem
{
	std::string mLabel;
	uint32 mCommand;
	MMenu* mSubMenu;
	bool mEnabled;
	bool mChecked;
};

// --------------------------------------------------------------------

class MXcbMenuImpl : public MMenuImpl
{
  public:
	MXcbMenuImpl(MMenu* inMenu);
	~MXcbMenuImpl();
	
	virtual void SetTarget(MHandler* inHandler);

	virtual void SetItemState(uint32 inItem, bool inEnabled, bool inChecked);

	virtual void AppendItem(const std::string& inLabel, uint32 inCommand);
	virtual void AppendSubmenu(MMenu* inSubmenu);
	virtual void AppendSeparator();
	virtual void AppendCheckbox(const std::string& inLabel, uint32 inCommand);
	virtual void AppendRadiobutton(const std::string& inLabel, uint32 inCommand);
	virtual uint32 CountItems() const;
	virtual void RemoveItems(uint32 inFirstIndex, uint32 inCount);

	virtual std::string	GetItemLabel(uint32 inIndex) const;
	virtual void SetItemCommand(uint32 inIndex, uint32 inCommand);
	virtual uint32 GetItemCommand(uint32 inIndex) const;
	virtual MMenu* GetSubmenu(uint32 inIndex) const;

	virtual void Popup(MWindow* inHandler, int32 inX, int32 inY, bool inBottomMenu);
	virtual void AddToWindow(MWindowImpl* inWindow);
	
	virtual void MenuUpdated();

  protected:
	MHandler* mTarget;
	std::vector<MMenuItem> mItems;
};

// --------------------------------------------------------------------

class MXcbMenuBarImpl : public MXcbMenuImpl, public MXcbWinMixin
{
  public:
	MXcbMenuBarImpl(MMenu* inMenu);

  private:
};

