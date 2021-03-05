//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <list>
#include <string>

class MMenu;

class MHandler
{
  public:
						MHandler(MHandler* inSuper);

	virtual				~MHandler();

	virtual bool		UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex, bool& outEnabled, bool& outChecked);

	virtual bool		ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex, uint32 inModifiers);

	virtual bool		HandleKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
	virtual bool		HandleCharacter(const std::string& inText, bool inRepeat);

	MHandler*			GetSuper() const						{ return mSuper; }
	void				SetSuper(MHandler* inSuper);

	virtual void		BeFocus();
	virtual void		DontBeFocus();

	virtual void		SetFocus();
	virtual void		ReleaseFocus();

	virtual bool		IsFocus() const;

  protected:

	MHandler*			mSuper;
};
