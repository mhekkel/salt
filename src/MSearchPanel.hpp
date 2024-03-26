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

// Copyright Maarten L. Hekkelman 2013
// All rights reserved

#pragma once

#include "MControls.hpp"
#include "MP2PEvents.hpp"

const uint32_t
	kSearchPanelHeight = 28,
	cmd_HideSearchPanel = 'hidS';

enum MSearchDirection
{
	searchUp,
	searchDown
};

class MSearchPanel : public MBoxControl
{
  public:
	MSearchPanel(const std::string &inID,
		MRect inBounds);

	virtual ~MSearchPanel();

	bool KeyPressed(uint32_t inKeyCode, uint32_t inModifiers) override;

	std::string GetSearchString() const;
	bool GetIgnoreCase() const;

	uint32_t GetHeight() const;

	MEventOut<void(MSearchDirection)> eSearch;
	MEdittext *GetTextBox() const { return mTextBox; }

	virtual void SetFocus();

  private:
	MEventIn<void()> eClose;
	void Close();

	MEventIn<void(const std::string &)> eFindBtn;
	void FindBtn(const std::string &);

	MEdittext *mTextBox;
	MCheckbox *mCaseSensitive;
};
