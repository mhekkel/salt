//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <set>
#include <memory>
#include <cstring>

#include "MCommands.hpp"
#include "MAcceleratorTable.hpp"

using namespace std;

const uint32_t
	kValidModifiersMask = kControlKey | kShiftKey | kOptionKey;

namespace {

struct MCommandToString
{
	char mCommandString[10];
	
	MCommandToString(uint32_t inCommand)
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

struct MAccelCombo
{
	int64_t		key;
	uint32_t		keyval;
	uint32_t		modifiers;
	uint32_t		command;
	
	bool		operator<(const MAccelCombo& rhs) const
					{ return key < rhs.key; }
};


struct MAcceleratorTableImp
{
	set<MAccelCombo>	mTable;
};

// -------------------------------------

MAcceleratorTable& MAcceleratorTable::Instance()
{
	static MAcceleratorTable sInstance;
	return sInstance;
}

MAcceleratorTable&
MAcceleratorTable::EditKeysInstance()
{
	static unique_ptr<MAcceleratorTable>	sInstance;
	
	if (sInstance.get() == nullptr)
	{
		sInstance.reset(new MAcceleratorTable);

		sInstance->RegisterAcceleratorKey(kcmd_MoveCharacterLeft, kLeftArrowKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveCharacterRight, kRightArrowKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveWordLeft, kLeftArrowKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_MoveWordRight, kRightArrowKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToBeginningOfLine, kHomeKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToEndOfLine, kEndKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToPreviousLine, kUpArrowKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToNextLine, kDownArrowKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToTopOfPage, kPageUpKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToEndOfPage, kPageDownKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToBeginningOfFile, kHomeKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_MoveToEndOfFile, kEndKeyCode, kControlKey);

		sInstance->RegisterAcceleratorKey(kcmd_MoveLineUp, kUpArrowKeyCode, kOptionKey);
		sInstance->RegisterAcceleratorKey(kcmd_MoveLineDown, kDownArrowKeyCode, kOptionKey);

		sInstance->RegisterAcceleratorKey(kcmd_DeleteCharacterLeft, kBackspaceKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_DeleteCharacterRight, kDeleteKeyCode, 0);
		sInstance->RegisterAcceleratorKey(kcmd_DeleteWordLeft, kBackspaceKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_DeleteWordRight, kDeleteKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_DeleteToEndOfLine, kDeleteKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_DeleteToEndOfFile, kDeleteKeyCode, kControlKey | kShiftKey);

		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionWithCharacterLeft, kLeftArrowKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionWithCharacterRight, kRightArrowKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionWithPreviousWord, kLeftArrowKeyCode, kShiftKey | kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionWithNextWord, kRightArrowKeyCode, kShiftKey | kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToCurrentLine, 'L', kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToPreviousLine, kUpArrowKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToNextLine, kDownArrowKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToBeginningOfLine, kHomeKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToEndOfLine, kEndKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToBeginningOfPage, kPageUpKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToEndOfPage, kPageDownKeyCode, kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToBeginningOfFile, kHomeKeyCode, kShiftKey | kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_ExtendSelectionToEndOfFile, kEndKeyCode, kShiftKey | kControlKey);

		sInstance->RegisterAcceleratorKey(kcmd_ScrollOneLineUp, kDownArrowKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_ScrollOneLineDown, kUpArrowKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_ScrollPageUp, kPageUpKeyCode, kControlKey);
		sInstance->RegisterAcceleratorKey(kcmd_ScrollPageDown, kPageDownKeyCode, kControlKey);
//		sInstance->RegisterAcceleratorKey(kcmd_ScrollToStartOfFile, GDK_Home, kControlKey);
//		sInstance->RegisterAcceleratorKey(kcmd_ScrollToEndOfFile, GDK_End, kControlKey);

		sInstance->RegisterAcceleratorKey(kcmd_MoveLineUp, kUpArrowKeyCode, kControlKey | kShiftKey);
		sInstance->RegisterAcceleratorKey(kcmd_MoveLineDown, kDownArrowKeyCode, kControlKey | kShiftKey);
	}
	
