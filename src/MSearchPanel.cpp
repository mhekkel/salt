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

#include "MSearchPanel.hpp"
#include "MCanvas.hpp"
#include "MControls.hpp"
#include "MDevice.hpp"
#include "MPreferences.hpp"
#include "MSound.hpp"
#include "MStrings.hpp"
#include "MTerminalWindow.hpp"

#include "mrsrc.hpp"

class MImageButton : public MCanvas
{
  public:
	MImageButton(const std::string &inID, MRect inBounds, const char *inImageResource);

	MEventOut<void()> eClicked;

	// virtual void	Draw(MRect inUpdate);
	virtual void Draw();
	virtual void MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers);
	virtual void MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void MouseExit();

  private:
	enum MMouseStateForCloseButton
	{
		mouseOut,
		mouseOver,
		mouseDownOver,
		mouseDownOut
	} mMouseState;
	MBitmap mCloseButtonBitmaps[3];
};

MImageButton::MImageButton(const std::string &inID, MRect inBounds, const char *inImageResource)
	: MCanvas(inID, inBounds)
	, mMouseState(mouseOut)
{
	mrsrc::rsrc rsrc(inImageResource);
	if (not rsrc)
		throw std::runtime_error("resource not found");

	MBitmap buttons(rsrc.data(), rsrc.size());
	mCloseButtonBitmaps[0] = MBitmap(buttons, MRect{ 0, 0, 16, 16 });
	mCloseButtonBitmaps[1] = MBitmap(buttons, MRect{ 16, 0, 16, 16 });
	mCloseButtonBitmaps[2] = MBitmap(buttons, MRect{ 32, 0, 16, 16 });
}

void MImageButton::Draw()
{
	MRect bounds = GetBounds();

	MDevice dev(this);

	if (kDialogBackgroundColor != kBlack)
	{
		dev.SetBackColor(kDialogBackgroundColor);
		dev.EraseRect(bounds);
	}

	float y = (bounds.height - 16) / 2;

	switch (mMouseState)
	{
		default: dev.DrawBitmap(mCloseButtonBitmaps[0], 0, y); break;
		case mouseOver:
		case mouseDownOut: dev.DrawBitmap(mCloseButtonBitmaps[1], 0, y); break;
		case mouseDownOver: dev.DrawBitmap(mCloseButtonBitmaps[2], 0, y); break;
	}
}

void MImageButton::MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers)
{
	if (mBounds.ContainsPoint(inX, inY))
	{
		mMouseState = mouseDownOver;
		Invalidate();
	}
}

void MImageButton::MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	if (mBounds.ContainsPoint(inX, inY))
		eClicked();
}

void MImageButton::MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	MMouseStateForCloseButton state(mMouseState);

	// if (mMouseState == mouseOut)
	// 	TrackMouse(true, true);

	if (mBounds.ContainsPoint(inX, inY))
	{
		if (mMouseState == mouseOut)
			mMouseState = mouseOver;
		else if (mMouseState == mouseDownOut)
			mMouseState = mouseDownOver;
	}
	else
	{
		if (mMouseState == mouseOver)
			mMouseState = mouseOut;
		else if (mMouseState == mouseDownOver)
			mMouseState = mouseDownOut;
	}

	if (mMouseState != state)
		Invalidate();
}

void MImageButton::MouseExit()
{
	if (mMouseState != mouseOut)
	{
		Invalidate();
		mMouseState = mouseOut;
	}
}

// --------------------------------------------------------------------

