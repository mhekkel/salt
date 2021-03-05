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

	void			AppendItem(const std::string& inLabel, uint32 inCommand);
	void			AppendRadioItem(const std::string& inLabel, uint32 inCommand);
	void			AppendCheckItem(const std::string& inLabel, uint32 inCommand);
	void			AppendSeparator();
	virtual void	AppendMenu(MMenu* inMenu);
	uint32			CountItems();
	void			RemoveItems(uint32 inFromIndex, uint32 inCount);

	std::string		GetItemLabel(uint32 inIndex) const;

	void			SetItemCommand(uint32 inIndex, uint32 inCommand);
	uint32			GetItemCommand(uint32 inIndex) const;

	void			SetTarget(MHandler* inHandler);

	void			UpdateCommandStatus();

	std::string		GetLabel()				{ return mLabel; }
	MHandler*		GetTarget()				{ return mTarget; }
	
	void			Popup(MWindow* inTarget, int32 inX, int32 inY, bool inBottomMenu);
	
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
