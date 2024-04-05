/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MCommand.hpp"
#include "MWindow.hpp"

#include <pinch.hpp>

class MBoxControl;
class MStatusbar;
class MScrollbar;
class MTerminalView;
class MSearchPanel;
class MAnimationVariable;
class MAnimationManager;
class MTerminalChannel;

class MTerminalWindow : public MWindow
{
  public:
	static MTerminalWindow *Create(const std::vector<std::string> &inArgv);
	static MTerminalWindow *Create(const std::string &inUser, const std::string &inHost, uint16_t inPort,
		const std::string &inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection);

	virtual MTerminalWindow *Clone(MTerminalWindow *inOriginal) = 0;

	void ShowSelf() override;
	void FocusTerminalView();

	static MTerminalWindow *GetFirstTerminal() { return sFirst; }
	MTerminalWindow *GetNextTerminal() { return mNext; }

	~MTerminalWindow() override;

	bool AllowClose(bool inLogOff) override;
	void Close() override;

	static bool IsAnyTerminalOpen();

	void SetTitle(const std::string &inTitle) override;

	uint32_t GetTerminalNr() const
	{
		return mNr;
	}

  protected:
	MTerminalWindow(MTerminalChannel *inChannel, const std::vector<std::string> &inArgv);

	void ShowSearchPanel();
	void HideSearchPanel();

	static MRect GetPreferredBounds();

	void OnClose();
	void OnCloneTerminal();
	void OnFind();
	void OnNextTerminal();
	void OnPrevTerminal();

	MCommand<void()> cClose;
	MCommand<void()> cCloneTerminal;
	MCommand<void()> cFind;
	MCommand<void()> cNextTerminal;
	MCommand<void()> cPrevTerminal;

	MTerminalChannel *mChannel;
	MBoxControl *mMainVBox;
	MStatusbar *mStatusbar;
	MScrollbar *mScrollbar;
	MSearchPanel *mSearchPanel;
	std::shared_ptr<MTerminalView> mTerminalView;

	static MTerminalWindow *sFirst;
	MTerminalWindow *mNext;

	uint32_t mNr;
	static uint32_t sNextNr;
};