MSearchPanel::MSearchPanel(const std::string &inID, MRect inBounds)
	: MBoxControl(inID, inBounds, true)
	, eClose(this, &MSearchPanel::Close)
	, eFindBtn(this, &MSearchPanel::FindBtn)
	, mTextBox(nullptr)
{
	MDevice dev;
	dev.SetFont("Segoe UI 9");

	std::string captionString(_("Find:"));
	dev.SetText(captionString);
	uint32_t captionWidth = static_cast<uint32_t>(dev.GetTextWidth());

	MRect bounds(inBounds);

	// close button

	bounds.x = bounds.y = (kSearchPanelHeight - 16) / 2;
	bounds.width = bounds.height = 16;
	MImageButton *closeButton = new MImageButton("close-button", bounds, "close.png");
	// closeButton->SetLayout(ePackStart, false, false, 8);
	AddChild(closeButton);
	AddRoute(eClose, closeButton->eClicked);

	// caption
	bounds = GetBounds();
	bounds.x = 32;
	bounds.width = captionWidth;
	bounds.y = (bounds.height - 24) / 2 + 4;
	bounds.height = dev.GetLineHeight();
	//	bounds.height = 24;
	MCaption *caption = new MCaption("search-caption", bounds, captionString);
	// caption->SetLayout(ePackStart, false, false, 4);
	AddChild(caption);

	bounds = GetBounds();
	bounds.x = 32 + captionWidth + 4;
	bounds.width = 200;
	bounds.y = (bounds.height - 24) / 2 + 0;
	bounds.height = 24;
	mTextBox = new MEdittext("searchstring", bounds);
	// mTextBox->SetLayout(ePackStart, true, true, 4);
	AddChild(mTextBox);
	mTextBox->SetText(Preferences::GetString("find-recent", ""));

	std::string label(_("Case sensitive"));
	dev.SetText(label);
	uint32_t labelWidth = static_cast<uint32_t>(dev.GetTextWidth());

	bounds = GetBounds();
	bounds.x = 32 + captionWidth + 4 + 200 + 4;
	bounds.height = 24;
	bounds.y = (bounds.height - 24) / 2 + 2;
	bounds.width = 20 + labelWidth;
	mCaseSensitive = new MCheckbox("case-sensitive", bounds, label);
	// mCaseSensitive->SetLayout(ePackStart, false, false, 4);
	AddChild(mCaseSensitive);
	mCaseSensitive->SetChecked(Preferences::GetBoolean("find-case-sensitive", false));

	// nog twee knoppen

	bounds.x += bounds.width + 10;

	label = _("Next");
	dev.SetText(label);
	labelWidth = static_cast<uint32_t>(dev.GetTextWidth());

	bounds.width = labelWidth + 20;
	MButton *next = new MButton("find-next", bounds, label);
	// next->SetLayout(ePackStart, false, false, 4);
	AddChild(next);
	AddRoute(next->eClicked, eFindBtn);

	bounds.x += bounds.width + 10;

	label = _("Previous");
	dev.SetText(label);
	labelWidth = static_cast<uint32_t>(dev.GetTextWidth());

	bounds.width = labelWidth + 20;
	MButton *prev = new MButton("find-prev", bounds, label);
	// prev->SetLayout(ePackStart, false, false, 4);
	AddChild(prev);
	AddRoute(prev->eClicked, eFindBtn);
}

MSearchPanel::~MSearchPanel()
{
}

void MSearchPanel::Close()
{
	Preferences::SetString("find-recent", mTextBox->GetText());
	// mHideSearchPanel();
	// ProcessCommand(cmd_HideSearchPanel, nullptr, 0, 0);
}

void MSearchPanel::SetFocus()
{
	mTextBox->SetFocus();
}

void MSearchPanel::FindBtn(const std::string &inID)
{
	if (inID == "find-next")
		eSearch(searchDown);
	else if (inID == "find-prev")
		eSearch(searchUp);
}

uint32_t MSearchPanel::GetHeight() const
{
	MRect bounds;
	bounds = GetBounds();
	return bounds.height;
}

bool MSearchPanel::HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool result = true;
	switch (inKeyCode)
	{
		case kReturnKeyCode:
		case kEnterKeyCode:
			eSearch((inModifiers & kShiftKey) ? searchUp : searchDown);
			break;

		case kEscapeKeyCode:
			Close();
			break;

		case kUpArrowKeyCode:
			eSearch(searchUp);
			break;
		
		case kDownArrowKeyCode:
			eSearch(searchDown);
			break;

		case kTabKeyCode:
			if (auto w = dynamic_cast<MTerminalWindow *>(GetWindow()); w != nullptr)
				w->FocusTerminalView();
			break;

		default:
			result = MBoxControl::HandleKeyDown(inKeyCode, inModifiers, inRepeat);
			break;
	}

	return result;
}

std::string MSearchPanel::GetSearchString() const
{
	return mTextBox->GetText();
}

bool MSearchPanel::GetIgnoreCase() const
{
	return not mCaseSensitive->IsChecked();
}
