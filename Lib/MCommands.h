//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// stock commands

enum MStdCommand {
	cmd_New =					'new ',
	cmd_Open =					'open',
	cmd_Quit =					'quit',
	cmd_Close =					'clos',
	cmd_Save =					'save',
	cmd_SaveAs =				'sava',
	cmd_Revert =				'reve',
	cmd_PageSetup =				'pgsu',
	cmd_Print =					'prnt',
	cmd_Undo =					'undo',
	cmd_Redo =					'redo',
	cmd_Cut =					'cut ',
	cmd_Copy =					'copy',
	cmd_Paste =					'past',
	cmd_Clear =					'clea',
	cmd_SelectAll =				'sall',
	
	cmd_About =					'abou',
	cmd_Help =					'help',
	
	cmd_Menu =					'menu',		// Shift-F10 or Menu key

	cmd_Find = 					'Find',
	cmd_FindNext =				'FndN',
	cmd_FindPrev =				'FndP',
	cmd_Replace =				'Repl',
	cmd_ReplaceAll =			'RplA',
	cmd_ReplaceFindNext =		'RpFN',
	cmd_ReplaceFindPrev =		'RpFP',
	cmd_Preferences	=			'pref',
	cmd_CloseAll =				'Clos',
	cmd_SaveAll =				'Save',
	cmd_OpenRecent =			'Recd',
	cmd_ClearRecent =			'ClrR',
	cmd_SelectWindowFromMenu =	'WSel',
};

// ---------------------------------------------------------------------------
//
// edit key commands
//

enum MKeyCommand
{
	kcmd_None,
	
	kcmd_MoveCharacterLeft,
	kcmd_MoveCharacterRight,
	kcmd_MoveWordLeft,
	kcmd_MoveWordRight,
	kcmd_MoveToBeginningOfLine,
	kcmd_MoveToEndOfLine,
	kcmd_MoveToPreviousLine,
	kcmd_MoveToNextLine,
	kcmd_MoveToTopOfPage,
	kcmd_MoveToEndOfPage,
	kcmd_MoveToBeginningOfFile,
	kcmd_MoveToEndOfFile,

	kcmd_MoveLineUp,
	kcmd_MoveLineDown,

	kcmd_DeleteCharacterLeft,
	kcmd_DeleteCharacterRight,
	kcmd_DeleteWordLeft,
	kcmd_DeleteWordRight,
	kcmd_DeleteToEndOfLine,
	kcmd_DeleteToEndOfFile,

	kcmd_ExtendSelectionWithCharacterLeft,
	kcmd_ExtendSelectionWithCharacterRight,
	kcmd_ExtendSelectionWithPreviousWord,
	kcmd_ExtendSelectionWithNextWord,
	kcmd_ExtendSelectionToCurrentLine,
	kcmd_ExtendSelectionToPreviousLine,
	kcmd_ExtendSelectionToNextLine,
	kcmd_ExtendSelectionToBeginningOfLine,
	kcmd_ExtendSelectionToEndOfLine,
	kcmd_ExtendSelectionToBeginningOfPage,
	kcmd_ExtendSelectionToEndOfPage,
	kcmd_ExtendSelectionToBeginningOfFile,
	kcmd_ExtendSelectionToEndOfFile,

	kcmd_ScrollOneLineUp,
	kcmd_ScrollOneLineDown,
	kcmd_ScrollPageUp,
	kcmd_ScrollPageDown,
	kcmd_ScrollToStartOfFile,
	kcmd_ScrollToEndOfFile
};
