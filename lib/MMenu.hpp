//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <list>

#include "MCommands.hpp"
#include "MP2PEvents.hpp"

class MHandler;
class MWindow;
class MWindowImpl;
class MFile;
class MMenuImpl;
class MMenuBar;

namespace zeep { namespace xml { class node; class element; } }

class MMenu
{
  public:
					MMenu(const std::string& inLabel, bool inPopup);
	virtual			~MMenu();

	static MMenu*	CreateFromResource(const char* inResourceName, bool inPopup);

	void			AppendItem(const std::string& inLabel, uint32_t inCommand);
	void			AppendRadioItem(const std::string& inLabel, uint32_t inCommand);
	void			AppendCheckItem(const std::string& inLabel, uint32_t inCommand);
	void			AppendSeparator();
	virtual void	AppendMenu(MMenu* inMenu);
	uint32_t			CountItems();
	void			RemoveItems(uint32_t inFromIndex, uint32_t inCount);

	std::string		GetItemLabel(uint32_t inIndex) const;

	void			SetItemCommand(uint32_t inIndex, uint32_t inCommand);
	uint32_t			GetItemCommand(uint32_t inIndex) const;

	void			SetTarget(MHandler* inHandler);

	void			UpdateCommandStatus();

	std::string		GetLabel()				{ return mLabel; }
	MHandler*		GetTarget()				{ return mTarget; }
	
	void			Popup(MWindow* inTarget, int32_t inX, int32_t inY, bool inBottomMenu);
	
	static MMenu*	Create(zeep::xml::element* inXMLNode, bool inPopup);

	MMenuImpl*		impl() const			{ return mImpl; }

  protected:

	MMenu(MMenuImpl* inImpl);

	MMenuImpl*		mImpl;
	std::string		mLabel;
	std::string		mSpecial;
	MHandler*		mTarget;
};

class MMenuBar : public MMenu
{
  public:
	MMenuBar();
	void AddToWindow(MWindowImpl* inWindowImpl);
	static MMenuBar* Create(zeep::xml::element* inXMLNode);
};