	return *sInstance.get();
}

MAcceleratorTable::MAcceleratorTable()
	: mImpl(new MAcceleratorTableImp)
{
}

MAcceleratorTable::~MAcceleratorTable()
{
	delete mImpl;
}

void MAcceleratorTable::RegisterAcceleratorKey(
	uint32_t			inCommand,
	uint32_t			inKeyValue,
	uint32_t			inModifiers)
{
	int64_t key = (int64_t(inKeyValue) << 32) | inModifiers;
	MAccelCombo kc = {
		key, inKeyValue, inModifiers, inCommand
	};

	mImpl->mTable.insert(kc);

	//gint nkeys;
	//GdkKeymapKey* keys;
	//
	//if (gdk_keymap_get_entries_for_keyval(gdk_keymap_get_default(),
	//	inKeyValue, &keys, &nkeys))
	//{
	//	for (int32_t k = 0; k < nkeys; ++k)
	//	{
	//		guint keyval;
	//		GdkModifierType consumed;
	//		
	//		if (gdk_keymap_translate_keyboard_state(nullptr,
	//			keys[k].keycode, GdkModifierType(inModifiers), 0, &keyval, nullptr, nullptr, &consumed))
	//		{
	//			uint32_t modifiers = inModifiers & ~consumed;
	//			if (inModifiers & kShiftKey)
	//				modifiers |= kShiftKey;				
	//			
	//			int64_t key = (int64_t(keyval) << 32) | modifiers;
	//			
	//			MAccelCombo kc = {
	//				key, inKeyValue, inModifiers, inCommand
	//			};
	//			
	//			mImpl->mTable.insert(kc);
	//		}

	//		if (gdk_keymap_translate_keyboard_state(nullptr,
	//			keys[k].keycode, GdkModifierType(inModifiers | GDK_LOCK_MASK), 0, &keyval, nullptr, nullptr, &consumed))
	//		{
	//			uint32_t modifiers = inModifiers & ~consumed;
	//			if (inModifiers & kShiftKey)
	//				modifiers |= kShiftKey;				
	//			
	//			int64_t key = (int64_t(keyval) << 32) | modifiers;
	//			
	//			MAccelCombo kc = {
	//				key, inKeyValue, inModifiers, inCommand
	//			};
	//			
	//			mImpl->mTable.insert(kc);
	//		}
	//	}

	//	g_free(keys);
	//}
}

bool MAcceleratorTable::GetAcceleratorKeyForCommand(
	uint32_t			inCommand,
	uint32_t&			outKeyValue,
	uint32_t&			outModifiers)
{
	bool result = false;
	
	for (set<MAccelCombo>::iterator a = mImpl->mTable.begin(); a != mImpl->mTable.end(); ++a)
	{
		if (a->command == inCommand)
		{
			outKeyValue = a->keyval;
			outModifiers = a->modifiers;
			result = true;
			break;
		}
	}
	
	return result;
}

bool MAcceleratorTable::IsAcceleratorKey(
	uint32_t			inKeyCode,
	uint32_t			inModifiers,
	uint32_t&			outCommand)
{
	bool result = false;

//	PRINT(("IsAcceleratorKey for %10.10s %c-%c-%c-%c (%x - %x)",
//		gdk_keyval_name(keyval),
//		inEvent->state & kControlKey ? 'C' : '_',
//		inEvent->state & kShiftKey ? 'S' : '_',
//		inEvent->state & kOptionKey ? '1' : '_',
//		inEvent->state & GDK_MOD2_MASK ? '2' : '_',
//		inEvent->state, modifiers));

	inModifiers &= kValidModifiersMask;
	int64_t key = (int64_t(inKeyCode) << 32) | inModifiers;

	MAccelCombo kc;
	kc.key = key;

	set<MAccelCombo>::iterator a = mImpl->mTable.find(kc);	
	if (a != mImpl->mTable.end())
	{
		outCommand = a->command;
		result = true;
	}
	
//	if (result)
//		PRINT(("cmd is %s", (const char*)MCommandToString(outCommand)));
	
	return result;
}

bool MAcceleratorTable::IsNavigationKey(
	uint32_t			inKeyValue,
	uint32_t			inModifiers,
	MKeyCommand&	outCommand)
{
	bool result = false;
	
//	PRINT(("IsNavigationKey for %10.10s %c-%c-%c-%c (%x - %x)",
//		gdk_keyval_name(inKeyValue),
//		inModifiers & kControlKey ? 'C' : '_',
//		inModifiers & kShiftKey ? 'S' : '_',
//		inModifiers & kOptionKey ? '1' : '_',
//		inModifiers & GDK_MOD2_MASK ? '2' : '_',
//		inModifiers, inModifiers & kValidModifiersMask));

	inModifiers &= kValidModifiersMask;

	MAccelCombo kc;
	kc.key = (int64_t(inKeyValue) << 32) | inModifiers;

	set<MAccelCombo>::iterator a = mImpl->mTable.find(kc);	
	if (a != mImpl->mTable.end())
	{
		outCommand = MKeyCommand(a->command);
		result = true;
	}

//	if (result)
//		PRINT(("cmd is %s", (const char*)MCommandToString(outCommand)));
//	
	return result;
}


