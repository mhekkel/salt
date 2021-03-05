//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include "MHandler.hpp"

using namespace std;

MHandler::MHandler(
	MHandler*		inSuper)
	: mSuper(inSuper)
{
}

MHandler::~MHandler()
{
	SetSuper(nullptr);
}

bool MHandler::UpdateCommandStatus(
	uint32			inCommand,
	MMenu*			inMenu,
	uint32			inItemIndex,
	bool&			outEnabled,
	bool&			outChecked)
{
	bool result = false;
	
	if (mSuper != nullptr)
	{
		result = mSuper->UpdateCommandStatus(
			inCommand, inMenu, inItemIndex, outEnabled, outChecked);
	}
	else
	{
		outEnabled = false;
		outChecked = false;
	}
	
	return result;
}

bool MHandler::ProcessCommand(
	uint32			inCommand,
	const MMenu*	inMenu,
	uint32			inItemIndex,
	uint32			inModifiers)
{
	bool result = false;
	
	if (mSuper != nullptr)
		result = mSuper->ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
	
	return result;
}

bool MHandler::HandleKeyDown(
	uint32			inKeyCode,
	uint32			inModifiers,
	bool			inRepeat)
{
	bool result = false;
	if (mSuper != nullptr)
		result = mSuper->HandleKeyDown(inKeyCode, inModifiers, inRepeat);
	return result;
}

bool MHandler::HandleCharacter(const string& inText, bool inRepeat)
{
	bool result = false;
	if (mSuper != nullptr)
		result = mSuper->HandleCharacter(inText, inRepeat);
	return result;
}

void MHandler::SetSuper(
	MHandler*		inSuper)
{
	mSuper = inSuper;
}

bool MHandler::IsFocus() const
{
	return false;
}

void MHandler::BeFocus()
{
	
}

void MHandler::DontBeFocus()
{
	
}

void MHandler::SetFocus()
{
}

void MHandler::ReleaseFocus()
{
}
