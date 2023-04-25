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

	virtual bool		UpdateCommandStatus(uint32_t inCommand, MMenu* inMenu, uint32_t inItemIndex, bool& outEnabled, bool& outChecked);

	virtual bool		ProcessCommand(uint32_t inCommand, const MMenu* inMenu, uint32_t inItemIndex, uint32_t inModifiers);

	virtual bool		HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
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
