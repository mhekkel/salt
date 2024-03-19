/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    std::list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this std::list of conditions and the following disclaimer in the documentation
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

#include "MTerminalView.hpp"
#include "MAnimation.hpp"
#include "MApplication.hpp"
#include "MCSICommands.hpp"
#include "MClipboard.hpp"
#include "MCommands.hpp"
#include "MControls.hpp"
#include "MDevice.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MPreferencesDialog.hpp"
#include "MSaltApp.hpp"
#include "MSearchPanel.hpp"
#include "MSound.hpp"
#include "MStrings.hpp"
#include "MTerminalBuffer.hpp"
#include "MUtils.hpp"
#include "MVT220CharSets.hpp"
#include "MWindow.hpp"

#include "MTerminalColours.hpp"

#include "Gtk/MPrimary.hpp"

#include <zeep/crypto.hpp>
#include <zeep/unicode-support.hpp>

#include <chrono>
#include <cmath>
#include <map>

namespace
{

// our commands
const uint32_t
	cmd_DebugUpdate = 'dbug',
	cmd_ResetAndClear = 'rstc',
	cmd_EncodingUTF8 = 'enU8',

	// cmd_AllowColor		= 'tClr',
    // cmd_AllowTitle		= 'tTtl',

	cmd_MetaSendsEscape = 'tEsc',
	cmd_BackSpaceIsDel = 'tBsD',
	cmd_DeleteIsDel = 'tDlD',
	cmd_OldFnKeys = 'tOlF',
	cmd_VT220Keyboard = 'tVTk',

	cmd_SendSTOP = 'STOP',
	cmd_SendCONT = 'CONT',
	cmd_SendINT = 'INT ',
	cmd_SendHUP = 'HUP ',
	cmd_SendTERM = 'TERM',
	cmd_SendKILL = 'KILL';

// This is what we report as terminal type:
// 62	VT200 family
// 1	132 columns
// 2	printer port
// 6	selective erase
// 8	user-defined keys
// 9	national replacement character-sets

const char
	//	kVT220Attributes[] = "\033[?62;1;2;6;8;9c";
	kVT420Attributes[] = "\033[?64;1;2;6;8;9c";
//	kVT520Attributes[] = "\033[?65;1;2;6;8;9c";

const std::string
	kCSI("\033["),
	kSS3("\033O"),
	kDCS("\033P");

uint32_t
	kBorderWidth = 4;

std::chrono::system_clock::duration
	kSmoothScrollDelay = std::chrono::milliseconds(25);

// enum {
//	kTextColor,
//	kBackColor,
//	kBoldColor,
//
//	kColorCount
// };
//
MColor
	sSelectionColor;

std::string
	kControlBreakMessage("Hello, world!");

enum MCtrlChr : uint8_t
{
	NUL = 0x00,
	DLE = 0x10,
	SP = 0x20,
	DCS = 0x90,
	SOH = 0x01,
	DC1 = 0x11,
	PU1 = 0x91,
	STX = 0x02,
	DC2 = 0x12,
	PU2 = 0x92,
	ETX = 0x03,
	DC3 = 0x13,
	STS = 0x93,
	EOT = 0x04,
	DC4 = 0x14,
	IND = 0x84,
	CCH = 0x94,
	ENQ = 0x05,
	NAK = 0x15,
	NEL = 0x85,
	MW = 0x95,
	ACK = 0x06,
	SYN = 0x16,
	SSA = 0x86,
	SPA = 0x96,
	BEL = 0x07,
	ETB = 0x17,
	ESA = 0x87,
	EPA = 0x97,
	BS = 0x08,
	CAN = 0x18,
	HTS = 0x88,
	HT = 0x09,
	EM = 0x19,
	HTJ = 0x89,
	LF = 0x0a,
	SUB = 0x1a,
	VTS = 0x8a,
	VT = 0x0b,
	ESC = 0x1b,
	PLD = 0x8b,
	CSI = 0x9b,
	FF = 0x0c,
	FS = 0x1c,
	PLU = 0x8c,
	ST = 0x9c,
	CR = 0x0d,
	GS = 0x1d,
	RI = 0x8d,
	OSC = 0x9d,
	SO = 0x0e,
	RS = 0x1e,
	SS2 = 0x8e,
	PM = 0x9e,
	SI = 0x0f,
	US = 0x1f,
	DEL = 0x7f,
	SS3 = 0x8f,
	APC = 0x9f
};

} // namespace

// --------------------------------------------------------------------
// We even support the weird 'Device Control Strings', programmable function keys

enum MPFKKey
{
	eUndefinedKey,
	eF1 = 11,
	eF2 = 12,
	eF3 = 13,
	eF4 = 14,
	eF5 = 15,
	eF6 = 17,
	eF7 = 18,
	eF8 = 19,
	eF9 = 20,
	eF10 = 21,
	eF11 = 23,
	eF12 = 24,
	eF13 = 25,
	eF14 = 26,
	eF15 = 28,
	eF16 = 29,
	eF17 = 31,
	eF18 = 32,
	eF19 = 33,
	eF20 = 34
};

struct MPFK
{
	bool clear;
	bool locked;
	std::map<uint32_t, std::string> key;
};

// --------------------------------------------------------------------
// Too bad the format library isn't finished yet

class MFormat
{
  public:
	template <typename... Arguments>
	MFormat(const char *fmt, Arguments... args)
	{
		m_str.reserve(255);
		auto n = snprintf(m_str.data(), 255, fmt, args...);
		m_str.resize(n);
	}

	operator std::string() const
	{
		return m_str;
	}

  private:
	std::string m_str;
};

// --------------------------------------------------------------------
// The MTerminalView class.

std::string MTerminalView::sSelectBuffer;
std::list<MTerminalView *> MTerminalView::sTerminalList;

MTerminalView::MTerminalView(const std::string &inID, MRect inBounds,
	MStatusbar *inStatusbar, MScrollbar *inScrollbar, MSearchPanel *inSearchPanel,
	MTerminalChannel *inTerminalChannel, const std::vector<std::string> &inArgv)
	: MCanvas(inID, inBounds, false, false)
	, eScroll(this, &MTerminalView::Scroll)
	, eSearch(this, &MTerminalView::FindNext)
	, ePreferencesChanged(this, &MTerminalView::PreferencesChanged)
	, ePreviewColor(this, &MTerminalView::PreviewColor)
	, eStatusPartClicked(this, &MTerminalView::StatusPartClicked)
	, mStatusInfo(0)
	, mStatusbar(inStatusbar)
	, mScrollbar(inScrollbar)
	, mSearchPanel(inSearchPanel)
	, mTerminalChannel(inTerminalChannel)
	, mArgv(inArgv)
	, mTerminalWidth(80)
	, mTerminalHeight(24)
	, mScreenBuffer(mTerminalWidth, mTerminalHeight, true)
	, mAlternateBuffer(mTerminalWidth, mTerminalHeight, false)
	, mStatusLineBuffer(mTerminalWidth, 1, false)
	, mBuffer(&mScreenBuffer)
	, eIdle(this, &MTerminalView::Idle)
	, mPFK(nullptr)
	, mNewPFK(nullptr)
	, mEscState(eESC_NONE)
	, eAnimate(this, &MTerminalView::Animate)
	, mDECSASD(false)
	, mDECSSDT(0)
	, mAnimationManager(new MAnimationManager())
	, mGraphicalBeep(nullptr)
	, mDisabledFactor(mAnimationManager->CreateVariable(1, 0, 1))
{
	mTerminalChannel->SetMessageCallback(
		[this](const std::string &msg, const std::string &lang)
		{ this->HandleMessage(msg, lang); });

	ReadPreferences();

#if DEBUG
	mDebugUpdate = false;
#endif
	std::string encoding = Preferences::GetString("default-encoding", "utf-8");
	if (encoding == "utf-8")
		mEncoding = kEncodingUTF8;
	else if (encoding == "iso-8859-1")
		mEncoding = kEncodingISO88591;
	else
		mEncoding = kEncodingUTF8;

	Reset();

	AddRoute(MSaltApp::instance().eIdle, eIdle);
	AddRoute(MPreferencesDialog::ePreferencesChanged, ePreferencesChanged);
	AddRoute(MPreferencesDialog::eColorPreview, ePreviewColor);
	AddRoute(mStatusbar->ePartClicked, eStatusPartClicked);
	AddRoute(mSearchPanel->eSearch, eSearch);
	AddRoute(mAnimationManager->eAnimate, eAnimate);

	//	mTerminalChannel->set_message_callbacks(
	//		std::bind(&MTerminalView::HandleMessage, this, _1, _2),
	//		std::bind(&MTerminalView::HandleMessage, this, _1, _2),
	//		std::bind(&MTerminalView::HandleMessage, this, _1, _2)
	//		);
	//
	AdjustScrollbar(0);

	std::string desc = MFormat("%dx%d", mTerminalWidth, mTerminalHeight);
	mStatusbar->SetStatusText(2, desc, false);

	// and add this to the std::list of open terminals
	sTerminalList.push_back(this);
}

MTerminalView::~MTerminalView()
{
	sTerminalList.erase(remove(sTerminalList.begin(), sTerminalList.end(), this), sTerminalList.end());

	delete mPFK;
	delete mNewPFK;

	RemoveRoute(eIdle, gApp->eIdle);
	RemoveRoute(eAnimate, mAnimationManager->eAnimate);

	delete mAnimationManager;
	delete mGraphicalBeep;
	delete mDisabledFactor;
}

MTerminalView *MTerminalView::GetFrontTerminal()
{
	MTerminalView *result = nullptr;
	if (not sTerminalList.empty())
		return sTerminalList.front();
	return result;
}

void MTerminalView::Open()
{
	mStatusbar->SetStatusText(0, _("Trying to connect"), false);

	MRect bounds;
	GetBounds(bounds);

	mTerminalChannel->SetTerminalSize(mTerminalWidth, mTerminalHeight,
		bounds.width - 2 * kBorderWidth, bounds.height - 2 * kBorderWidth);

	// set some environment variables
	std::vector<std::string> env;
	Preferences::GetArray("env", env);

	mTerminalChannel->Open(
		Preferences::GetString("terminal-type", "xterm-256color"),
		Preferences::GetBoolean("forward-ssh-agent", true),
		Preferences::GetBoolean("forward-x11", true),
		mArgv, env,
		[this](const std::error_code &ec)
		{
			this->HandleOpened(ec);
		});
}

void MTerminalView::Close()
{
	mTerminalChannel->Close();
	mTerminalChannel->Release();
	mTerminalChannel = nullptr;

	RemoveRoute(MSaltApp::instance().eIdle, eIdle);
	RemoveRoute(MPreferencesDialog::ePreferencesChanged, ePreferencesChanged);
	RemoveRoute(MPreferencesDialog::eColorPreview, ePreviewColor);
	RemoveRoute(mStatusbar->ePartClicked, eStatusPartClicked);
	RemoveRoute(mSearchPanel->eSearch, eSearch);
	RemoveRoute(mAnimationManager->eAnimate, eAnimate);
}

void MTerminalView::Destroy()
{
	Close();
	mAnimationManager->Stop();
}

void MTerminalView::ReadPreferences()
{
	mScreenBuffer.SetBufferSize(Preferences::GetInteger("buffer-size", 5000));

	mCursor.block = mBlockCursor = Preferences::GetBoolean("block-cursor", false);
	mCursor.blink = mBlinkCursor = Preferences::GetBoolean("blink-cursor", true);

	mFont = Preferences::GetString("font", Preferences::GetString("font", "Consolas 10"));
	mIgnoreColors = Preferences::GetBoolean("ignore-color", false);

	// set the color
	PreviewColor(MColor(Preferences::GetString("back-color", "#0f290e")));

	MDevice dev;
	dev.SetFont(mFont);

	mCharWidth = dev.GetXWidth();
	mLineHeight = dev.GetLineHeight();

	mAudibleBeep = Preferences::GetBoolean("audible-beep", true);
	if (Preferences::GetBoolean("graphical-beep", true))
	{
		if (mGraphicalBeep == nullptr)
			mGraphicalBeep = mAnimationManager->CreateVariable(0, 0, 1.0);
	}
	else if (mGraphicalBeep != nullptr)
	{
		delete mGraphicalBeep;
		mGraphicalBeep = nullptr;
	}

	mUDKWithShift = Preferences::GetBoolean("udk-with-shift", true);
	mDeleteIsDel = false;

	// XTerm
	mAltSendsEscape = true;
	mOldFnKeys = false;
	mXTermKeys = true;

	// VT320 ?
	mDECSSDT = Preferences::GetBoolean("show-status-line", false) ? 1 : 0;

	if (mCursor.blink == false)
		Invalidate();
}

void MTerminalView::PreferencesChanged()
{
	ReadPreferences();

	MRect bounds;
	GetBounds(bounds);

	uint32_t w = static_cast<uint32_t>(ceil(mTerminalWidth * mCharWidth) + 2 * kBorderWidth);
	uint32_t h = mTerminalHeight * mLineHeight + 2 * kBorderWidth;

	if (mDECSSDT > 0)
		h += mLineHeight;

	GetWindow()->ResizeWindow(w - bounds.width, h - bounds.height);

	bool showStatusBar = Preferences::GetBoolean("show-status-bar", true);
	if (mStatusbar->IsVisible() != showStatusBar)
	{
		if (showStatusBar)
			mStatusbar->Show();
		else
			mStatusbar->Hide();
	}

	Invalidate();
}

void MTerminalView::PreviewColor(MColor inColor)
{
	// color calculation
	MColor base(inColor);

	// work with floats
	float r = (base.red / 255.f), g = (base.green / 255.f), b = (base.blue / 255.f);

	// recalculate to hsv
	float h, s, v;
	rgb2hsv(r, g, b, h, s, v);

	mTerminalColors[eBack] = base;

	// text color is base but lighter (or darker) and less saturated
	if (v < 0.5f)
		hsv2rgb(h, s / 3, 1 - ((1 - v) / 6), r, g, b);
	else
		hsv2rgb(h, s / 3, v / 6, r, g, b);

	mTerminalColors[eText] = MColor(r, g, b);

	// bold color is base, but very light
	if (v < 0.5f)
		hsv2rgb(h, s / 6, 1, r, g, b);
	else
		hsv2rgb(h, s / 6, 0, r, g, b);

	mTerminalColors[eBold] = MColor(r, g, b);

	Invalidate();
}

void MTerminalView::StatusPartClicked(uint32_t inPart, MRect)
{
	auto info = mTerminalChannel->GetConnectionInfo();

	if (not info.empty())
	{
		mStatusInfo = (mStatusInfo + 1) % info.size();
		mStatusbar->SetStatusText(1, info[mStatusInfo], false);
	}
}

void MTerminalView::ResetCursor()
{
	mCursor.x = 0;
	mCursor.y = 0;
	mCursor.style = MStyle(kStyleNormal);
	mCursor.charSetG[0] = kUSCharSet;
	mCursor.charSetGSel[0] = 'B';
	mCursor.charSetG[1] = kLineCharSet;
	mCursor.charSetGSel[1] = '0';
	mCursor.charSetG[2] = kUSCharSet;
	mCursor.charSetGSel[2] = 'B';
	mCursor.charSetG[3] = kLineCharSet;
	mCursor.charSetGSel[3] = '0';
	mCursor.CSGL = 0;
	mCursor.CSGR = 2;
	mCursor.DECOM = false;
	mCursor.DECAWM = true;
	mCursor.SS = 0;
	mCursor.blink = mBlinkCursor;
	mCursor.block = mBlockCursor;
}

void MTerminalView::Reset()
{
	mS8C1T = false;

	mTabStops = std::vector<bool>(mTerminalWidth, false);
	for (int32_t i = 8; i < mTerminalWidth; i += 8)
		mTabStops[i] = true;

	mEscState = eESC_NONE;

	mMarginTop = 0;
	mMarginBottom = mTerminalHeight - 1;

	mMarginLeft = 0;
	mMarginTop = 0;
	mMarginRight = mTerminalWidth - 1;
	mMarginBottom = mTerminalHeight - 1;

	ResetCursor();

	mBracketedPaste = Preferences::GetBoolean("enable-bracketed-paste", true);

	mIRM = false;
	mKAM = false;
	mLNM = false;
	mSRM = true;

	mDECCKM = false;
	mDECANM = true;
	mDECSCLM = false;
	mDECSCNM = false;
	mDECARM = true;
	mDECPFF = false;
	mDECPEX = false;
	mDECNMK = false;
	mDECBKM = false;

	mDECSCL = 4; // FIXME ? really?
	mDECTCEM = true;
	mDECNRCM = false;

	mDECSASD = false;
	bool needResize = mDECSSDT > 0;
	mDECSSDT = Preferences::GetBoolean("show-status-line", false) ? 1 : 0;
	mDECVSSM = false;

	mCursor.saved = false;
	mNextSmoothScroll.reset();

	mDECSACE = false;

	mAltSendsEscape = true;

	// avoid problems when restore cursor is called
	mAlternate.saved = false;
	mSaved.saved = false;
	mSavedSL.saved = false;

	if (needResize and GetWindow() != nullptr)
		ResizeTerminal(mTerminalWidth, mTerminalHeight, true);

	mMouseMode = eTrackMouseNone;
}

void MTerminalView::SoftReset()
{
	mDECTCEM = true;
	mIRM = false;
	mDECNMK = false;
	mDECCKM = false;
	mMarginTop = 0;
	mMarginBottom = mTerminalHeight - 1;
	mDECNRCM = false;

	mCursor.style = MStyle();
	mCursor.charSetG[0] = kUSCharSet;
	mCursor.charSetGSel[0] = 'B';
	mCursor.charSetG[1] = kLineCharSet;
	mCursor.charSetGSel[1] = '0';
	mCursor.charSetG[2] = kUSCharSet;
	mCursor.charSetGSel[2] = 'B';
	mCursor.charSetG[3] = kLineCharSet;
	mCursor.charSetGSel[3] = '0';
	mCursor.CSGL = 0;
	mCursor.CSGR = 2;
	mCursor.DECOM = false;
	mCursor.DECAWM = true; // should be false
	mCursor.SS = 0;

	mSaved = mCursor;
}

void MTerminalView::ResizeTerminal(uint32_t inColumns, uint32_t inRows, bool inResetCursor, bool inResizeWindow)
{
	int32_t dh = inRows - mTerminalHeight;

	mTerminalWidth = inColumns;
	mTerminalHeight = inRows;

	mMarginRight = mTerminalWidth - 1;
	mMarginLeft = 0;

	mMarginBottom = mTerminalHeight - 1;
	mMarginTop = 0;

	mCursor.y += dh;
	mSaved.y += dh;
	mAlternate.y += dh;

	// not sure if this is really what we want:
	mMarginTop = 0;
	mMarginBottom = mTerminalHeight - 1;

	int32_t anchor = GetTopLine();
	mBuffer->Resize(inColumns, inRows, anchor);

	mTabStops = std::vector<bool>(mTerminalWidth, false);
	for (int32_t i = 8; i < mTerminalWidth; i += 8)
		mTabStops[i] = true;

	if (mStatusbar != nullptr)
	{
		std::string desc = MFormat("%dx%d", mTerminalWidth, mTerminalHeight);
		mStatusbar->SetStatusText(2, desc, false);
	}

	MRect bounds;
	GetBounds(bounds);

	if (mTerminalChannel->IsOpen())
		mTerminalChannel->SetTerminalSize(mTerminalWidth, mTerminalHeight,
			bounds.width - 2 * kBorderWidth, bounds.height - 2 * kBorderWidth);

	AdjustScrollbar(anchor);

	if (mBuffer == &mAlternateBuffer)
		mScreenBuffer.Resize(mTerminalWidth, mTerminalHeight, anchor);
	else
		mAlternateBuffer.Resize(mTerminalWidth, mTerminalHeight, anchor);

	mStatusLineBuffer.Resize(mTerminalWidth, 1, anchor);

	if (inResetCursor)
	{
		mMarginTop = 0;
		mMarginBottom = inRows - 1;
		mCursor.x = mCursor.y = 0;

		EraseInDisplay(2, false);
	}

	if (inResizeWindow and GetWindow() != nullptr)
	{
		uint32_t w, h;
		GetTerminalMetrics(mTerminalWidth, mTerminalHeight, mDECSSDT > 0, w, h);
		GetWindow()->ResizeWindow(w - bounds.width, h - bounds.height);
	}
}

MRect MTerminalView::GetCharacterBounds(uint32_t inLine, uint32_t inColumn)
{
	return {
		static_cast<int32_t>(kBorderWidth + ceil(inColumn * mCharWidth)),
		static_cast<int32_t>(kBorderWidth + inLine * mLineHeight),
		static_cast<int32_t>(ceil(mCharWidth)),
		mLineHeight
	};
}

bool MTerminalView::GetCharacterForPosition(int32_t inX, int32_t inY, int32_t &outLine, int32_t &outColumn)
{
	inX -= kBorderWidth;
	inY -= kBorderWidth;
	outLine = (inY / mLineHeight) - (mScrollbar->GetMaxValue() - mScrollbar->GetValue());
	if (outLine < 0 and outLine < -static_cast<int32_t>(mBuffer->BufferedLines()))
	{
		outLine = -static_cast<int32_t>(mBuffer->BufferedLines());
		outColumn = 0;
	}
	else if (outLine > mTerminalHeight - 1)
	{
		outLine = mTerminalHeight - 1;
		outColumn = mTerminalWidth - 1;
	}
	else
	{
		outColumn = static_cast<uint32_t>(floor(inX / mCharWidth));

		if (outColumn < 0)
			outColumn = 0;
		if (outColumn > mTerminalWidth - 1)
			outColumn = mTerminalWidth - 1;
	}

	return true;
}

void MTerminalView::MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers)
{
	bool done = false;

	if (not IsFocus())
		SetFocus();

	if (not mBuffer->IsSelectionEmpty())
	{
		if (inModifiers & kShiftKey and mMouseMode == eTrackMouseNone)
		{
			mMouseClick = eSingleClick;

			int32_t line, column;
			GetCharacterForPosition(inX, inY, line, column);

			if (line < mMinSelLine or (line == mMinSelLine and column < mMinSelCol))
			{
				mBuffer->SetSelection(line, column, mMaxSelLine, mMaxSelCol, mMouseBlockSelect);

				mMinSelLine = mMaxSelLine;
				mMinSelCol = mMaxSelCol;
			}
			else
			{
				mBuffer->SetSelection(mMinSelLine, mMinSelCol, line, column + 1, mMouseBlockSelect);

				mMaxSelLine = mMinSelLine;
				mMaxSelCol = mMinSelCol;
			}

			Invalidate();
			done = true;
		}
		else
		{
			mBuffer->SetSelection(0, 0, 0, 0, false);
			Invalidate();
		}
	}

	if (mMouseMode != eTrackMouseNone and inClickCount == 1)
	{
		SendMouseCommand(0, inX, inY, inModifiers);
		mMouseClick = eTrackClick;
		done = true;
	}

	if (not done)
	{
		switch (inClickCount)
		{
			case 1:
				mMouseClick = eWaitClick;
				mMouseBlockSelect = inModifiers & kControlKey;
				mLastMouseDown = std::chrono::system_clock::now();
				mLastMouseX = inX;
				mLastMouseY = inY;
				break;

			case 2:
			{
				mMouseClick = eDoubleClick;
				mMouseBlockSelect = false;

				int32_t line, column;
				GetCharacterForPosition(mLastMouseX, mLastMouseY, line, column);
				mBuffer->FindWord(line, column, mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol);

				mBuffer->SetSelection(mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol, false);
				Invalidate();
				break;
			}

			default:
			{
				mMouseClick = eTripleClick;
				mMouseBlockSelect = false;

				int32_t line, column;
				GetCharacterForPosition(mLastMouseX, mLastMouseY, line, column);
				mMinSelLine = mMaxSelLine = line;
				mMinSelCol = 0;
				mMaxSelCol = mTerminalWidth;

				mBuffer->SetSelection(mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol, false);
				Invalidate();
				break;
			}
		}
	}
}

void MTerminalView::MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	using namespace std::chrono_literals;

	if (mMouseClick == eTrackClick)
	{
		if (mMouseMode >= eTrackMouseCellMotionTracking)
			SendMouseCommand(32, inX, inY, inModifiers);
		return;
	}

	// shortcut
	if (mMouseClick == eNoClick)
		return;

	if (mMouseClick == eWaitClick)
	{
		if (std::abs(mLastMouseX - inX) > 1 or
			std::abs(mLastMouseY - inY) > 1 or
			std::chrono::system_clock::now() > mLastMouseDown + 100ms)
		{
			mMouseClick = eSingleClick;

			int32_t line, column;
			GetCharacterForPosition(mLastMouseX, mLastMouseY, line, column);
			mMinSelLine = mMaxSelLine = line;
			mMinSelCol = column;
			mMaxSelCol = column + 1;

			mBuffer->SetSelection(mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol, mMouseBlockSelect);
		}
	}

	int32_t line, column;
	GetCharacterForPosition(inX, inY, line, column);

	switch (mMouseClick)
	{
		case eSingleClick:
			if (line < mMinSelLine or (line == mMinSelLine and column < mMinSelCol))
				mBuffer->SetSelection(line, column, mMaxSelLine, mMaxSelCol, mMouseBlockSelect);
			else if (line > mMaxSelLine or (line == mMaxSelLine and column > mMaxSelCol))
				mBuffer->SetSelection(mMinSelLine, mMinSelCol, line, column + 1, mMouseBlockSelect);
			else
				mBuffer->SetSelection(mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol, mMouseBlockSelect);
			break;

		case eDoubleClick:
		{
			int32_t wl1, wc1, wl2, wc2;
			mBuffer->FindWord(line, column, wl1, wc1, wl2, wc2);

			if (wl1 < mMinSelLine or (wl1 == mMinSelLine and wc1 < mMinSelCol))
				mBuffer->SetSelection(wl1, wc1, mMaxSelLine, mMaxSelCol);
			else if (wl2 > mMaxSelLine or (wl2 == mMaxSelLine and wc2 > mMaxSelCol))
				mBuffer->SetSelection(mMinSelLine, mMinSelCol, wl2, wc2);
			else
				mBuffer->SetSelection(mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol);
			break;
		}

		case eTripleClick:
		{
			if (line < mMinSelLine)
				mBuffer->SetSelection(line, mMinSelCol, mMaxSelLine, mMaxSelCol);
			else if (line > mMaxSelLine)
				mBuffer->SetSelection(mMinSelLine, mMinSelCol, line, mMaxSelCol);
			else
				mBuffer->SetSelection(mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol);
			break;
		}

		default:;
	}

	Invalidate();
}

void MTerminalView::MouseExit()
{
	mMouseClick = eNoClick;
}

void MTerminalView::MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	if (mMouseMode >= eTrackMouseSendXYOnButton)
		SendMouseCommand(3, inX, inY, inModifiers);
	else if (not mBuffer->IsSelectionEmpty())
	{
		sSelectBuffer = mBuffer->GetSelectedText();
#if defined(_MSC_VER)
		MClipboard::Instance().SetData(sSelectBuffer, mBuffer->IsSelectionBlock());
#else
		MPrimary::Instance().SetText(sSelectBuffer);
#endif
	}

	mMouseClick = eNoClick;
}

void MTerminalView::MouseWheel(int32_t inX, int32_t inY, int32_t inDeltaX, int32_t inDeltaY, uint32_t inModifiers)
{
	if (inDeltaY != 0)
	{
		if (mMouseMode == eTrackMouseNone)
			for (int i = 0; i < 2 * std::abs(inDeltaY); ++i)
				Scroll(inDeltaY > 0 ? kScrollLineUp : kScrollLineDown);
		else
			SendMouseCommand(inDeltaY > 0 ? 64 : 65, inX, inY, inModifiers);
	}
}

void MTerminalView::ShowContextMenu(int32_t inX, int32_t inY)
{
	if (not sSelectBuffer.empty() and mTerminalChannel->IsOpen())
	{
		mBuffer->ClearSelection();
		mTerminalChannel->SendData(sSelectBuffer);
	}
}

void MTerminalView::Draw()
{
	int32_t selLine1, selLine2, selCol1, selCol2;
	bool blockSelection;
	mBuffer->GetSelection(selLine1, selCol1, selLine2, selCol2, blockSelection);
	if (selLine1 > selLine2)
		std::swap(selLine1, selLine2);
	if (selCol1 > selCol2 and selLine1 == selLine2)
		std::swap(selCol1, selCol2);

	MDevice dev(this);
	dev.SetReplaceUnknownCharacters(true);

	//	if (mDECSCNM)
	//	{
	//		copy(mNormalColors, mNormalColors + kColorCount, inverseColor);
	//		copy(mInverseColors, mInverseColors + kColorCount, normalColor);
	//	}
	//	else
	//	{
	//		copy(mNormalColors, mNormalColors + kColorCount, normalColor);
	//		copy(mInverseColors, mInverseColors + kColorCount, inverseColor);
	//	}

	// selection colours

	if (sSelectionColor == kBlack)
		MDevice::GetSysSelectionColor(sSelectionColor);

	MColor selectionColor, bc = mTerminalColors[eBack], fc = mTerminalColors[eText];

	if (IsActive())
		selectionColor = sSelectionColor;
	else
		selectionColor = sSelectionColor.Disable(mTerminalColors[eBack]);

	// correction factor for color. If negative, we bleach, otherwise we disable
	float factor = 0;

	if (mGraphicalBeep and mGraphicalBeep->GetValue() != 0.0)
	{
		factor = -static_cast<float>(mGraphicalBeep->GetValue());
		bc = bc.Bleach(-factor);
		fc = fc.Bleach(-factor);
	}

	if (mDisabledFactor->GetValue() > 0)
	{
		factor = static_cast<float>(mDisabledFactor->GetValue());
		bc = bc.Disable(mTerminalColors[eBack], factor);
		fc = fc.Disable(mTerminalColors[eBack], factor);
	}

	dev.SetBackColor(bc);
	dev.SetForeColor(fc);

	dev.EraseRect(mBounds);
	dev.SetFont(mFont);

	// fill text layout with a line of spaces
	// otherwise, it is more difficult to draw background
	// colors once we have
	std::string text(mTerminalWidth, ' ');
	dev.SetText(text);

	float x, y;
	x = y = static_cast<float>(kBorderWidth);

	int32_t H = mTerminalHeight;
	if (mDECSSDT > 0)
	{
		H += 1;

		if (mDECSSDT == 1)
		{
			// write default status line
			text = MFormat(" 1 (%03.3d,%03.3d)", mCursor.y + 1, mCursor.x + 1);

			std::string trailing = "Printer: None          Network: ";
			trailing += (mTerminalChannel->IsOpen() ? "Connected    " : "Not Connected");

			if (text.length() + trailing.length() < static_cast<size_t>(mTerminalWidth))
				text += std::string(mTerminalWidth - text.length() - trailing.length(), ' ');
			text += trailing;

			bool savedDECSSAD = mDECSASD;
			auto savedCursor = mCursor;

			mDECSASD = true;
			ResetCursor();

			for (char c : text)
				WriteChar(c);

			mCursor = savedCursor;
			mDECSASD = savedDECSSAD;
		}
	}

	for (int32_t l = 0; l < H; ++l)
	{
		MDeviceContextSaver save(dev);
		text.clear();

		int32_t lineNr = GetTopLine() + l;

		// being paranoid
		if (l != mTerminalHeight and
			lineNr < 0 and -lineNr > static_cast<int32_t>(mBuffer->BufferedLines()))
		{
			y += mLineHeight;
			continue;
		}

		const MLine &line(lineNr == mTerminalHeight ? mStatusLineBuffer.GetLine(0) : mBuffer->GetLine(lineNr));
		static MEncodingTraits<kEncodingUTF8> traits;

		float ty = y;
		if (line.IsDoubleHeight())
		{
			if (not line.IsDoubleHeightTop())
				ty -= mLineHeight;
			dev.SetScale(2.0, 2.0, x, ty);
		}
		else if (line.IsDoubleWidth())
			dev.SetScale(2.0, 1.0, x, y);

		std::vector<MColor> colors;
		std::vector<uint32_t> colorIndex, colorOffset;
		std::vector<uint32_t> backColorIndex, backColorOffset;
		std::vector<uint32_t> styleValue, styleOffset;

		auto pushColor = [&](MColor c, bool back, uint32_t offset)
		{
			uint32_t ix = find(colors.begin(), colors.end(), c) - colors.begin();
			if (ix >= colors.size())
				colors.push_back(c);

			if (back)
			{
				if (backColorIndex.empty() or backColorIndex.back() != ix)
				{
					backColorIndex.push_back(ix);
					backColorOffset.push_back(offset);
				}
			}
			else
			{
				if (colorIndex.empty() or colorIndex.back() != ix)
				{
					colorIndex.push_back(ix);
					colorOffset.push_back(offset);
				}
			}
		};

		pushColor(mTerminalColors[eBack], true, 0);

		int32_t n = mTerminalWidth;
		if (line.IsDoubleWidth() or line.IsDoubleHeight())
			n /= 2;

		MRect caretRect; // initially empty
		MColor caretColor;

		auto iter = back_inserter(text);
		for (int32_t c = 0; c < n; ++c)
		{
			// calculate selected region
			int32_t sc1 = 1, sc2 = 0;
			if (lineNr >= selLine1 and lineNr <= selLine2)
			{
				if (blockSelection)
				{
					sc1 = selCol1;
					sc2 = selCol2;
					if (sc1 > sc2)
						std::swap(sc1, sc2);
				}
				else
				{
					if (lineNr == selLine1)
						sc1 = selCol1;
					else
						sc1 = 0;
					if (lineNr == selLine2)
						sc2 = selCol2;
					else
						sc2 = mTerminalWidth;
				}
			}

			const int
				eNormalBack = -1,
				eNormalText = -2,
				eNormalBold = -3;

			int textColorIx = eNormalText, backColorIx = eNormalBack;

			unicode uc = line[c];
			MStyle st = line[c];

			if (uc == 0 or st & kStyleInvisible)
				uc = ' ';

			if (st & kStyleBold)
				textColorIx = eNormalBold;

			if (not mIgnoreColors)
			{
				if (st.GetForeColor() == kXTermColorRegularBack)
					textColorIx = eNormalBack;
				else if (st.GetForeColor() == kXTermColorRegularText)
					textColorIx = eNormalText;
				else if (st.GetForeColor() != kXTermColorNone)
					textColorIx = st.GetForeColor();

				if (st.GetBackColor() == kXTermColorRegularBack)
					backColorIx = eNormalBack;
				else if (st.GetBackColor() == kXTermColorRegularText)
					backColorIx = eNormalText;
				else if (st.GetBackColor() != kXTermColorNone)
					backColorIx = st.GetBackColor();

				if (st & kStyleBold and (textColorIx >= kXTermColorBlack and textColorIx <= kXTermColorWhite))
					textColorIx += 8;
			}

			if (((st & kStyleInverse) xor mDECSCNM) or
				(lineNr == mTerminalHeight and (st & kStyleInverse) == 0))
			{
				std::swap(textColorIx, backColorIx);
			}

			MColor textC, backC;

			switch (textColorIx)
			{
				case eNormalBack:
					textC = mTerminalColors[eBack];
					break;
				case eNormalText:
					textC = mTerminalColors[eText];
					break;
				case eNormalBold:
					textC = mTerminalColors[eBold];
					break;
				default:
					textC = k256AnsiColors[textColorIx];
					break;
			}

			switch (backColorIx)
			{
				case eNormalBack:
					backC = mTerminalColors[eBack];
					break;
				case eNormalText:
					backC = mTerminalColors[eText];
					break;
				case eNormalBold:
					backC = mTerminalColors[eBold];
					break;
				default:
					backC = k256AnsiColors[backColorIx];
					break;
			}

			if (c >= sc1 and c < sc2) // 'selected!'
				backC = selectionColor;

			if (textColorIx < 16 and st.GetForeColor() != kXTermColorRegularBack and st.GetBackColor() != kXTermColorRegularText)
				textC = textC.Distinct(backC);

			if (st & kStyleBlink and mBlinkOn)
				textC = backC;

			// wow, quite a few conditions:
			bool drawCaret = mCursor.y == lineNr and mCursor.x == c and
			                 (mBlinkOn or mCursor.blink == false) and
			                 mDECTCEM and IsActive() and IsFocus() and mTerminalChannel->IsOpen();

			if (drawCaret)
			{
				caretRect = GetCharacterBounds(mCursor.y, mCursor.x);

				if (mCursor.block)
				{
					backC = mTerminalColors[eBold];
					textC = mTerminalColors[eBack];
				}
				else
				{
					caretRect.height = 2;
					caretRect.y += static_cast<int32_t>(ceil(dev.GetAscent()));
					caretColor = mTerminalColors[eBold].Distinct(backC);
				}
			}

			pushColor(backC, true, text.length());
			pushColor(textC, false, text.length());

			uint32_t style = 0;
			if (st & kStyleBold)
				style |= MDevice::eTextStyleBold;
			if (st & kStyleUnderline)
				style |= MDevice::eTextStyleUnderline;

			if (styleValue.empty() or styleValue.back() != style)
			{
				styleValue.push_back(style);
				styleOffset.push_back(text.length());
			}

			traits.WriteUnicode(iter, uc);
		}

		dev.SetText(text);

		// adjust colors, if needed:
		if (factor > 0)
		{
			for (MColor &c : colors)
				c = c.Disable(mTerminalColors[eBack], factor);
		}
		else if (factor < 0)
		{
			for (MColor &c : colors)
				c = c.Bleach(-factor);
		}

		if (not colorIndex.empty())
			dev.SetTextColors(colorIndex.size(), &colorIndex[0], &colorOffset[0], &colors[0]);

		if (not styleValue.empty())
			dev.SetTextStyles(styleValue.size(), &styleValue[0], &styleOffset[0]);

		// draw background rects
		backColorOffset.push_back(text.length());
		for (uint32_t b = 0; b < backColorIndex.size(); ++b)
		{
			if (backColorIndex[b] == 0)
				continue;

			dev.RenderTextBackground(x, y, backColorOffset[b],
				backColorOffset[b + 1] - backColorOffset[b], colors[backColorIndex[b]]);
		}

		dev.RenderText(x, ty);

		// draw the caret if needed
		if (not caretRect.empty())
		{
			dev.SetBackColor(caretColor);
			dev.EraseRect(caretRect);
		}

		y += mLineHeight;
	}

	mBuffer->SetDirty(false);
}

void MTerminalView::ActivateSelf()
{
	MView::ActivateSelf();
	Invalidate();

	if (not sTerminalList.empty() and sTerminalList.front() != this)
	{
		sTerminalList.erase(remove(sTerminalList.begin(), sTerminalList.end(), this), sTerminalList.end());
		sTerminalList.push_front(this);
	}
}

void MTerminalView::DeactivateSelf()
{
	MView::ActivateSelf();
	Invalidate();
}

void MTerminalView::AdjustCursor(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	SetCursor(eNormalCursor);
}

void MTerminalView::Idle()
{
	using namespace std::chrono_literals;

	auto now = std::chrono::system_clock::now();

	bool update = false;
	int32_t savedCursorX = mCursor.x, savedCursorY = mCursor.y;

	if (not mInputBuffer.empty() and
		(not mNextSmoothScroll.has_value() or *mNextSmoothScroll < now))
	{
		mScrollForwardCount = 0;
		int32_t topLine = GetTopLine();

		if (mNextSmoothScroll.has_value())
		{
			if (mScrollForward)
			{
				mBuffer->ScrollForward(mMarginTop, mMarginBottom, mMarginLeft, mMarginRight);
				++mScrollForwardCount;
			}
			else
			{
				mBuffer->ScrollBackward(mMarginTop, mMarginBottom, mMarginLeft, mMarginRight);
			}

			mNextSmoothScroll.reset();
		}

		bool scrolled = mScrollbar->GetValue() < mScrollbar->GetMaxValue();

		Emulate();

		if (scrolled)
			topLine -= mScrollForwardCount;

		AdjustScrollbar(topLine);
	}

	if (now - mLastBlink >= 660ms)
	{
		mBlinkOn = not mBlinkOn;
		mLastBlink = now;
		update = IsActive() and mTerminalChannel->IsOpen();
	}

	if (mDECTCEM and IsActive() and mTerminalChannel->IsOpen() and mCursor.blink)
	{
		if (savedCursorX != mCursor.x or savedCursorY != mCursor.y)
		{
			mBlinkOn = true;
			mLastBlink = now;
			update = true;
		}
	}

	//	if (mBuffer->IsDirty())
	//	{
	//		std::string desc = (MFormat("%d,%d", mCursor.x + 1, mCursor.y + 1));
	//		mStatusbar->SetStatusText(3, desc, false);
	//	}

	if (update or mBuffer->IsDirty())
		Invalidate();

	if (not mSetWindowTitle.empty())
		GetWindow()->SetTitle(std::exchange(mSetWindowTitle, ""));
}

std::string MTerminalView::ProcessKeyCommon(uint32_t inKeyCode, uint32_t inModifiers)
{
	std::string text;

	if (inKeyCode == kBackspaceKeyCode)
		text = mDECBKM ? BS : DEL;
	else if (inKeyCode == kReturnKeyCode)
		text = mLNM ? "\r\n" : "\r";
	else if (inKeyCode == kTabKeyCode)
		text = "\t";
	else if ((inModifiers & kControlKey) and not(inModifiers & kNumPad))
	{
		switch (inKeyCode)
		{
			case '2':
			case ' ':
				text = NUL;
				break;
			case '3':
				text = ESC;
				break;
			case '4':
				text = FS;
				break;
			case '5':
				text = GS;
				break;
			case '6':
				text = RS;
				break;
			case '7':
				text = US;
				break;
			case '8':
				text = DEL;
				break;
			default:
				// check to see if this is a decent control key
				if (inKeyCode >= '@' and inKeyCode < '`')
					text = char(inKeyCode - '@');
				else if (inKeyCode == kCancelKeyCode)
					text = kControlBreakMessage;
				break;
		}
	}

	return text;
}

std::string MTerminalView::ProcessKeyVT52(uint32_t inKeyCode, uint32_t inModifiers)
{
	std::string text;

	switch (inKeyCode)
	{
		case kUpArrowKeyCode:
			text = "\033A";
			break;
		case kDownArrowKeyCode:
			text = "\033B";
			break;
		case kRightArrowKeyCode:
			text = "\033C";
			break;
		case kLeftArrowKeyCode:
			text = "\033D";
			break;
	}

	if (text.empty() and inModifiers & kNumPad)
	{
		switch (inKeyCode)
		{
			case kNumlockKeyCode:
				text = "\033P";
				break;
			case kDivideKeyCode:
				text = "\033Q";
				break;
			case kMultiplyKeyCode:
				text = "\033R";
				break;
			case kSubtractKeyCode:
				if (inModifiers & kOptionKey)
				{
					inModifiers &= ~kOptionKey;
					text = mDECNMK ? "\033?m" : "-";
				}
				else
					text = "\033S";
				break;
			case kEnterKeyCode:
				if (mDECNMK)
					text = "\033?M";
				else if (mLNM)
					text = "\r\n";
				else
					text = "\r";
				break;
			default:
				if (mDECNMK)
				{
					switch (inKeyCode)
					{
						case '+':
						case ',':
							text = "\033?l";
							break;
						case '.':
							text = "\033?n";
							break;
						default:
							if (inKeyCode >= '0' and inKeyCode <= '9')
							{
								text = "\033?p";
								text[2] += (inKeyCode - '0');
							}
							break;
					}
				}
				else if ((inKeyCode >= '0' and inKeyCode <= '9') or inKeyCode == '-' or inKeyCode == '.')
					text = char(inKeyCode);
				else if (inKeyCode == '+' or inKeyCode == ',')
					text = ',';
				break;
		}
	}

	if (text.empty())
		text = ProcessKeyCommon(inKeyCode, inModifiers);

	return text;
}

std::string MTerminalView::ProcessKeyANSI(uint32_t inKeyCode, uint32_t inModifiers)
{
	std::string text;

	switch (inKeyCode)
	{
		case kUpArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + "A";
			break;
		case kDownArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + "B";
			break;
		case kRightArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + "C";
			break;
		case kLeftArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + "D";
			break;

		case kHomeKeyCode:
			text = "\033[1~";
			break;
		case kEndKeyCode:
			text = "\033[4~";
			break;
		case kInsertKeyCode:
			text = "\033[2~";
			break;
		case kDeleteKeyCode:
			text = "\033[3~";
			break;
		case kPageUpKeyCode:
			text = "\033[5~";
			break;
		case kPageDownKeyCode:
			text = "\033[6~";
			break;
	}

	if (text.empty() and inModifiers & kNumPad)
	{
		switch (inKeyCode)
		{
			case kNumlockKeyCode:
				text = kSS3 + 'P';
				break;
			case kDivideKeyCode:
				text = kSS3 + 'Q';
				break;
			case kMultiplyKeyCode:
				text = kSS3 + 'R';
				break;
			case kSubtractKeyCode:
				if (inModifiers & kOptionKey)
				{
					inModifiers &= ~kOptionKey;
					text = mDECNMK ? "\033Om" : "-";
				}
				else
					text = kSS3 + 'S';
				break;
			case kEnterKeyCode:
				if (mDECNMK)
					text = "\033OM";
				else
					text = mLNM ? "\r\n" : "\r";
				break;
			case ',':
				text = mDECNMK ? "\033Ol" : ",";
				break;
			case '+':
				text = mDECNMK ? "\033Ol" : ",";
				break;
			case '.':
				text = mDECNMK ? "\033On" : ".";
				break;
			default:
				if (mDECNMK)
					text = kSS3 + char(inKeyCode - '0' + 'p');
				else
					text = char(inKeyCode);
				break;
		}
	}

	if (text.empty())
	{
		std::string suffix = (inModifiers & kShiftKey) ? ";2~" : "~";

		if ((inModifiers & kOptionKey) == 0)
		{
			switch (inKeyCode)
			{
				case kF1KeyCode:
					text = kCSI + "11" + suffix;
					break;
				case kF2KeyCode:
					text = kCSI + "12" + suffix;
					break;
				case kF3KeyCode:
					text = kCSI + "13" + suffix;
					break;
				case kF4KeyCode:
					text = kCSI + "14" + suffix;
					break;
				case kF5KeyCode:
					text = kCSI + "15" + suffix;
					break;
				case kF6KeyCode:
					text = kCSI + "17" + suffix;
					break;
				case kF7KeyCode:
					text = kCSI + "18" + suffix;
					break;
				case kF8KeyCode:
					text = kCSI + "19" + suffix;
					break;
				case kF9KeyCode:
					text = kCSI + "20" + suffix;
					break;
				case kF10KeyCode:
					text = kCSI + "21" + suffix;
					break;
				case kF11KeyCode:
					text = kCSI + "23" + suffix;
					break;
				case kF12KeyCode:
					text = kCSI + "24" + suffix;
					break;
			}
		}
		else
		{
			switch (inKeyCode)
			{
				case kF1KeyCode:
					text = kCSI + "23" + suffix;
					break;
				case kF2KeyCode:
					text = kCSI + "24" + suffix;
					break;
				case kF3KeyCode:
					text = kCSI + "25" + suffix;
					break;
				case kF4KeyCode:
					text = kCSI + "26" + suffix;
					break;
				case kF5KeyCode:
					text = kCSI + "28" + suffix;
					break;
				case kF6KeyCode:
					text = kCSI + "29" + suffix;
					break;
				case kF7KeyCode:
					text = kCSI + "31" + suffix;
					break;
				case kF8KeyCode:
					text = kCSI + "32" + suffix;
					break;
				case kF9KeyCode:
					text = kCSI + "33" + suffix;
					break;
				case kF10KeyCode:
					text = kCSI + "34" + suffix;
					break;
				case kF11KeyCode:
					text = kCSI + "35" + suffix;
					break;
				case kF12KeyCode:
					text = kCSI + "36" + suffix;
					break;
			}

			if (not text.empty())
				inModifiers &= ~kOptionKey;
		}
	}

	if (text.empty())
		text = ProcessKeyCommon(inKeyCode, inModifiers);

	return text;
}

std::string MTerminalView::ProcessKeyXTerm(uint32_t inKeyCode, uint32_t inModifiers)
{
	std::string text;

	char modN =
		(inModifiers & kShiftKey ? 1 : 0) +
		(inModifiers & kOptionKey ? 2 : 0) +
		(inModifiers & kControlKey ? 4 : 0);

	char modS2[3] = { modN ? ';' : '\0', static_cast<char>('0' + modN + 1), 0 };
	char modS3[4] = { modN ? '1' : '\0', ';', static_cast<char>('0' + modN + 1), 0 };

	switch (inKeyCode)
	{
		case kUpArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + modS3 + "A";
			break;
		case kDownArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + modS3 + "B";
			break;
		case kRightArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + modS3 + "C";
			break;
		case kLeftArrowKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + modS3 + "D";
			break;
		case kHomeKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + modS3 + "H";
			break;
		case kEndKeyCode:
			text = (mDECCKM ? kSS3 : kCSI) + modS3 + "F";
			break;

		case kInsertKeyCode:
			text = kCSI + '2' + modS2 + "~";
			break;
		case kDeleteKeyCode:
			text = kCSI + '3' + modS2 + "~";
			break;
		case kPageUpKeyCode:
			text = kCSI + '5' + modS2 + "~";
			break;
		case kPageDownKeyCode:
			text = kCSI + '6' + modS2 + "~";
			break;

		case kF1KeyCode:
			text = kSS3 + modS3 + "P";
			break;
		case kF2KeyCode:
			text = kSS3 + modS3 + "Q";
			break;
		case kF3KeyCode:
			text = kSS3 + modS3 + "R";
			break;
		case kF4KeyCode:
			text = kSS3 + modS3 + "S";
			break;
		case kF5KeyCode:
			text = kCSI + "15" + modS2 + "~";
			break;
		case kF6KeyCode:
			text = kCSI + "17" + modS2 + "~";
			break;
		case kF7KeyCode:
			text = kCSI + "18" + modS2 + "~";
			break;
		case kF8KeyCode:
			text = kCSI + "19" + modS2 + "~";
			break;
		case kF9KeyCode:
			text = kCSI + "20" + modS2 + "~";
			break;
		case kF10KeyCode:
			text = kCSI + "21" + modS2 + "~";
			break;
		case kF11KeyCode:
			text = kCSI + "23" + modS2 + "~";
			break;
		case kF12KeyCode:
			text = kCSI + "24" + modS2 + "~";
			break;
	}

	// override special case
	if (mOldFnKeys and inKeyCode >= kF1KeyCode and inKeyCode <= kF12KeyCode)
	{
		const char *kFnKeyCode[24] = {
			"11", "12", "13", "14", "15", "17", "18", "19", "20", "21", "23", "24",
			"23", "24", "25", "26", "28", "29", "31", "32", "33", "34", "42", "43"
		};

		int keyNr = inKeyCode - kF1KeyCode + (inModifiers & kControlKey ? 12 : 0);

		int modN =
			(inModifiers & kShiftKey ? 1 : 0) +
			(inModifiers & kOptionKey ? 2 : 0);
		char modS[3] = { modN ? ';' : '\0', static_cast<char>('0' + modN + 1), 0 };

		text = kCSI + kFnKeyCode[keyNr] + modS + "~";
	}

	if (inModifiers & kNumPad)
	{
		if (mDECNMK)
		{
			switch (inKeyCode)
			{
				case kMultiplyKeyCode:
					text = kSS3 + 'j';
					break;
				case '+':
					text = kSS3 + 'k';
					break;
				case ',':
					text = kSS3 + 'l';
					break;
				case kSubtractKeyCode:
					text = kSS3 + 'm';
					break;
				case '.':
					text = kCSI + "3~";
					break;
				case kDivideKeyCode:
					text = kSS3 + 'o';
					break;
				case '0':
					text = kCSI + "2~";
					break;
				case '1':
					text = kSS3 + 'F';
					break;
				case '2':
					text = kCSI + 'B';
					break;
				case '3':
					text = kCSI + "6~";
					break;
				case '4':
					text = kCSI + 'D';
					break;
				case '5':
					text = kCSI + 'E';
					break;
				case '6':
					text = kCSI + 'C';
					break;
				case '7':
					text = kSS3 + 'H';
					break;
				case '8':
					text = kCSI + 'A';
					break;
				case '9':
					text = kCSI + "5~";
					break;
				case kEnterKeyCode:
					text = kSS3 + 'M';
					break;
			}
		}
		else
		{
			switch (inKeyCode)
			{
				case kMultiplyKeyCode:
					text = '*';
					break;
				case kSubtractKeyCode:
					text = '-';
					break;
				case kDivideKeyCode:
					text = '/';
					break;
				case kEnterKeyCode:
					text = mLNM ? "\r\n" : "\r";
					break;
				default:
					if (inKeyCode >= '0' and inKeyCode <= '9')
						text = char(inKeyCode);
					break;
			}
		}
	}

	if (text.empty() and inModifiers & kOptionKey and isprint(inKeyCode))
	{
		if (mAltSendsEscape)
		{
			char s[3] = { ESC, static_cast<char>(inKeyCode), 0 };
			text = s;
		}
		else if (mEncoding == kEncodingUTF8)
		{
			auto iter = back_inserter(text);
			MEncodingTraits<kEncodingUTF8>::WriteUnicode(iter, inKeyCode + 128);
		}
		else
			text = char(inKeyCode + 128);
	}

	if (text.empty())
		text = ProcessKeyCommon(inKeyCode, inModifiers);

	return text;
}

bool MTerminalView::HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers,
	bool inRepeat)
{
	// PRINT(("HandleKeyDown(0x%x, 0x%x)", inKeyCode, inModifiers));

	// shortcut
	if (inRepeat and mDECARM == false)
		return true;

	bool handled = false;

	if (not mTerminalChannel->IsOpen())
	{
		if (inKeyCode == kSpaceKeyCode or inKeyCode == kReturnKeyCode)
		{
			Open();
			handled = true;
		}
	}
	else
	{
		if ((inModifiers & (kControlKey | kOptionKey | kShiftKey)) == kShiftKey)
		{
			// mimic xterm behaviour
			switch (inKeyCode)
			{
				case kHomeKeyCode:
					Scroll(kScrollToStart);
					handled = true;
					break;
				case kEndKeyCode:
					Scroll(kScrollToEnd);
					handled = true;
					break;
				case kPageUpKeyCode:
					Scroll(kScrollPageUp);
					handled = true;
					break;
				case kPageDownKeyCode:
					Scroll(kScrollPageDown);
					handled = true;
					break;
			}
		}
		else if ((inModifiers & (kControlKey | kOptionKey | kShiftKey)) == (kControlKey | kShiftKey))
		{
			// mimic xterm behaviour
			switch (inKeyCode)
			{
				case kUpArrowKeyCode:
					Scroll(kScrollLineUp);
					handled = true;
					break;
				case kDownArrowKeyCode:
					Scroll(kScrollLineDown);
					handled = true;
					break;
			}
		}

		std::string text;

		// VT220, device control strings
		if (inModifiers == (mUDKWithShift ? kShiftKey : 0) and
			inKeyCode >= kF1KeyCode and inKeyCode <= kF20KeyCode and
			mPFK != nullptr and
			mPFK->key.find(inKeyCode) != mPFK->key.end())
		{
			text = mPFK->key[inKeyCode];
			handled = true;
		}

		if (not handled)
		{
			if (mDECANM == false) // We're in VT52 mode
				text = ProcessKeyVT52(inKeyCode, inModifiers);
			else if (mXTermKeys)
				text = ProcessKeyXTerm(inKeyCode, inModifiers);
			else
				text = ProcessKeyANSI(inKeyCode, inModifiers);
		}

		if (not handled and inKeyCode == kEscapeKeyCode and inModifiers == 0)
			text = "\x1b";

		// brain dead for now
		if (not text.empty())
		{
			mBuffer->ClearSelection();

			if (mKAM)
				Beep();
			else
			{
				SendCommand(text);

				if (not mSRM)
					mInputBuffer.insert(mInputBuffer.end(), text.begin(), text.end());

				// force a scroll to the bottom
				Scroll(kScrollToEnd);
			}

			handled = true;
		}
	}

	if (handled)
	{
		mLastBlink = {};
		mBlinkOn = true;

		if (mBuffer->IsDirty())
			Invalidate();

		ObscureCursor();
	}
	else
		handled = MHandler::HandleKeyDown(inKeyCode, inModifiers, inRepeat);

	return handled;
}

void MTerminalView::HandleMessage(const std::string &inMessage, const std::string &inLanguage)
{
	mInputBuffer.insert(mInputBuffer.end(), inMessage.begin(), inMessage.end());

	std::string tail("\r\n");

	mInputBuffer.insert(mInputBuffer.end(), tail.begin(), tail.end());

	Invalidate();
}

bool MTerminalView::HandleCharacter(const std::string &inText, bool inRepeat)
{
	// shortcut
	if (inRepeat and mDECARM == false)
		return true;

	bool handled = false;

	if (not mTerminalChannel->IsOpen())
	{
		if (inText == " " or inText == "\n")
		{
			Open();
			handled = true;
		}
	}
	else
	{
		mBuffer->ClearSelection();

		if (mKAM)
			Beep();
		else
		{
			SendCommand(inText);
			if (not mSRM)
				mInputBuffer.insert(mInputBuffer.end(), inText.begin(), inText.end());

			// force a scroll to the bottom
			Scroll(kScrollToEnd);
		}

		mLastBlink = {};
		mBlinkOn = true;

		if (mBuffer->IsDirty())
			Invalidate();

		ObscureCursor();

		handled = true;
	}

	return handled;
}

// const std::vector<std::string> kDisallowedPasteCharacters{
// 	"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS", "HT", "LF", "VT",
// 	"FF", "CR", "SO", "SI", "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
// 	"CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US", "DEL"};

struct DisallowedChar
{
	const char *name;
	char ch;
} kDisallowedPasteCharacters[] = {
	{ "NUL", 0x00 }, //  '\0' (null character)
	{ "SOH", 0x01 }, //  (start of heading)
	{ "STX", 0x02 }, //  (start of text)
	{ "ETX", 0x03 }, //  (end of text)
	{ "EOT", 0x04 }, //  (end of transmission)
	{ "ENQ", 0x05 }, //  (enquiry)
	{ "ACK", 0x06 }, //  (acknowledge)
	{ "BEL", 0x07 }, //  '\a' (bell)
	{ "BS", 0x08 },  //  '\b' (backspace)
	{ "HT", 0x09 },  //  '\t' (horizontal tab)
	{ "LF", 0x0A },  //  '\n' (new line)
	{ "VT", 0x0B },  //  '\v' (vertical tab)
	{ "FF", 0x0C },  //  '\f' (form feed)
	{ "CR", 0x0D },  //  '\r' (carriage ret)
	{ "SO", 0x0E },  //  (shift out)
	{ "SI", 0x0F },  //  (shift in)
	{ "DLE", 0x10 }, //  (data link escape)
	{ "DC1", 0x11 }, //  (device control 1)
	{ "DC2", 0x12 }, //  (device control 2)
	{ "DC3", 0x13 }, //  (device control 3)
	{ "DC4", 0x14 }, //  (device control 4)
	{ "NAK", 0x15 }, //  (negative ack.)
	{ "SYN", 0x16 }, //  (synchronous idle)
	{ "ETB", 0x17 }, //  (end of trans. blk)
	{ "CAN", 0x18 }, //  (cancel)
	{ "EM", 0x19 },  //  (end of medium)
	{ "SUB", 0x1A }, //  (substitute)
	{ "ESC", 0x1B }, //  (escape)
	{ "FS", 0x1C },  //  (file separator)
	{ "GS", 0x1D },  //  (group separator)
	{ "RS", 0x1E },  //  (record separator)
	{ "US", 0x1F },  //  (unit separator)
	{ "DEL", 0x7F }
};

bool MTerminalView::PastePrimaryBuffer(const std::string &inText)
{
	bool result = false;

	std::string text(inText);

	if (mTerminalChannel->IsOpen())
	{
		// clean up the text to be pasted

		std::vector<std::string> dpc;
		Preferences::GetArray("disallowed-paste-characters", dpc);
		if (dpc.empty())
		{
			dpc = std::vector<std::string>{ "BS", "DEL", "ENQ", "EOT", "ESC", "NUL" };
			Preferences::SetArray("disallowed-paste-characters", dpc);
		}

		for (const auto &dc : dpc)
		{
			for (const auto &[name, ch] : kDisallowedPasteCharacters)
			{
				if (zeep::iequals(dc, name) or (zeep::iequals(dc, "C0") and ch <= 0x1F))
					zeep::replace_all(text, { &ch, 1 }, " ");
			}
		}

		mBuffer->ClearSelection();

		if (mKAM)
			Beep();
		else
		{
			if (mBracketedPaste)
				SendCommand(kCSI + "200~" + text + kCSI + "201~");
			else
				SendCommand(text);

			if (not mSRM)
				mInputBuffer.insert(mInputBuffer.end(), text.begin(), text.end());

			// force a scroll to the bottom
			Scroll(kScrollToEnd);
		}

		mLastBlink = {};
		mBlinkOn = true;

		if (mBuffer->IsDirty())
			Invalidate();

		ObscureCursor();

		result = true;
	}

	return result;
}

bool MTerminalView::UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked)
{
	bool handled = true;
	switch (inCommand)
	{
		case cmd_EnterTOTP:
			outEnabled = mTerminalChannel->IsOpen();
			break;

		case cmd_Copy:
			outEnabled = not mBuffer->IsSelectionEmpty();
			break;

		case cmd_Paste:
			outEnabled = MClipboard::Instance().HasData();
			break;

		case cmd_SelectAll:
			outEnabled = true;
			break;

		case cmd_FindNext:
		case cmd_FindPrev:
			outEnabled = true;
			break;

#if DEBUG
		case cmd_DebugUpdate:
			outEnabled = true;
			outChecked = mDebugUpdate;
			break;
#endif

		case cmd_NextTerminal:
		case cmd_PrevTerminal:
			outEnabled = true;
			break;

		case cmd_Reset:
		case cmd_ResetAndClear:
			outEnabled = true;
			break;

		case cmd_EncodingUTF8:
			outEnabled = true;
			outChecked = mEncoding == kEncodingUTF8;
			break;

		case cmd_BackSpaceIsDel:
			outEnabled = true;
			outChecked = not mDECBKM;
			break;

		case cmd_DeleteIsDel:
			outEnabled = true;
			outChecked = mDeleteIsDel;
			break;

		case cmd_VT220Keyboard:
			outEnabled = true;
			outChecked = not mXTermKeys;
			break;

		case cmd_MetaSendsEscape:
			outEnabled = mXTermKeys;
			outChecked = mAltSendsEscape;
			break;

		case cmd_OldFnKeys:
			outEnabled = mXTermKeys;
			outChecked = mOldFnKeys;
			break;

		case cmd_SendSTOP:
			outEnabled = mTerminalChannel->IsOpen();
			break;
		case cmd_SendCONT:
			outEnabled = mTerminalChannel->IsOpen();
			break;
		case cmd_SendINT:
			outEnabled = mTerminalChannel->IsOpen();
			break;
		case cmd_SendHUP:
			outEnabled = mTerminalChannel->IsOpen();
			break;
		case cmd_SendTERM:
			outEnabled = mTerminalChannel->IsOpen();
			break;
		case cmd_SendKILL:
			outEnabled = mTerminalChannel->IsOpen();
			break;

		default:
			handled = MHandler::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
	}
	return handled;
}

bool MTerminalView::ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers)
{
	bool handled = true;
	switch (inCommand)
	{
		case cmd_Copy:
			MClipboard::Instance().SetData(mBuffer->GetSelectedText(),
				mBuffer->IsSelectionBlock());
			break;

		case cmd_Paste:
		{
			std::string text;
			bool block;
			MClipboard::Instance().GetData(text, block);
			handled = PastePrimaryBuffer(text);
			break;
		}

		case cmd_EnterTOTP:
			EnterTOTP(inItemIndex - 2);
			break;

		case cmd_SelectAll:
			mBuffer->SelectAll();
			Invalidate();
			break;

		case cmd_FindNext:
			FindNext(searchDown);
			break;

		case cmd_FindPrev:
			FindNext(searchUp);
			break;

		case cmd_Reset:
		{
			value_changer<int32_t> savedX(mCursor.x, mCursor.x), savedY(mCursor.y, mCursor.y);
			Reset();
			break;
		}

		case cmd_ResetAndClear:
			Reset();
			mBuffer->Clear();
			Invalidate();
			break;

		case cmd_EncodingUTF8:
			if (mEncoding == kEncodingUTF8)
				mEncoding = kEncodingISO88591;
			else
				mEncoding = kEncodingUTF8;
			break;

		case cmd_MetaSendsEscape:
			mAltSendsEscape = not mAltSendsEscape;
			break;

		case cmd_BackSpaceIsDel:
			mDECBKM = not mDECBKM;
			break;

		case cmd_DeleteIsDel:
			mDeleteIsDel = not mDeleteIsDel;
			break;

		case cmd_OldFnKeys:
			mOldFnKeys = not mOldFnKeys;
			break;

		case cmd_VT220Keyboard:
			mXTermKeys = not mXTermKeys;
			break;

		case cmd_SendSTOP:
			mTerminalChannel->SendSignal("STOP");
			break;
		case cmd_SendCONT:
			mTerminalChannel->SendSignal("CONT");
			break;
		case cmd_SendINT:
			mTerminalChannel->SendSignal("INT");
			break;
		case cmd_SendHUP:
			mTerminalChannel->SendSignal("HUP");
			break;
		case cmd_SendTERM:
			mTerminalChannel->SendSignal("TERM");
			break;
		case cmd_SendKILL:
			mTerminalChannel->SendSignal("KILL");
			break;

#if DEBUG
		case cmd_DebugUpdate:
			mDebugUpdate = not mDebugUpdate;
			break;
#endif

		default:
			handled = MHandler::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
	}
	return handled;
}

void MTerminalView::EnterTOTP(uint32_t inItemIndex)
{
	std::vector<std::string> totp;
	Preferences::GetArray("totp", totp);

	// silently break on errors
	for (;;)
	{
		if (inItemIndex >= totp.size())
			break;

		auto s = totp[inItemIndex].rfind(';');
		if (s == std::string::npos)
			break;

		std::string hash = totp[inItemIndex].substr(s + 1, std::string::npos);

		auto h = zeep::decode_base32(hash);

		int timestamp = time(nullptr) / 30;

		uint8_t val[8];
		for (int i = 8; i-- > 0; timestamp >>= 8)
			val[i] = static_cast<uint8_t>(timestamp);

		auto computed = zeep::hmac_sha1(std::string_view((char *)val, 8), h);

		int offset = computed.back() & 0xf;
		uint32_t truncated = 0;
		for (int i = 0; i < 4; ++i)
		{
			truncated <<= 8;
			truncated |= static_cast<uint8_t>(computed[offset + i]);
		}

		truncated &= 0x7fffffff;
		truncated %= 1000000;

		std::stringstream ss;
		ss << std::fixed << std::setw(6) << std::setfill('0') << truncated;

		mTerminalChannel->SendData(ss.str());

		break;
	}
}

void MTerminalView::FindNext(MSearchDirection inSearchDirection)
{
	int32_t l1, l2, c1, c2;
	bool block;

	mBuffer->GetSelection(l1, c1, l2, c2, block);

	int32_t line, column;

	bool wrapped = false, found;
	bool ignoreCase = mSearchPanel->GetIgnoreCase();
	std::string what = mSearchPanel->GetSearchString();

	if (inSearchDirection == searchDown)
	{
		if (l1 == l2 and c1 == c2)
		{
			line = GetTopLine();
			column = 0;
		}
		else
		{
			line = l2;
			column = c2;
		}
	}
	else
	{
		if (l1 == l2 and c1 == c2)
		{
			line = GetTopLine() + mTerminalHeight - 1;
			column = mTerminalWidth - 1;
		}
		else
		{
			line = l1;
			column = c1;
		}
	}

	int32_t startLine = line, startColumn = column;

	for (;;)
	{
		if (inSearchDirection == searchDown)
			found = mBuffer->FindNext(line, column, what, ignoreCase, false);
		else
			found = mBuffer->FindPrevious(line, column, what, ignoreCase, false);

		if (found and wrapped)
		{
			if (inSearchDirection == searchDown and (line > startLine or (line == startLine and column >= startColumn)))
				found = false;

			if (inSearchDirection == searchUp and (line < startLine or (line == startLine and column <= startColumn)))
				found = false;
		}

		if (found)
		{
			mBuffer->SetSelection(line, column, line, column + what.length());
			Scroll(kScrollToSelection);
			break;
		}

		Beep();

		if (wrapped)
			break;

		wrapped = true;

		if (inSearchDirection == searchDown)
		{
			line = -mScrollbar->GetMaxValue();
			column = 0;
		}
		else
		{
			line = mTerminalHeight - 1;
			column = mTerminalWidth - 1;
		}
	}
}

int32_t MTerminalView::GetTopLine() const
{
	int32_t result = 0;
	if (mBuffer->BufferedLines() > 0)
		result = mScrollbar->GetValue() - mScrollbar->GetMaxValue();
	return result;
}

void MTerminalView::AdjustScrollbar(int32_t inTopLine)
{
	// recalculate scrollbar values
	int32_t min = 0;
	int32_t max = mBuffer->BufferedLines();
	int32_t page = mTerminalHeight;
	int32_t value = max + inTopLine;

	if (value < min)
		value = min;
	if (value > max)
		value = max;

	mScrollbar->SetAdjustmentValues(min, max + page - 1, 1, page, value);

	if (mBuffer->BufferedLines() > 0)
		mScrollbar->Enable();
	else
		mScrollbar->Disable();
}

void MTerminalView::Scroll(MScrollMessage inMessage)
{
	if (mBuffer->BufferedLines() == 0)
		return;

	int32_t max = mScrollbar->GetMaxValue();
	int32_t min = mScrollbar->GetMinValue();
	int32_t value = mScrollbar->GetValue();

	switch (inMessage)
	{
		case kScrollLineDown:
			if (value < max)
				mScrollbar->SetValue(value + 1);
			break;

		case kScrollLineUp:
			if (value > min)
				mScrollbar->SetValue(value - 1);
			break;

		case kScrollPageDown:
			value += mTerminalHeight;
			if (value > max)
				value = max;
			mScrollbar->SetValue(value);
			break;

		case kScrollPageUp:
			value -= mTerminalHeight;
			if (value < min)
				value = min;
			mScrollbar->SetValue(value);
			break;

		case kScrollToStart:
			mScrollbar->SetValue(min);
			break;

		case kScrollToEnd:
			mScrollbar->SetValue(max);
			break;

		case kScrollToSelection:
			if (not mBuffer->IsSelectionEmpty())
			{
				int32_t lb, le, cb, ce;
				bool block;
				mBuffer->GetSelection(lb, cb, le, ce, block);

				bool forceCenter = false;

				int32_t minLine = GetTopLine();
				if (lb == minLine - 1 and lb < 0)
					mScrollbar->SetValue(max + lb);
				else if (lb < minLine)
					forceCenter = true;

				int32_t maxLine = minLine + mTerminalHeight;
				if (lb == maxLine and lb < 0)
					mScrollbar->SetValue(max + lb);
				else if (lb > maxLine)
					forceCenter = true;

				if (forceCenter)
				{
					value = max + (lb - mTerminalHeight / 3);
					if (value > max)
						value = max;
					if (value < min)
						value = min;
					mScrollbar->SetValue(value);
				}
			}
			break;

		default:;
	}

	Invalidate();
	GetWindow()->UpdateNow();
}

void MTerminalView::GetTerminalMetrics(uint32_t inColumns, uint32_t inRows, bool inStatusLine,
	uint32_t &outWidth, uint32_t &outHeight)
{
	MDevice dev;
	dev.SetFont(Preferences::GetString("font", Preferences::GetString("font", "Consolas 10")));

	float charWidth = dev.GetXWidth();
	uint32_t lineHeight = dev.GetLineHeight();

	outWidth = static_cast<uint32_t>(ceil(inColumns * charWidth) + 2 * kBorderWidth);
	outHeight = inRows * lineHeight + 2 * kBorderWidth;
	if (inStatusLine)
		outHeight += lineHeight;
}

MRect MTerminalView::GetIdealTerminalBounds(uint32_t inColumns, uint32_t inRows)
{
	uint32_t w, h;
	GetTerminalMetrics(inColumns, inRows, Preferences::GetBoolean("show-status-line", false), w, h);
	return { 0, 0, static_cast<int32_t>(w), static_cast<int32_t>(h) };
}

void MTerminalView::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	MCanvas::ResizeFrame(inWidthDelta, inHeightDelta);

	MRect bounds;
	GetBounds(bounds);

	MDevice dev;
	dev.SetFont(mFont);

	// avoid absurd sizes
	if (static_cast<uint32_t>(bounds.width) <= 2 * kBorderWidth or static_cast<uint32_t>(bounds.height) <= 2 * kBorderWidth)
		return;

	int32_t w = static_cast<int32_t>((bounds.width - 2 * kBorderWidth) / dev.GetXWidth());
	int32_t h = static_cast<int32_t>((bounds.height - 2 * kBorderWidth) / dev.GetLineHeight());

	if (mDECSSDT > 0)
		h -= 1;

	if (w != mTerminalWidth or h != mTerminalHeight)
		ResizeTerminal(w, h, false, false);
}

void MTerminalView::SendCommand(std::string inData)
{
	if (mTerminalChannel->IsOpen())
	{
		if (mS8C1T)
		{
			zeep::replace_all(inData, "\033D", "\204");  // IND
			zeep::replace_all(inData, "\033E", "\205");  // NEL
			zeep::replace_all(inData, "\033H", "\210");  // HTS
			zeep::replace_all(inData, "\033M", "\215");  // RI
			zeep::replace_all(inData, "\033N", "\216");  // SS2
			zeep::replace_all(inData, "\033O", "\217");  // SS3
			zeep::replace_all(inData, "\033P", "\220");  // DCS
			zeep::replace_all(inData, "\033[", "\233");  // CSI
			zeep::replace_all(inData, "\033\\", "\234"); // ST
			zeep::replace_all(inData, "\033]", "\235");  // OSC
		}

		mTerminalChannel->SendData(std::move(inData));
	}
}

void MTerminalView::SendMouseCommand(int32_t inButton, int32_t inX, int32_t inY, uint32_t inModifiers)
{
	int32_t line, column;
	GetCharacterForPosition(inX, inY, line, column);

	if (inButton == 32)
	{
		if (mMouseTrackX == column and mMouseTrackY == line)
			return;
	}

	mMouseTrackX = column;
	mMouseTrackY = line;

	char cb = 32 + inButton;

	if (inModifiers & kShiftKey)
		cb |= 4;
	if (inModifiers & kOptionKey)
		cb |= 8;
	if (inModifiers & kControlKey)
		cb |= 16;

	const int32_t kMaxPosition = '~' - '!';

	if (line < 0)
		line = 0;
	if (line > kMaxPosition)
		line = kMaxPosition;
	if (column < 0)
		column = 0;
	if (column > kMaxPosition)
		column = kMaxPosition;

	char cx = '!' + column;
	char cy = '!' + line;

	SendCommand(kCSI + 'M' + cb + cx + cy);
}

void MTerminalView::Opened()
{
	value_changer<int32_t> x(mCursor.x, 0), y(mCursor.y, 0);

	mStatusbar->SetStatusText(0, _("Connected"), false);

	auto info = mTerminalChannel->GetConnectionInfo();
	if (not info.empty())
	{
		mStatusInfo = 0;
		mStatusbar->SetStatusText(1, info[0], false);
	}

	Reset();
	// EraseInDisplay(2);
	Invalidate();

	using namespace std::chrono_literals;

	MStoryboard *storyboard = mAnimationManager->CreateStoryboard();
	storyboard->AddTransition(mDisabledFactor, 0, 100ms, "acceleration-decelleration");
	mAnimationManager->Schedule(storyboard);
}

void MTerminalView::Closed()
{
	if (mTerminalChannel->IsOpen())
		mTerminalChannel->Close();

	mStatusbar->SetStatusText(0, _("Connection closed"), false);

	mStatusbar->SetStatusText(1, "", false);
	mBlinkOn = true;
	Invalidate();
	const char kReconnectMsg[] = "\r\nPress enter or space to reconnect\r\n";
	std::string reconnectMsg = _(kReconnectMsg);
	mInputBuffer.insert(mInputBuffer.end(), reconnectMsg.begin(), reconnectMsg.end());

	mStatusbar->SetStatusText(1, "", false);

	using namespace std::chrono_literals;

	MStoryboard *storyboard = mAnimationManager->CreateStoryboard();
	storyboard->AddTransition(mDisabledFactor, 1, 1000ms, "acceleration-decelleration");

	// if (not mArgv.empty())
	// {
	// 	storyboard->AddFinishedCallback([w = GetWindow()]()
	// 		{ static_cast<MSaltApp  *>(gApp)->execute([w]()
	// 			  {
	// 			if (MWindow::WindowExists(w))
	// 				w->ProcessCommand(cmd_Close, nullptr, 0, 0); }); });
	// }

	mAnimationManager->Schedule(storyboard);
}

void MTerminalView::HandleOpened(const std::error_code &ec)
{
	if (ec)
	{
		const std::string &msg = ec.message();
		mInputBuffer.insert(mInputBuffer.end(), msg.begin(), msg.end());

		Closed();

		mTerminalChannel->Close();
	}
	else
	{
		Opened();

		if (Preferences::GetBoolean("forward-gpg-agent", true))
		{
		}

		mTerminalChannel->ReadData([this](std::error_code ec, std::streambuf &inData)
			{ this->HandleReceived(ec, inData); });
	}
}

void MTerminalView::HandleReceived(const std::error_code &ec, std::streambuf &inData)
{
	if (ec)
	{
		const std::string &msg = ec.message();
		mInputBuffer.insert(mInputBuffer.end(), msg.begin(), msg.end());

		Closed();
	}
	else
	{
		while (inData.in_avail() > 0)
			mInputBuffer.push_back(inData.sbumpc());

		mTerminalChannel->ReadData([this](std::error_code ec, std::streambuf &inData)
			{ this->HandleReceived(ec, inData); });

		Idle();
	}
}

// --------------------------------------------------------------------
//

void MTerminalView::EraseInDisplay(uint32_t inMode, bool inSelective)
{
	mBuffer->EraseDisplay(mCursor.y, mCursor.x, inMode, inSelective);
}

void MTerminalView::EraseInLine(uint32_t inMode, bool inSelective)
{
	mBuffer->EraseLine(mCursor.y, mCursor.x, inMode, inSelective);
}

void MTerminalView::ScrollForward()
{
	if (mDECSCLM)
	{
		mNextSmoothScroll = std::chrono::system_clock::now() + kSmoothScrollDelay;
		mScrollForward = true;
	}
	else
	{
		mBuffer->ScrollForward(mMarginTop, mMarginBottom, mMarginLeft, mMarginRight);
		++mScrollForwardCount;
	}
}

void MTerminalView::ScrollBackward()
{
	if (mDECSCLM)
	{
		mNextSmoothScroll = std::chrono::system_clock::now() + kSmoothScrollDelay;
		mScrollForward = false;
	}
	else
		mBuffer->ScrollBackward(mMarginTop, mMarginBottom, mMarginLeft, mMarginRight);
}

void MTerminalView::WriteChar(unicode inChar)
{
	MTerminalBuffer *buffer = mDECSASD ? &mStatusLineBuffer : mBuffer;

	int32_t mr = mCursor.DECOM ? mMarginRight : mTerminalWidth - 1;
	int32_t ml = mCursor.DECOM ? mMarginLeft : 0;
	int32_t mb = mCursor.DECOM ? mMarginBottom : mTerminalHeight - 1;

	if (mCursor.x >= mr + 1)
	{
		if (mCursor.DECAWM)
		{
			buffer->WrapLine(mCursor.y);
			if (mCursor.y < mb)
				++mCursor.y;
			else
				ScrollForward();

			mCursor.x = ml;
		}
		else
			mCursor.x = mr;
	}

	if (mIRM)
		buffer->InsertCharacter(mCursor.y, mCursor.x);

	buffer->SetCharacter(mCursor.y, mCursor.x, inChar, mCursor.style);

	++mCursor.x;

	mLastChar = inChar;
}

void MTerminalView::MoveCursor(MCursorMovement inDirection)
{
	int32_t x = mCursor.x, y = mCursor.y;

	switch (inDirection)
	{
		case kMoveUp:
			if (mCursor.y > mMarginTop or (mCursor.y < mMarginTop and mCursor.y > 0))
				--mCursor.y;
			break;

		case kMoveDown:
			if (mCursor.y < mMarginBottom or (mCursor.y > mMarginBottom and mCursor.y < mTerminalHeight - 1))
				++mCursor.y;
			break;

		case kMoveRight:
			if (mCursor.x < mMarginRight or (mCursor.x > mMarginRight and mTerminalWidth - 1))
				++mCursor.x;
			break;

		case kMoveLeft:
			if (mCursor.x >= mTerminalWidth - 1)
				mCursor.x = mTerminalWidth - 1 - 1;
			else if (mCursor.x > mMarginLeft or (mCursor.x < mMarginLeft and mCursor.x > 0))
				--mCursor.x;
			break;

		case kMoveIND:
			if (mCursor.y < mMarginBottom)
				++mCursor.y;
			else if (mCursor.y == mMarginBottom)
				ScrollForward();
			break;

		// same as MoveUp
		case kMoveRI:
			if (mCursor.y > mMarginTop)
				--mCursor.y;
			else if (mCursor.y == mMarginTop)
				ScrollBackward();
			break;

		case kMoveLF:
			if (mCursor.y < mMarginBottom)
				++mCursor.y;
			else if (mCursor.y == mMarginBottom)
				ScrollForward();
			break;

		case kMoveCR:
			mCursor.x = mMarginLeft;
			break;

		case kMoveCRLF:
			mCursor.x = mMarginLeft;
			if (mCursor.y < mMarginBottom)
				++mCursor.y;
			else if (mCursor.y == mMarginBottom)
				ScrollForward();
			break;

		case kMoveHT:
			while (mCursor.x < mMarginRight)
			{
				++mCursor.x;
				if (mTabStops[mCursor.x])
					break;
			}
			break;

		case kMoveCBT:
			while (mCursor.x > mMarginLeft)
			{
				--mCursor.x;
				if (mTabStops[mCursor.x])
					break;
			}
			break;

		case kMoveBI:
			if (mCursor.x > mMarginLeft)
				--mCursor.x;
			else if (mCursor.x >= 0)
				mBuffer->InsertCharacter(mCursor.y, mMarginLeft, mMarginRight);
			break;

		case kMoveFI:
			if (mCursor.x < mMarginRight - 1)
				++mCursor.x;
			else if (mCursor.x < mTerminalWidth)
				mBuffer->DeleteCharacter(mCursor.y, mMarginLeft, mMarginRight);
			break;

		case kMoveSL:
			for (int32_t line = 0; line < mTerminalHeight; ++line)
				mBuffer->DeleteCharacter(line, mMarginLeft, mMarginRight);
			if (mCursor.x > 0)
				--mCursor.x;
			break;

		case kMoveSR:
			for (int32_t line = 0; line < mTerminalHeight - 1; ++line)
				mBuffer->InsertCharacter(line, mMarginLeft, mMarginRight);
			if (mCursor.x < mMarginRight)
				++mCursor.x;
			break;
	}

	if (x != mCursor.x or y != mCursor.y)
		mBuffer->SetDirty(true);
}

void MTerminalView::MoveCursorTo(int32_t inX, int32_t inY)
{
	if (mCursor.DECOM)
	{
		inX += mMarginLeft;
		inY += mMarginTop;
	}

	if (inX < 0)
		inX = 0;
	if (inX > mTerminalWidth - 1)
		inX = mTerminalWidth - 1;

	if (inY < 0)
		inY = 0;
	if (inY > mTerminalHeight - 1)
		inY = mTerminalHeight - 1;

	if (mCursor.DECOM)
	{
		if (inY < mMarginTop)
			inY = mMarginTop;
		if (inY > mMarginBottom)
			inY = mMarginBottom;

		if (inX < mMarginLeft)
			inX = mMarginLeft;
		if (inX > mMarginRight)
			inX = mMarginRight;
	}

	if (mCursor.x != inX or mCursor.y != inY)
	{
		mCursor.x = inX;
		mCursor.y = inY;

		if (mCursor.y > mTerminalHeight - 1)
			mCursor.y = mTerminalHeight - 1;

		mBuffer->SetDirty(true);
	}
}

void MTerminalView::SetTabstop()
{
	if (mCursor.x < mTerminalWidth)
		mTabStops[mCursor.x] = true;
}

void MTerminalView::Emulate()
{
	while (not mInputBuffer.empty())
	{
		// break on a smooth scroll event
		if (mNextSmoothScroll.has_value())
			break;
	

#if DEBUG
	if (mDebugUpdate and mBuffer->IsDirty())
	{
		Invalidate();
		GetWindow()->UpdateNow();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
#endif
	// process bytes. We try to keep this code UTF-8 savvy
	uint8_t ch = mInputBuffer.front();
	mInputBuffer.pop_front();

	// start by testing if this byte is a control code
	// The C1 control codes are never leading bytes in a UTF-8
	// sequence and so we can easily check for them here.

	if (ch < 0x20 or ch == 0x7f or // a C0 control code
	                               // or a C1 control code
		(mDECSCL >= 2 and mS8C1T and (ch & 0xe0) == 0x80))
	{
		if (mLastCtrl)
			mLastChar = 0;
		mLastCtrl = true;

		// shortcut, do not process control code if in a DCS or OSC std::string
		if (ch != ESC)
		{
			switch (mEscState)
			{
				case eDCS:
					EscapeDCS(ch);
					continue;
				case eOSC:
					EscapeOSC(ch);
					continue;
				default:
					break;
			}
		}

		switch (ch)
		{
			case ENQ:
			{
				std::string answer_back = Preferences::GetString("answer-back", "salt");
				for (auto p = answer_back.find_first_of("\r\n"); p != std::string::npos; p = answer_back.find_first_of("\n\r", p))
					answer_back.erase(answer_back.begin() + p);
				mTerminalChannel->SendData(answer_back);
				break;
			}

			case BEL:
				if (mEscState == eOSC)
					EscapeOSC(ch);
				else
					Beep();
				break;

			case BS:
				MoveCursor(kMoveLeft);
				break;
			case HT:
				MoveCursor(kMoveHT);
				break;
			case LF:
			case VT:
			case FF:
				MoveCursor(mLNM ? kMoveCRLF : kMoveLF);
				break;
			case CR:
				MoveCursor(kMoveCR);
				break;
			case SO:
				mCursor.CSGL = 1;
				break;
			case SI:
				mCursor.CSGL = 0;
				break;
			case SUB:
				WriteChar(0x00bf /*¿*/); // fall through
			case CAN:
				mEscState = eESC_NONE;
				break;

			case ESC:
				// If we're processing an escape std::string and then receive an escape
				// this can either mean the beginning of a new escape std::string cancelling
				// the previous, or it may be the start of a ST or BEL sequence.
				// The latter can only occur when 8 bit controls are in use.

				if (mDECSCL >= 2 and mS8C1T)
					mEscState = eESC_SEEN;
				else
				{
					switch (mEscState)
					{
						case eDCS:
							mEscState = eDCS_ESC;
							break;
						case eOSC:
							mEscState = eOSC_ESC;
							break;
						case ePM:
							mEscState = ePM_ESC;
							break;
						case eAPC:
							mEscState = eAPC_ESC;
							break;
						default:
							mEscState = eESC_SEEN;
							break;
					}
				}
				break;

			case IND:
				MoveCursor(kMoveIND);
				break;
			case NEL:
				MoveCursor(kMoveCRLF);
				break;
			case HTS:
				SetTabstop();
				break;
			case RI:
				MoveCursor(kMoveRI);
				break;
			case SS2:
				mCursor.SS = 2;
				break;
			case SS3:
				mCursor.SS = 3;
				break;
			case DCS:
				mPFKKeyNr = 0;
				mEscState = eDCS;
				break;
			case OSC:
				mEscState = eOSC;
				break;
			case CSI:
				mEscState = eCSI;
				break;
			case PM:
				mEscState = ePM;
				break;

			case ST:
				switch (mEscState)
				{
					case eDCS:
						EscapeDCS(ch);
						break;
					case eOSC:
						EscapeOSC(ch);
						break;
					case ePM:
						mEscState = eESC_NONE;
						break;
					case eAPC:
						mEscState = eESC_NONE;
						break;
					default:
						break;
				}
				break;

			default: /* ignore */
				break;
		}

		continue;
	}

	// If we're processing an escape sequence, feed the byte to the processor.

	if (mEscState != eESC_NONE)
	{
		if (mEscState == eDCS_ESC or mEscState == ePM_ESC or mEscState == eAPC_ESC or mEscState == eOSC_ESC)
		{
			if (ch == '\\')
			{
				if (mEscState == eDCS_ESC)
					EscapeDCS(ST);
				else if (mEscState == eOSC_ESC)
					EscapeOSC(ST);
				mEscState = eESC_NONE;
			}
			else if (mDECANM)
				EscapeStart(ch);
			else
				EscapeVT52(ch);

			continue;
		}

		// OK, so we're in the middle of an escape std::string
		switch (mEscState)
		{
			case eESC_SEEN:
				if (mDECANM)
					EscapeStart(ch);
				else
					EscapeVT52(ch);
				break;

			case eCSI:
				EscapeCSI(ch);
				break;
			case eOSC:
				EscapeOSC(ch);
				break;
			case eDCS:
				EscapeDCS(ch);
				break;
			case ePM: /* ignore */
				break;

			case eAPC:
				EscapeAPC(ch);
				break;

			case eSELECT_CHAR_SET:
				SelectCharSet(ch);
				break;
			case eSELECT_CTRL_TRANSM:
				SelectControlTransmission(ch);
				break;
			case eSELECT_DOUBLE:
				SelectDouble(ch);
				break;

			// special VT52 handling
			case eVT52_LINE:
				mArgs.push_back(ch - US);
				mEscState = eVT52_COLUMN;
				break;

			case eVT52_COLUMN:
				MoveCursorTo((ch - US) - 1, mArgs[0] - 1);
				mEscState = eESC_NONE;
				break;

			default:
				assert(false); // now what?
		}

		continue; // ignore anything else, we're in an escape std::string
	}

	mLastCtrl = false;

	// No control character and no escape sequence, so we have to write
	// this character to the screen.
	unicode uc = ch;

	// if it is an ascii character, std::map it using GL
	if (ch >= 32 and ch < 127) // GL
	{
		int set = mCursor.CSGL;
		if (mCursor.SS != 0)
			set = mCursor.SS;
		uc = mCursor.charSetG[set][ch - 32];
	}
	else // GR
	{    // no ascii, see if NRC is in use
		if (mDECNRCM)
		{
			int set = mCursor.CSGR;
			if (mCursor.SS != 0)
				set = mCursor.SS;
			uc = mCursor.charSetG[set][ch - 32 - 128];
		}
		else // no NRC, maybe the data should be interpreted as UTF-8
		{
			if (mEncoding == kEncodingUTF8)
			{
				uint32_t len = 1;
				if ((ch & 0x0E0) == 0x0C0)
					len = 2;
				else if ((ch & 0x0F0) == 0x0E0)
					len = 3;
				else if ((ch & 0x0F8) == 0x0F0)
					len = 4;

				// be safe, in case the text in the buffer is too short for a valid UTF-8 character
				// just wait until we receive some more.
				mInputBuffer.push_front(ch);
				if (mInputBuffer.size() < len)
					break;

				static MEncodingTraits<kEncodingUTF8> traits;
				traits.ReadUnicode(mInputBuffer.begin(), len, uc);

				while (len-- > 0)
					mInputBuffer.pop_front();
			}
			else
				uc = MUnicodeMapping::GetUnicode(kEncodingISO88591, ch);
		}
	}

	// reset the single shift code
	mCursor.SS = 0;

	// and write the final unicode to the screen
	WriteChar(uc);
}
}

inline uint32_t MTerminalView::GetParam(uint32_t inParamNr, uint32_t inDefault)
{
	uint32_t result = inDefault;
	if (inParamNr < mArgs.size())
		result = mArgs[inParamNr];
	return result;
}

void MTerminalView::GetRectParam(uint32_t inParamOffset, int32_t &outTop, int32_t &outLeft, int32_t &outBottom, int32_t &outRight)
{
	//	int32_t dx = mCursor.DECOM ? mMarginLeft : 0;
	//	int32_t dy = mCursor.DECOM ? mMarginTop : 0;

	outTop = GetParam(inParamOffset + 0, 1) - 1;
	outLeft = GetParam(inParamOffset + 1, 1) - 1;
	outBottom = GetParam(inParamOffset + 2, mTerminalHeight) - 1;
	outRight = GetParam(inParamOffset + 3, mTerminalWidth) - 1;

	if (mCursor.DECOM)
	{
		outTop += mMarginTop;
		outLeft += mMarginLeft;
		outBottom += mMarginTop;
		outRight += mMarginLeft;

		if (outTop < mMarginTop)
			outTop = mMarginTop;
		if (outLeft < mMarginLeft)
			outLeft = mMarginLeft;
		if (outBottom > mMarginBottom)
			outBottom = mMarginBottom;
		if (outRight > mMarginRight)
			outRight = mMarginRight;
	}
}

void MTerminalView::EscapeVT52(uint8_t inChar)
{
	mEscState = eESC_NONE;
	mArgs.clear();
	mState = 0;

	switch (inChar)
	{
		case '<':
			mDECANM = true;
			mDECSCL = 1;
			break;
		case 'A':
			MoveCursor(kMoveUp);
			break;
		case 'B':
			MoveCursor(kMoveDown);
			break;
		case 'C':
			MoveCursor(kMoveRight);
			break;
		case 'D':
			MoveCursor(kMoveLeft);
			break;
		case 'H':
			MoveCursorTo(0, 0);
			break;
		case 'Y':
			mEscState = eVT52_LINE;
			break;
		case 'I':
			MoveCursor(kMoveRI);
			break;
		case '=':
			mDECNMK = true;
			break;
		case '>':
			mDECNMK = false;
			break;
		case 'F':
			mCursor.CSGL = 1;
			break;
		case 'G':
			mCursor.CSGL = 0;
			break;
		case 'K':
			EraseInLine(0);
			break;
		case 'J':
			EraseInDisplay(0);
			break;
		case 'W':
		case 'X':
		case 'V':
		case ']':  /* ignore */
			break; // printing controls
		case 'Z':
			SendCommand("\033/Z");
			break;
	}
}

void MTerminalView::EscapeStart(uint8_t inChar)
{
	mEscState = eESC_NONE;
	mArgs.clear();
	mState = 0;

	switch (inChar)
	{
		// VT100 escape codes
		case '<':
			mDECANM = true;
			break;
		case '=':
			mDECNMK = true; /* DECKNAM */
			break;
		case '>':
			mDECNMK = false; /* DECKPNM */
			break;
		case 'D':
			MoveCursor(kMoveIND);
			break;
		case 'E':
			MoveCursor(kMoveCRLF);
			break;
		case 'H':
			SetTabstop();
			break;
		case 'M':
			MoveCursor(kMoveRI);
			break;
		case 'N':
			mCursor.SS = 2;
			break;
		case 'O':
			mCursor.SS = 3;
			break;
		case '7':
			SaveCursor();
			break;
		case '8':
			RestoreCursor();
			break;
		case '(':
			mArgs.push_back(0);
			mArgs.push_back(94);
			mEscState = eSELECT_CHAR_SET;
			break;
		case ')':
			mArgs.push_back(1);
			mArgs.push_back(94);
			mEscState = eSELECT_CHAR_SET;
			break;
		case '#':
			mEscState = eSELECT_DOUBLE;
			break;
		case '[':
			mEscState = eCSI;
			break;
		case 'Z':
			SendCommand(kVT420Attributes);
			break;
		case 'c':
			Reset();
			break;
		case '_':
			mEscState = eAPC;
			break;

		// VT220 escape codes
		case 'P':
			mPFKKeyNr = 0;
			mEscState = eDCS;
			break;
		case '*':
			mArgs.push_back(2);
			mArgs.push_back(94);
			mEscState = eSELECT_CHAR_SET;
			break;
		case '+':
			mArgs.push_back(3);
			mArgs.push_back(94);
			mEscState = eSELECT_CHAR_SET;
			break;
		case '~':
			mCursor.CSGR = 1;
			break;
		case 'n':
			mCursor.CSGL = 2;
			break;
		case '}':
			mCursor.CSGR = 2;
			break;
		case 'o':
			mCursor.CSGL = 3;
			break;
		case '|':
			mCursor.CSGR = 3;
			break;
		case ' ':
			mEscState = eSELECT_CTRL_TRANSM;
			break;

		// VT320 escape codes
		case '6':
			MoveCursor(kMoveBI);
			break;
		case '9':
			MoveCursor(kMoveFI);
			break;

		case '-':
			mArgs.push_back(1);
			mArgs.push_back(96);
			mEscState = eSELECT_CHAR_SET;
			break;
		case '.':
			mArgs.push_back(2);
			mArgs.push_back(96);
			mEscState = eSELECT_CHAR_SET;
			break;
		case '/':
			mArgs.push_back(3);
			mArgs.push_back(96);
			mEscState = eSELECT_CHAR_SET;
			break;

		// xterm support
		case ']':
			mEscState = eOSC;
			break;
		case '%':
			mArgs.push_back(4);
			mArgs.push_back(94);
			mEscState = eSELECT_CHAR_SET;
			break;

		case 'V':
			mCursor.style.SetFlag(kProtected);
			break;
		case 'W':
			mCursor.style.ClearFlag(kProtected);
			break;

		// unimplemented for now
		case 'l': /* Memory Lock */
			break;
		case 'm': /* Memory Unlock */
			break;
		case '^': /* privacy message */
			break;
		case 'X': /* Start of std::string */
			break;

		default: /* ignore */
			break;
	}
}

void MTerminalView::EscapeCSI(uint8_t inChar)
{
	if (mState == 0)
	{
#if DEBUG
		mCtrlSeq.clear();
#endif
		mCSICmd = 0;
		mArgs.clear();
		mArgs.push_back(0);
		mState = 1;
	}

#if DEBUG
	mCtrlSeq += inChar;
#endif

	if (inChar >= '0' and inChar <= '9')
		mArgs.back() = mArgs.back() * 10 + (inChar - '0');
	else if (inChar == ';')
		mArgs.push_back(0);
	else if (inChar >= ' ' and inChar <= '?')
		mCSICmd = mCSICmd << 8 | uint8_t(inChar);
	else if (not(inChar >= '0' and inChar <= '~'))
		mEscState = eESC_NONE; // error
	else
	{
		mEscState = eESC_NONE;
		mCSICmd = mCSICmd << 8 | uint8_t(inChar);

		// PRINT(("CSI: %s (%s)", mCtrlSeq.c_str(), Cmd2Name(mCSICmd)));

		MCSICmd cmd = static_cast<MCSICmd>(mCSICmd);

		if (mDECSCL > 1)
			ProcessCSILevel4(cmd);
		else
			ProcessCSILevel1(cmd);
	}
}

void MTerminalView::ProcessCSILevel1(uint32_t inCmd)
{
	uint32_t n = GetParam(0, 1);
	if (n == 0)
		n = 1;

	switch (inCmd)
	{
		// CBT -- Cursor Backward Tabulation
		case eCBT:
			while (n-- > 0)
				MoveCursor(kMoveCBT);
			break;
		// CHA -- Cursor Horizontal Absolute
		case eCHA:
			MoveCursorTo(GetParam(0, 1) - 1, mCursor.y);
			break;
		// CHT -- Cursor Horizontal Forward Tab
		case eCHT:
			while (n-- > 0)
				MoveCursor(kMoveHT);
			break;
		// CNL -- Cursor Next Line
		case eCNL:
			while (n-- > 0)
			{
				MoveCursor(kMoveCRLF);
			}
			break;
		// CPL -- Cursor Previous Line
		case eCPL:
			while (n-- > 0)
			{
				MoveCursor(kMoveCR);
				MoveCursor(kMoveUp);
			}
			break;
		// CUB -- Cursor Backward
		case eCUB:
			while (n-- > 0)
				MoveCursor(kMoveLeft);
			break;
		// CUD -- Cursor Down
		case eCUD:
			while (n-- > 0)
				MoveCursor(kMoveDown);
			break;
		// CUF -- Cursor Forward
		case eCUF:
			while (n-- > 0)
				MoveCursor(kMoveRight);
			break;
		// CUP -- Cursor Position
		case eCUP:
			MoveCursorTo(GetParam(1, 1) - 1, GetParam(0, 1) - 1);
			break;
		// CUU -- Cursor Up
		case eCUU:
			while (n-- > 0)
				MoveCursor(kMoveUp);
			break;
		// DA1 -- Response to Device Attributes
		case eDA1:
			SendCommand(kVT420Attributes);
			break;
		// DA2 -- Secondary Device Attributes
		case eDA2:
			SendCommand("\033[>64;278;1c"); /* compatible with xterm version 278? `*/
			break;
		// DA3 -- Tertiary Device Attributes
		case eDA3:
			SendCommand("\033P!|00000000\033\\");
			break;

		// DCH -- Delete Character
		case eDCH:
			if (not CursorIsInMargins())
				break;
			while (n-- > 0)
				mBuffer->DeleteCharacter(mCursor.y, mCursor.x, mMarginRight + 1);
			break;

		// DECCOLM -- Set columns per page
		case eDECCOLM:
		{
			int w = GetParam(0, 0);
			if (w == 0)
				w = 80;
			if (w > 10 and w < 1000) // sensible values please
				ResizeTerminal(w, mTerminalHeight, true);
			mDECVSSM = false;
			mMarginLeft = 0;
			mMarginRight = mTerminalWidth - 1;
			// TODO clear status line if host writable
			break;
		}

		// DL -- Delete Line(s)
		case eDL:
			if (not CursorIsInMargins())
				break;
			while (n-- > 0)
				mBuffer->ScrollForward(mCursor.y, mMarginBottom, mMarginLeft, mMarginRight);
			break;

		// DSR1 -- Device Status Report ANSI
		case eDSR1:
			switch (mArgs[0])
			{
				case 5:
					SendCommand("\033[0n");
					break; // terminal OK
				case 6:
					SendCommand(MFormat("\033[%d;%dR", mCursor.y + 1, mCursor.x + 1));
					break;
				case 15:
					SendCommand("\033[?13n");
					break; // we have no printer
				case 25:
					SendCommand(MFormat("\033[?2%dn", mPFK != nullptr and mPFK->locked));
					break;
				// TODO: Find out the keyboard layout
				case 26:
					SendCommand("\033[?27;0n");
					break; // report an unknown keyboard for now
			}
			break;
		// DSR2 -- Device Status Report DEC
		case eDSR2:
			switch (GetParam(0, 0))
			{
				case 6:
					SendCommand(MFormat("\033[?%d;%d;1R", mCursor.y + 1, mCursor.x + 1));
					break;
				case 15:
					SendCommand("\033[?11n");
					break;
				case 25:
					SendCommand(MFormat("\033[?2%dn", mPFK != nullptr and mPFK->locked));
					break;
				case 26:
					SendCommand("\033[?27;1n");
					break; // report a US keyboard for now
				case 53:
					SendCommand("\033[?50n");
					break; // No locator
			}
			break;
		// ED -- Erase in Display
		case eED:
			EraseInDisplay(GetParam(0, 0), false);
			break;
		// EL -- Erase in Line
		case eEL:
			EraseInLine(GetParam(0, 0), false);
			break;
		// HPA -- Horizontal Position Absolute
		case eHPA:
			MoveCursorTo(GetParam(0, 1) - 1, mCursor.y);
			break;
		// HPR -- Horizontal Position Relative
		case eHPR:
			while (n-- > 0)
				MoveCursor(kMoveRight);
			break;
		// HVP -- Horizontal/Vertical Position
		case eHVP:
			MoveCursorTo(GetParam(1, 1) - 1, GetParam(0, 1) - 1);
			break;
		// IL -- Insert Line
		case eIL:
			if (not CursorIsInMargins())
				break;
			while (n-- > 0)
				mBuffer->ScrollBackward(mCursor.y, mMarginBottom, mMarginLeft, mMarginRight);
			break;
			//	case eMC:
			//		break;
			// NP -- Next Page
		case eNP: /* unimplemented */
			break;
		// PP -- Preceding Page
		case ePP: /* unimplemented */
			break;
		// PPA -- Page Position Absolute
		case ePPA: /* unimplemented */
			break;
		// PPB -- Page Position Backwards
		case ePPB: /* unimplemented */
			break;
		// PPR -- Page Position Relative
		case ePPR: /* unimplemented */
			break;
		// SCORC -- Restore Saved Cursor Position (SCO Console)
		case eSCORC:
			RestoreCursor();
			break;
		// case eSCOSC:
		//	break;
		//  SD -- Pan Up
		case eSD:
			while (n-- > 0)
				ScrollBackward();
			break;
		// SGR -- Select Graphic Rendition
		case eSGR:
			for (size_t i = 0; i < mArgs.size(); ++i)
			{
				auto a = mArgs[i];

				switch (a)
				{
					case 0:
						mCursor.style = MStyle();
						mBuffer->SetColors(kXTermColorNone, kXTermColorNone);
						break;
					case 1:
						mCursor.style.SetFlag(kStyleBold);
						break;
					case 4:
						mCursor.style.SetFlag(kStyleUnderline);
						break;
					case 5:
						mCursor.style.SetFlag(kStyleBlink);
						break;
					case 7:
						mCursor.style.SetFlag(kStyleInverse);
						break;
					case 8:
						mCursor.style.SetFlag(kStyleInvisible);
						break;
					case 22:
						mCursor.style.ClearFlag(kStyleBold);
						break;
					case 24:
						mCursor.style.ClearFlag(kStyleUnderline);
						break;
					case 25:
						mCursor.style.ClearFlag(kStyleBlink);
						break;
					case 27:
						mCursor.style.ClearFlag(kStyleInverse);
						break;

					// vt300
					case 28:
						mCursor.style.ClearFlag(kStyleInvisible);
						break;

					// xterm colors
					case 30:
						mCursor.style.SetForeColor(kXTermColorRegularBack);
						break;
					case 31:
						mCursor.style.SetForeColor(kXTermColorRed);
						break;
					case 32:
						mCursor.style.SetForeColor(kXTermColorGreen);
						break;
					case 33:
						mCursor.style.SetForeColor(kXTermColorYellow);
						break;
					case 34:
						mCursor.style.SetForeColor(kXTermColorBlue);
						break;
					case 35:
						mCursor.style.SetForeColor(kXTermColorMagenta);
						break;
					case 36:
						mCursor.style.SetForeColor(kXTermColorCyan);
						break;
					case 37:
						mCursor.style.SetForeColor(kXTermColorNone);
						break;
					case 39:
						mCursor.style.SetForeColor(kXTermColorNone);
						break;

					case 40:
						mCursor.style.SetBackColor(kXTermColorNone);
						break;
					case 41:
						mCursor.style.SetBackColor(kXTermColorRed);
						break;
					case 42:
						mCursor.style.SetBackColor(kXTermColorGreen);
						break;
					case 43:
						mCursor.style.SetBackColor(kXTermColorYellow);
						break;
					case 44:
						mCursor.style.SetBackColor(kXTermColorBlue);
						break;
					case 45:
						mCursor.style.SetBackColor(kXTermColorMagenta);
						break;
					case 46:
						mCursor.style.SetBackColor(kXTermColorCyan);
						break;
					case 47:
						mCursor.style.SetBackColor(kXTermColorRegularText);
						break;
					case 49:
						mCursor.style.SetBackColor(kXTermColorNone);
						break;

					case 90:
						mCursor.style.SetForeColor(kXTermColorBrightBlack);
						break;
					case 91:
						mCursor.style.SetForeColor(kXTermColorBrightRed);
						break;
					case 92:
						mCursor.style.SetForeColor(kXTermColorBrightGreen);
						break;
					case 93:
						mCursor.style.SetForeColor(kXTermColorBrightYellow);
						break;
					case 94:
						mCursor.style.SetForeColor(kXTermColorBrightBlue);
						break;
					case 95:
						mCursor.style.SetForeColor(kXTermColorBrightMagenta);
						break;
					case 96:
						mCursor.style.SetForeColor(kXTermColorBrightCyan);
						break;
					case 97:
						mCursor.style.SetForeColor(kXTermColorBrightWhite);
						break;

					case 100:
						mCursor.style.SetBackColor(kXTermColorBrightBlack);
						break;
					case 101:
						mCursor.style.SetBackColor(kXTermColorBrightRed);
						break;
					case 102:
						mCursor.style.SetBackColor(kXTermColorBrightGreen);
						break;
					case 103:
						mCursor.style.SetBackColor(kXTermColorBrightYellow);
						break;
					case 104:
						mCursor.style.SetBackColor(kXTermColorBrightBlue);
						break;
					case 105:
						mCursor.style.SetBackColor(kXTermColorBrightMagenta);
						break;
					case 106:
						mCursor.style.SetBackColor(kXTermColorBrightCyan);
						break;
					case 107:
						mCursor.style.SetBackColor(kXTermColorBrightWhite);
						break;

					// color support
					case 38:
					case 48:
					{
						switch (mArgs[++i])
						{
							case 5:
							{
								uint8_t colorIndex = static_cast<uint8_t>(mArgs[++i]);
								if (a == 38)
									mCursor.style.SetForeColor((MXTermColor)colorIndex);
								else
									mCursor.style.SetBackColor((MXTermColor)colorIndex);
								break;
							}
						}

						break;
					}
				}

				if ((a >= 30 and a <= 49) or (a >= 90 and a <= 107))
					mBuffer->SetColors(mCursor.style.GetForeColor(), mCursor.style.GetBackColor());
			}
			break;
		// SU -- Pan Down
		case eSU:
			while (n-- > 0)
				ScrollForward();
			break;
		// TBC -- Clear Tabs
		case eTBC:
			switch (GetParam(0, 0))
			{
				case 0:
					if (mCursor.x < mTerminalWidth)
						mTabStops[mCursor.x] = false;
					break;
				case 3:
					fill(mTabStops.begin(), mTabStops.end(), false);
					break;
			}
			break;
		// VPA -- Vertical Line Position Absolute
		case eVPA:
			MoveCursorTo(mCursor.x, GetParam(0, 1) - 1);
			break;
		// VPR -- Vertical Position Relative
		case eVPR:
			MoveCursorTo(mCursor.x, mCursor.y + GetParam(0, 1));
			break;

		// SM_ANSI -- Set Mode ANSI
		case eSM_ANSI:
			for (uint32_t a : mArgs)
				SetResetMode(a, true, true);
			break;
		// RM_ANSI -- Reset Mode ANSI
		case eRM_ANSI:
			for (uint32_t a : mArgs)
				SetResetMode(a, true, false);
			break;
		// SM_DEC -- Set Mode DEC
		case eSM_DEC:
			for (uint32_t a : mArgs)
				SetResetMode(a, false, true);
			break;
		// RM_DEC -- Reset Mode DEC
		case eRM_DEC:
			for (uint32_t a : mArgs)
				SetResetMode(a, false, false);
			break;
		// SAVEMODE -- Save DEC Private Mode Values
		case eSAVEMODE:
			for (int a : mArgs)
				mSavedPrivateMode[a] = GetMode(a, false);
			break;
		// RESTMODE -- Restore DEC Private Mode Values
		case eRESTMODE:
			for (int a : mArgs)
				SetResetMode(a, false, mSavedPrivateMode[a]);
			break;
		// DECREQTPARM -- no comment
		case eDECREQTPARM:
			SendCommand((MFormat("\033[%d;1;1;128;128;1;0x", mArgs[0] + 2)));
			break;
		// XTERMEMK -- Reset XTerm modify keys
		case eXTERMEMK:
			//				switch (GetParam(0, 0))
			//				{
			//					case 1:	mModifyCursorKeys = GetParam(1, 0); break;
			//					case 1:	mModifyFunctionKeys = GetParam(1, 0); break;
			//					case 1:	mModifyOtherKeys = GetParam(1, 0); break;
			//				}
			break;
		// XTERMDMK -- Set XTerm modify keys
		case eXTERMDMK:
			//				switch (GetParam(0, 0))
			//				{
			//					case 1:	mModifyCursorKeys = -1; break;
			//					case 1:	mModifyFunctionKeys = -1; break;
			//					case 1:	mModifyOtherKeys = -1; break;
			//				}
			break;

		// REP -- Repeat preceding character
		case eREP:
			if (mLastChar != 0)
			{
				while (n-- > 0)
					WriteChar(mLastChar);
			}
			break;

		// SL -- Shift Left
		case eSL:
			while (n-- > 0)
				MoveCursor(kMoveSL);
			break;
		// SR -- Shift Right
		case eSR:
			while (n-- > 0)
				MoveCursor(kMoveSR);
			break;

		// DECSCL -- Select Conformance Level
		case eDECSCL:
			if (mArgs[0] >= 61 and mArgs[0] <= 65)
			{
				mDECSCL = mArgs[0] - 60;

				if (mDECSCL <= 1)
					mS8C1T = false;
				else
				{
					if (mArgs.size() == 1 or mArgs[1] == 0 or mArgs[1] == 2)
						mS8C1T = true;
					else if (mArgs[1] == 1)
						mS8C1T = false;
				}
			}
			break;

		case eDECELR:
			PRINT(("DECELR iets met de muis doen?"));
			// hmmmm
			break;

		default:
			PRINT(("Unhandled CSI level 1 command: >> %4.4s <<", &inCmd));
			break;
	}
}

void MTerminalView::ProcessCSILevel4(uint32_t inCmd)
{
	uint32_t n = GetParam(0, 1);
	if (n == 0)
		n = 1;

	switch (inCmd)
	{
		// ECH -- Erase character
		case eECH:
			mBuffer->EraseCharacter(mCursor.y, mCursor.x, n);
			break;

		// ICH -- Insert character
		case eICH:
			if (not CursorIsInMargins())
				break;
			while (n-- > 0)
				mBuffer->InsertCharacter(mCursor.y, mCursor.x, mMarginRight + 1);
			break;

		// DECDC -- Delete column
		case eDECDC:
			if (not CursorIsInMargins())
				break;
			while (n-- > 0)
			{
				for (int line = mMarginTop; line <= mMarginBottom; ++line)
					mBuffer->DeleteCharacter(line, mCursor.x, mMarginRight + 1);
			}
			break;

		// DECIC -- Insert column
		case eDECIC:
			if (not CursorIsInMargins())
				break;
			while (n-- > 0)
			{
				for (int line = mMarginTop; line <= mMarginBottom; ++line)
					mBuffer->InsertCharacter(line, mCursor.x, mMarginRight + 1);
			}
			break;

		// DECCARA -- Change attributes in rectangular area
		case eDECCARA:
		{
			int32_t t, l, b, r;
			GetRectParam(0, t, l, b, r);

			for (int a : mArgs)
			{
				mBuffer->ForeachInRectangle(t, l, b, r, [a](MChar &inChar, int32_t inLine, int32_t inColumn)
					{
					switch (a)
					{
						case 0:
							inChar = MStyle();
							break;
						case 1:
							inChar |= kStyleBold;
							break;
						case 4:
							inChar |= kStyleUnderline;
							break;
						case 5:
							inChar |= kStyleBlink;
							break;
						case 7:
							inChar |= kStyleInverse;
							break;
						case 21:
							inChar &= ~kStyleBold;
							break;
						case 24:
							inChar &= ~kStyleUnderline;
							break;
						case 25:
							inChar &= ~kStyleBlink;
							break;
						case 27:
							inChar &= ~kStyleInverse;
							break;
					} });
			}
			break;
		}

		// DECCRA -- Copy rectangular area
		case eDECCRA:
		{
			int ps = GetParam(4, 1);
			int pd = GetParam(7, 1);
			if (ps != 1 or pd != 1)
				break;

			int32_t t, l, b, r;
			GetRectParam(0, t, l, b, r);

			int32_t w = r - l, h = b - t;
			int32_t dt = GetParam(5, 1) - 1;
			if (mCursor.DECOM)
				dt += mMarginTop;
			int32_t dl = GetParam(6, 1) - 1;
			if (mCursor.DECOM)
				dl += mMarginLeft;

			if (dt + h > mTerminalHeight)
			{
				h = mTerminalHeight - dt;
				b = t + h;
			}

			if (dl + w > mTerminalWidth)
			{
				w = mTerminalWidth - dl;
				r = l + w;
			}

			if (w == 0 or h == 0)
				break;

			std::vector<MChar> buffer(mTerminalWidth * mTerminalHeight);
			uint32_t i = 0;
			mBuffer->ForeachInRectangle(t, l, b, r,
				[&i, &buffer](MChar &inChar, int32_t inLine, int32_t inColumn)
				{
					buffer[i++] = inChar;
				});

			assert(i <= buffer.size());
			i = 0;

			mBuffer->ForeachInRectangle(dt, dl, dt + h, dl + w,
				[&i, &buffer](MChar &inChar, int32_t inLine, int32_t inColumn)
				{
					inChar = buffer[i++];
				});
			break;
		}

		// DECELF -- Enable local functions
		case eDECELF: /* unimplemented */
			break;

		// DECERA -- Erase rectangular area
		case eDECERA:
		{
			int32_t t, l, b, r;
			GetRectParam(0, t, l, b, r);

			MChar ch(' ', mCursor.style);
			mBuffer->ForeachInRectangle(t, l, b, r,
				[ch](MChar &inChar, int32_t inLine, int32_t inColumn)
				{
					inChar = ch;
				});
			break;
		}
		// DECFRA -- Fill rectangular area
		case eDECFRA:
		{
			int32_t t, l, b, r;
			GetRectParam(1, t, l, b, r);

			MChar ch(GetParam(0, ' '), mCursor.style);
			mBuffer->ForeachInRectangle(t, l, b, r,
				[ch](MChar &inChar, int32_t inLine, int32_t inColumn)
				{
					inChar = ch;
				});
			break;
		}

		// DECINVM -- Invoke stored macro
		case eDECINVM: /* unimplemented */
			break;
		// DECLFKC -- Local function key control
		case eDECLFKC: /* unimplemented */
			break;
		// DECMSR -- Macro Space Report
		case eDECMSR: /* unimplemented */
			break;

		// DECRARA -- Reverse attributes in rectangular area
		case eDECRARA:
		{
			int32_t t, l, b, r;
			GetRectParam(0, t, l, b, r);

			for (uint32_t ai = 4; ai < mArgs.size(); ++ai)
			{
				uint32_t flag{};
				switch (mArgs[ai])
				{
					case 0:
						flag = kStyleBold | kStyleUnderline | kStyleBlink | kStyleInverse;
						break;
					case 1:
						flag = kStyleBold;
						break;
					case 4:
						flag = kStyleUnderline;
						break;
					case 5:
						flag = kStyleBlink;
						break;
					case 7:
						flag = kStyleInverse;
						break;
				}

				mBuffer->ForeachInRectangle(t, l, b, r,
					[flag](MChar &inChar, int32_t inLine, int32_t inColumn)
					{
						inChar.ReverseFlag(MCharStyle(flag));
					});
			}
			break;
		}

		// DECRPM -- Report mode
		case eDECRPM: /* unimplemented */
			break;
		// DECRQCRA -- Request checksum of rectangular area
		case eDECRQCRA: /* unimplemented */
			break;
		// DECRQDE -- Request displayed extent
		case eDECRQDE:
			SendCommand(MFormat("\033[%d;%d;1;1;1\"w", mTerminalHeight, mTerminalWidth));
			break;

		// DECRQMANSI -- Request mode ANSI
		case eDECRQMANSI:
		{
			int p = GetParam(0, 0);
			SendCommand(MFormat("\033[%d;%d$y", p, GetMode(p, true) ? 1 : 2));
			break;
		}
		// DECRQMDEC -- Request mode DEC Private
		case eDECRQMDEC:
		{
			int p = GetParam(0, 0);
			SendCommand(MFormat("\033[?%d;%d$y", p, GetMode(p, false) ? 1 : 2));
			break;
		}
		// DECRQPSR -- Request presentation state
		case eDECRQPSR:
			switch (GetParam(0, 0))
			{
				case 1: /* DECCIR */
				{
					MStyle st = mCursor.style;
					SendCommand(MFormat("\033P1$u%d;%d;%d;%c;%c;%c;%d;%d;%c;%c%c%c%c\033\\",
						mCursor.y + 1,
						mCursor.x + 1,
						1,
						char(0x40 + (st & kStyleInverse ? 8 : 0) + (st & kStyleBlink ? 4 : 0) + (st & kStyleUnderline ? 2 : 0) + (st & kStyleBold ? 1 : 0)),
						char(0x40 + (st & kUnerasable ? 1 : 0)),
						char(0x40 + (mCursor.DECAWM ? 8 : 0) + (mCursor.SS == 3 ? 4 : 0) + (mCursor.SS == 2 ? 2 : 0) + (mCursor.DECOM ? 1 : 0)),
						mCursor.CSGL,
						mCursor.CSGR,
						char(0x5f), // pffft, not sure about this one... FIXME
						mCursor.charSetGSel[0],
						mCursor.charSetGSel[1],
						mCursor.charSetGSel[2],
						mCursor.charSetGSel[3]));
					break;
				}
				case 2: /* DECTABSR */
				{
					std::vector<std::string> ts;
					for (int i = 0; i < mTerminalWidth; ++i)
						if (mTabStops[i])
							ts.push_back(std::to_string(i + 1));
					SendCommand(kDCS + "2$u" + zeep::join(ts, "/") + "\033\\");
					break;
				}
			}
			break;
		// DECRQUPSS -- Request User-Preferred Supplemental Set
		case eDECRQUPSS:
			SendCommand("\033P0!u%5\033\\");
			break;
		// DECSACE -- Select attribute change extent
		case eDECSACE:
			mDECSACE = (GetParam(0, 1) == 2);
			break;
		// DECSASD -- Select active status display
		case eDECSASD:
			if (GetParam(0, 0))
			{
				if (not mDECSASD)
				{
					mSavedSL = mCursor;
					ResetCursor();
					mDECSASD = true;
				}
			}
			else
			{
				if (mDECSASD)
				{
					mCursor = mSavedSL;
					mDECSASD = false;
				}
			}
			break;
		// DECSCA -- Select character attribute
		case eDECSCA:
			switch (GetParam(0, 0))
			{
				case 0:
				case 2:
					mCursor.style.ClearFlag(kUnerasable);
					break;
				case 1:
					mCursor.style.SetFlag(kUnerasable);
					break;
			}
			break;

		// DECSED -- Selective erase in display
		case eDECSED:
			EraseInDisplay(GetParam(0, 0), true);
			break;
			break;
		// DECSEL -- Selective erase in line
		case eDECSEL:
			EraseInLine(GetParam(0, 0), true);
			break;
		// DECSERA -- Selective erase rectangular area
		case eDECSERA:
		{
			int32_t t, l, b, r;
			GetRectParam(0, t, l, b, r);

			mBuffer->ForeachInRectangle(t, l, b, r,
				[](MChar &inChar, int32_t inLine, int32_t inColumn)
				{
					if (not(inChar & kUnerasable))
						inChar = ' ';
				});
			break;
		}
		// DECSLPP -- Set Lines Per Page
		case eDECSLPP:
		{
			MRect r;

			switch (GetParam(0, 0))
			{
				case 11:
					SendCommand("\033[1t");
					break;
				case 13:
					GetWindow()->GetWindowPosition(r);
					SendCommand(MFormat("\033[3;%d;%dt", r.x, r.y));
					break;
				case 14:
					GetWindow()->GetBounds(r);
					SendCommand(MFormat("\033[4;%d;%dt", r.width, r.height));
					break;
				case 18:
					SendCommand(MFormat("\033[8;%d;%dt", mTerminalWidth, mTerminalHeight));
					break;
				case 20:
					SendCommand("\033]L\033\\");
					break;
				case 21:
					SendCommand(std::string("\033]l") + GetWindow()->GetTitle() + "\033\\");
					break;
				default:
				{
					int h = GetParam(1, 24);
					if (h >= 4 and h < 240)
						ResizeTerminal(mTerminalWidth, h, true);
					break;
				}
			}
			break;
		}
		// DECSLRM -- Set left and right margins
		case eDECSLRM:
			//			if (mArgs.size() == 1 and mArgs[0] == 0)
			//				SaveCursor();
			//			else
			if (mDECVSSM)
			{
				int32_t left = GetParam(0, 1);
				if (left < 1)
					left = 1;
				int32_t right = GetParam(1, mTerminalWidth);
				if (right > mTerminalWidth)
					right = mTerminalWidth;
				if (right < left + 1)
					right = left + 1;

				mMarginLeft = left - 1;
				mMarginRight = right - 1;
				MoveCursorTo(0, 0);
			}
			break;
		// DECSMKR -- Select modifier key reporting
		case eDECSMKR: /* unimplemented */
			break;
		// DECSNLS -- Select number of lines per screen
		case eDECSNLS:
			if (n >= 24 and n < 250)
				ResizeTerminal(mTerminalWidth, n, true);
			break;
		// DECSR -- Secure reset
		case eDECSR: /* unimplemented */
			break;
		// DECSSDT -- Select status display type
		case eDECSSDT:
			mDECSSDT = GetParam(0, 1);
			ResizeTerminal(mTerminalWidth, mTerminalHeight, true);
			break;
		// DECSTBM -- Set Top and Bottom Margin
		case eDECSTBM:
		{
			int32_t top = GetParam(0, 1);
			if (top < 1)
				top = 1;
			int32_t bottom = GetParam(1, mTerminalHeight);
			if (bottom > mTerminalHeight)
				bottom = mTerminalHeight;
			if (bottom < top + 1)
				bottom = top + 1;

			mMarginTop = top - 1;
			mMarginBottom = bottom - 1;

			MoveCursorTo(0, 0);
			break;
		}
		// DECSTR -- Soft terminal reset
		case eDECSTR:
			SoftReset();
			break;

		/*
			XTerm / DEC VT520
		*/

		// DECSCUSR -- Set Cursor Style
		case eDECSCUSR:
			switch (GetParam(0, 1))
			{
				case 1:
					mCursor.block = true;
					mCursor.blink = true;
					break;
				case 2:
					mCursor.block = true;
					mCursor.blink = false;
					break;
				case 3:
					mCursor.block = false;
					mCursor.blink = true;
					break;
				case 4:
					mCursor.block = false;
					mCursor.blink = false;
					break;
			}
			Invalidate();
			break;
		// DECSWBV -- Set Warning Bell Volume
		case eDECSWBV: /* unimplemented */
			break;
		// DECSMBV -- Set Margin Bell Volume
		case eDECSMBV: /* unimplemented */
			break;

		default:
			ProcessCSILevel1(inCmd);
			break;
	}
}

void MTerminalView::SelectCharSet(uint8_t inChar)
{
	mEscState = eESC_NONE;

	int charset = GetParam(0, 0);
	int charcount = GetParam(1, 0);

	if (charset == 4)
	{
		switch (inChar)
		{
			case '@':
				mEncoding = kEncodingISO88591;
				break;
			case 'G':
				mEncoding = kEncodingUTF8;
				break;
		}
	}
	else
	{
		if (mDECSCL == 1 and charset > 1)
		{
			PRINT(("Unsupported set G%d", charset));
			return;
		}

		mCursor.charSetGSel[charset] = inChar;

		if (charcount == 96)
		{
			switch (inChar)
			{
				case 'A':
					mCursor.charSetG[charset] = kISOLatin1Supplemental;
					break;
			}
		}
		else
		{
			switch (inChar)
			{
				case 'A':
					mCursor.charSetG[charset] = kUKCharSet;
					break;
				case 'B':
					mCursor.charSetG[charset] = kUSCharSet;
					break;
				case '4':
					mCursor.charSetG[charset] = kNLCharSet;
					break;
				case 'C':
				case '5':
					mCursor.charSetG[charset] = kFICharSet;
					break;
				case 'R':
					mCursor.charSetG[charset] = kFRCharSet;
					break;
				case 'Q':
					mCursor.charSetG[charset] = kCACharSet;
					break;
				case 'K':
					mCursor.charSetG[charset] = kDECharSet;
					break;
				case 'Y':
					mCursor.charSetG[charset] = kITCharSet;
					break;
				case 'E':
				case '6':
					mCursor.charSetG[charset] = kDKCharSet;
					break;
				case 'Z':
					mCursor.charSetG[charset] = kSPCharSet;
					break;
				case 'H':
				case '7':
					mCursor.charSetG[charset] = kSECharSet;
					break;
				case '=':
					mCursor.charSetG[charset] = kCHCharSet;
					break;
				case '0':
					mCursor.charSetG[charset] = kLineCharSet;
					break;
				case '1':
					mCursor.charSetG[charset] = kUSCharSet;
					break;
				case '2':
					mCursor.charSetG[charset] = kLineCharSet;
					break;
			}
		}
	}
}

void MTerminalView::SelectControlTransmission(uint8_t inCode)
{
	switch (inCode)
	{
		case 'F':
			mS8C1T = false;
			break;
		case 'G':
			if (mDECSCL >= 2)
				mS8C1T = true;
			break;

		// unimplemented
		case 'N': /* ANSI conformance level 3 */
			mCursor.charSetG[0] = kUSCharSet;
			mCursor.CSGL = 0;
			// fall through
		case 'L': /* ANSI conformance level 1 */
		case 'M': /* ANSI conformance level 2 */
			mCursor.charSetG[1] = kISOLatin1Supplemental;
			mCursor.CSGR = 1;
			break;
	}

	mEscState = eESC_NONE;
}

void MTerminalView::SelectDouble(uint8_t inDoubleMode)
{
	mEscState = eESC_NONE;

	switch (inDoubleMode)
	{
		case '3':
			mBuffer->SetLineDoubleHeight(mCursor.y, true);
			break;
		case '4':
			mBuffer->SetLineDoubleHeight(mCursor.y, false);
			break;
		case '5':
			mBuffer->SetLineSingleWidth(mCursor.y);
			break;
		case '6':
			mBuffer->SetLineDoubleWidth(mCursor.y);
			break;
		case '8':
			mBuffer->FillWithE();
			break;
	}
}

void MTerminalView::CommitPFK()
{
	for (auto k : mNewPFK->key)
	{
		std::string s;
		for (uint32_t i = 0; i < k.second.length(); i += 2)
		{
			char c1 = tolower(k.second[i]);
			char c2 = 0;
			if (i < k.second.length())
				c2 = tolower(k.second[i + 1]);

			if (c1 >= '0' and c1 <= '9')
				c1 -= '0';
			else
				c1 -= 'a' + 10;

			if (c2 >= '0' and c2 <= '9')
				c2 -= '0';
			else
				c2 -= 'a' + 10;

			s += char((c1 << 4) | c2);
		}
		mNewPFK->key[k.first] = s;
	}

	if (mNewPFK->clear or mPFK == nullptr)
	{
		delete mPFK;
		mPFK = mNewPFK;
	}
	else
	{
		for (auto k : mNewPFK->key)
			mPFK->key[k.first] = k.second;
		delete mNewPFK;
	}

	mNewPFK = nullptr;
}

void MTerminalView::EscapeDCS(uint8_t inChar)
{
	if (inChar == ST)
	{
		if (mNewPFK)
			CommitPFK();
		else if (not mDECRQSS.empty() and mDECSCL >= 4)
		{
			std::string response("\033P0$r");

			if (mDECRQSS == "m") // SGR - Set Graphic Rendition
			{
				std::vector<std::string> sgr;
				if (mCursor.style & kStyleBold)
					sgr.push_back("1");
				if (mCursor.style & kStyleUnderline)
					sgr.push_back("4");
				if (mCursor.style & kStyleBlink)
					sgr.push_back("5");
				if (mCursor.style & kStyleInverse)
					sgr.push_back("7");
				if (mCursor.style & kStyleInvisible)
					sgr.push_back("8");
				if (mCursor.style.GetForeColor() != kXTermColorNone)
					sgr.push_back(std::to_string(30 + mCursor.style.GetForeColor()));
				if (mCursor.style.GetBackColor() != kXTermColorNone)
					sgr.push_back(std::to_string(40 + mCursor.style.GetBackColor()));

				response = MFormat("\033P1$r%s", zeep::join(sgr, ";").c_str());
			}
			//			else if (mDECRQSS == ",|")	// DECAC - Assign Color
			//			else if (mDECRQSS == ",}")	// DECATC - Alternate Text Color
			else if (mDECRQSS == "$}") // DECSASD - Select Active Status Display
				response = "\033P1$r0}";
			else if (mDECRQSS == "*x") // DECSACE - Select Attribute Change Extent
				response = MFormat("\033P1$r%d", mDECSACE ? 2 : 1);
			else if (mDECRQSS == "\"q") // DECSCA - Set Character Attribute
				response = mCursor.style & kUnerasable ? "\033P1$r1" : "\033P1$r0";
			else if (mDECRQSS == "$|") // DECSCPP - Set Columns Per Page
				response = MFormat("\033P1$r%d", mTerminalWidth);
			//			else if (mDECRQSS == "*r")	// DECSCS - Select Communication Speed
			//			else if (mDECRQSS == "*u")	// DECSCP - Select Communication Port
			else if (mDECRQSS == "\"p") // DECSCL - Set Conformance Level
			{
				if (mDECSCL >= 2)
					response = MFormat("\033P1$r%d;%d", mDECSCL, mS8C1T ? 0 : 1);
				else
					response = "\033P1$r61";
			}
			else if (mDECRQSS == " q") // DECSCUSR - Set Cursor Style
			{
				int cs = mCursor.block ? (mCursor.blink ? 1 : 2) : (mCursor.blink ? 3 : 4);
				response = MFormat("\033P1$r%d", cs);
			}
			//			else if (mDECRQSS == ")p")	// DECSDPT - Select Digital Printed Data Type
			//			else if (mDECRQSS == "$q")	// DECSDDT - Select Disconnect Delay Time
			//			else if (mDECRQSS == "*s")	// DECSFC - Select Flow Control Type
			//			else if (mDECRQSS == " r")	// DECSKCV - Set Key Click Volume
			else if (mDECRQSS == "s") // DECSLRM - Set Left and Right Margins
				response = MFormat("\033P1$r%d;%d", mMarginLeft + 1, mMarginRight + 1);
			else if (mDECRQSS == "t") // DECSLPP - Set Lines Per Page
				response = MFormat("\033P1$r%d", mTerminalHeight);
			//			else if (mDECRQSS == " v")	// DECSLCK - Set Lock Key Style
			//			else if (mDECRQSS == " u")	// DECSMBV - Set Margin Bell Volume
			else if (mDECRQSS == "*|") // DECSNLS - Set Number of Lines per Screen
				response = MFormat("\033P1$r%d", mTerminalHeight);
			//			else if (mDECRQSS == ",x")	// DECSPMA - Session Page Memory Allocation
			//			else if (mDECRQSS == "+w")	// DECSPP - Set Port Parameter
			//			else if (mDECRQSS == "$s")	// DECSPRTT - Select Printer Type
			//			else if (mDECRQSS == "*p")	// DECSPPCS - Select ProPrinter Character Set
			//			else if (mDECRQSS == " p")	// DECSSCLS - Set Scroll Speed
			//			else if (mDECRQSS == "p")	// DECSSL - Select Set-Up Language
			else if (mDECRQSS == "$~") // DECSSDT - Set Status Line Type
				response = MFormat("\033P1$r%d", mDECSSDT);
			else if (mDECRQSS == "r") // DECSTBM - Set Top and Bottom Margins
				response = MFormat("\033P1$r%d;%d", mMarginTop + 1, mMarginBottom + 1);
			//			else if (mDECRQSS == "\"u")	// DECSTRL - Set Transmit Rate Limit
			//			else if (mDECRQSS == " t")	// DECSWBV - Set Warning Bell Volume
			//			else if (mDECRQSS == ",{")	// DECSZS - Select Zero Symbol

			SendCommand(response + mDECRQSS + "\033\\");
			mDECRQSS.clear();
		}
		return;
	}

	switch (mState)
	{
		case 0:
			switch (inChar)
			{
				case '0':
					delete mNewPFK;
					mNewPFK = new MPFK;
					mNewPFK->clear = mNewPFK->locked = true;
					mState = 10;
					break;

				case '1':
					delete mNewPFK;
					mNewPFK = new MPFK;
					mNewPFK->clear = false;
					mNewPFK->locked = true;
					mState = 10;
					break;

				case ';':
					delete mNewPFK;
					mNewPFK = new MPFK;
					mNewPFK->clear = mNewPFK->locked = true;
					mState = 11;
					break;

				case '$':
					mState = 20;
					break;
				case '+':
					mState = 30;
					break;
				default:
					mState = 100; /* unrecognized */
					break;
			}
			break;

			// ';'

		case 10:
			if (inChar == ';')
				mState = 11;
			else
				mState = 100;
			break;

		case 11:
			switch (inChar)
			{
				case '0':
					mState = 12;
					break;
				case '1':
					mNewPFK->locked = false;
					mState = 12;
					break;
				case '|':
					mState = 13;
					break;
				default:
					mState = 100;
					break;
			}
			break;

		case 12:
			if (inChar == '|')
				mState = 13;
			else
				mState = 100;
			break;

		case 13:
			if (inChar >= '0' and inChar <= '9')
				mPFKKeyNr = mPFKKeyNr * 10 + (inChar - '0');
			else if (inChar == '/')
				mState = 14;
			else
				mState = 100;
			break;

		case 14:
			if (inChar == ';')
				mPFKKeyNr = 0;
			else if (isxdigit(inChar))
			{
				uint32_t keyCode;
				switch (mPFKKeyNr)
				{
					case eF1:
						keyCode = kF1KeyCode;
						break;
					case eF2:
						keyCode = kF2KeyCode;
						break;
					case eF3:
						keyCode = kF3KeyCode;
						break;
					case eF4:
						keyCode = kF4KeyCode;
						break;
					case eF5:
						keyCode = kF5KeyCode;
						break;
					case eF6:
						keyCode = kF6KeyCode;
						break;
					case eF7:
						keyCode = kF7KeyCode;
						break;
					case eF8:
						keyCode = kF8KeyCode;
						break;
					case eF9:
						keyCode = kF9KeyCode;
						break;
					case eF10:
						keyCode = kF10KeyCode;
						break;
					case eF11:
						keyCode = kF11KeyCode;
						break;
					case eF12:
						keyCode = kF12KeyCode;
						break;
					case eF13:
						keyCode = kF13KeyCode;
						break;
					case eF14:
						keyCode = kF14KeyCode;
						break;
					case eF15:
						keyCode = kF15KeyCode;
						break;
					case eF16:
						keyCode = kF16KeyCode;
						break;
					case eF17:
						keyCode = kF17KeyCode;
						break;
					case eF18:
						keyCode = kF18KeyCode;
						break;
					case eF19:
						keyCode = kF19KeyCode;
						break;
					case eF20:
						keyCode = kF20KeyCode;
						break;
				}
				mNewPFK->key[keyCode] += inChar;
			}
			else
				mState = 100;
			break;

			// $q DECSRQSS

		case 20:
			if (inChar == 'q')
				mState = 21;
			else
				mState = 100;
			break;

		case 21:
			mDECRQSS = inChar;
			mState = 22;
			break;

		case 22:
			mDECRQSS += inChar;
			break;
	}

	if (mEscState == eESC_NONE)
	{
		delete mNewPFK;
		mNewPFK = nullptr;
	}
}

void MTerminalView::EscapeOSC(uint8_t inChar)
{
	if (inChar == BEL or inChar == ST)
	{
		// done
		mEscState = eESC_NONE;

		switch (mArgs[0])
		{
			case 0:
			case 1:
			case 2:
				mSetWindowTitle = mArgString;
				for (char &ch : mSetWindowTitle)
				{
					if (std::iscntrl(ch))
						ch = '_';
				}
				break;

			case 10:
				if (mArgString == "?")
				{
					std::string textColor = mTerminalColors[eText].hex();

					SendCommand(
						"\033]11;rgb:" +
						textColor.substr(1, 2) + textColor.substr(1, 2) + '/' +
						textColor.substr(3, 2) + textColor.substr(3, 2) + '/' +
						textColor.substr(5, 2) + textColor.substr(5, 2) +
						"\033\\");

					// PRINT(("Request for text colour"));
				}
				break;

			case 11:
				if (mArgString == "?")
				{
					std::string backColor = mTerminalColors[eBack].hex();

					SendCommand(
						"\033]11;rgb:" +
						backColor.substr(1, 2) + backColor.substr(1, 2) + '/' +
						backColor.substr(3, 2) + backColor.substr(3, 2) + '/' +
						backColor.substr(5, 2) + backColor.substr(5, 2) +
						"\033\\");

					// PRINT(("Request for background colour"));
				}
				break;

				/* unimplemented: veel */

			case 52:
				if (mArgString.length() > 2 and mArgString[1] == ';' and mArgString[0] == 'c')
				{
					if (mArgString[2] == '?')
						SendCommand("\033]52;c;\033\\"); // empty std::string as reply, sorry
					else
					{
						auto s = zeep::decode_base64({ mArgString.data() + 2, mArgString.length() - 2 });
						MClipboard::Instance().SetData(s, false);
					}
				}
				break;

			default:
				PRINT(("Ignored %d OSC option", mArgs[0]));
				break;
		}
	}
	else
	{
		switch (mState)
		{
			case 0: // start, expect a number, or ';'
				mArgs.clear();
				mArgs.push_back(0);
				mArgString.clear();

				if (inChar == ';')
				{
					mArgs.push_back(0);
					mState = 2;
				}
				else if (inChar >= '0' and inChar <= '9')
				{
					mArgs[0] = inChar - '0';
					mState = 1;
				}
				else
				{
					mArgString += inChar;
					mState = 2;
				}
				break;

			case 1:
				if (inChar >= '0' and inChar <= '9')
					mArgs.back() = mArgs.back() * 10 + (inChar - '0');
				else if (inChar == ';')
					mArgs.push_back(0);
				else // error
				{
					mArgString += inChar;
					mState = 2;
				}
				break;

			case 2:
				mArgString += inChar;
				break;
		}
	}
}

void MTerminalView::EscapeAPC(uint8_t inChar)
{
	if (inChar == BEL or inChar == ST)
	{
		// done
		mEscState = eESC_NONE;

		switch (mArgs[0])
		{
			case 7:
			{
				// Bash function for get is:
				// get() { file="$1"; printf "\033_7;%s\x9c" $(realpath -qez "$file" | base64 -w0);}

				auto s = zeep::decode_base64({ mArgString.data(), mArgString.length() });

				DownloadFile(s);
				break;
			}

			case 8:
			{
				// Bash function for get is:
				// put() { file="$1"; printf "\033_8;%s\x9c" $(realpath -qz "$file" | base64 -w0);}

				auto s = zeep::decode_base64({ mArgString.data(), mArgString.length() });

				UploadFile(s);
				break;
			}

			default:
				PRINT(("Ignored %d APC option", mArgs[0]));
				break;
		}
	}
	else
	{
		switch (mState)
		{
			case 0: // start, expect a number, or ';'
				mArgs.clear();
				mArgs.push_back(0);
				mArgString.clear();

				if (inChar == ';')
				{
					mArgs.push_back(0);
					mState = 2;
				}
				else if (inChar >= '0' and inChar <= '9')
				{
					mArgs[0] = inChar - '0';
					mState = 1;
				}
				else
				{
					mArgString += inChar;
					mState = 2;
				}
				break;

			case 1:
				if (inChar >= '0' and inChar <= '9')
					mArgs.back() = mArgs.back() * 10 + (inChar - '0');
				else if (inChar == ';')
					mArgs.push_back(0);
				else // error
				{
					mArgString += inChar;
					mState = 2;
				}
				break;

			case 2:
				mArgString += inChar;
				break;
		}
	}
}

void MTerminalView::SaveCursor(void)
{
	if (mBuffer == &mAlternateBuffer)
	{
		mAlternate = mCursor;
		mAlternate.saved = true;
	}
	else
	{
		mSaved = mCursor;
		mSaved.saved = true;
	}
}

void MTerminalView::RestoreCursor(void)
{
	if (mBuffer == &mAlternateBuffer and mAlternate.saved)
		mCursor = mAlternate;
	else if (mBuffer == &mScreenBuffer and mSaved.saved)
		mCursor = mSaved;
	else
	{
		MoveCursorTo(0, 0);

		mCursor.charSetG[0] = kUSCharSet;
		mCursor.charSetG[1] = kLineCharSet;
		mCursor.charSetG[2] = kUSCharSet;
		mCursor.charSetG[3] = kLineCharSet;
		mCursor.CSGL = 0;
		mCursor.CSGR = 2;
		mCursor.style = MStyle();
		mCursor.DECOM = false;
	}
}

void MTerminalView::SwitchToAlternateScreen()
{
	if (mBuffer != &mAlternateBuffer)
	{
		mBuffer = &mAlternateBuffer;
		EraseInDisplay(2);
		AdjustScrollbar(0);
		Invalidate();
	}
}

void MTerminalView::SwitchToRegularScreen()
{
	if (mBuffer != &mScreenBuffer)
	{
		mBuffer = &mScreenBuffer;
		AdjustScrollbar(0);
		Invalidate();
	}
}

void MTerminalView::Animate()
{
	Invalidate();
}

void MTerminalView::Beep()
{
	using namespace std::chrono_literals;

	auto now = std::chrono::system_clock::now();
	bool beeped = false;

	if (mGraphicalBeep and now - mLastBeep > 250ms)
	{
		if (mAnimationManager->Update())
			PRINT(("duh"));

		MStoryboard *storyboard = mAnimationManager->CreateStoryboard();
		storyboard->AddTransition(mGraphicalBeep, 0.75, 75ms, "acceleration-decelleration");
		storyboard->AddTransition(mGraphicalBeep, 0.00, 75ms, "acceleration-decelleration");
		mAnimationManager->Schedule(storyboard);

		beeped = true;
	}

	if (mAudibleBeep and now - mLastBeep > 500ms)
	{
		PlaySound("warning");
		beeped = true;
	}

	if (beeped)
		mLastBeep = now;
}

void MTerminalView::SetResetMode(uint32_t inMode, bool inANSI, bool inSet)
{
	if (inANSI)
	{
		switch (inMode)
		{
			case 2:
				mKAM = inSet;
				break;
			case 4:
				mIRM = inSet;
				break;
			case 12:
				mSRM = inSet;
				break;
			case 20:
				mLNM = inSet;
				break;
			default:
				PRINT(("Ignored %s of option %d", inSet ? "set" : "reset", inMode));
				break;
		}
	}
	else
	{
		switch (inMode)
		{
			case 1:
				mDECCKM = inSet;
				break;
			case 2:
				mDECANM = inSet;
				break;
			case 3:           // DECCOLM
				mDECSSDT = 0; // reset status line conforming to specification
				ResizeTerminal(inSet ? 132 : 80, mTerminalHeight, true);
				break;
			case 4:
				mDECSCLM = inSet;
				break;
			case 5:
				mDECSCNM = inSet;
				break;
			case 6:
				mCursor.DECOM = inSet;
				if (inSet)
					MoveCursorTo(0, 0);
				break;
			case 7:
				mCursor.DECAWM = inSet;
				break;
			case 8:
				mDECARM = inSet;
				break;
			case 12:
				mCursor.blink = inSet;
				break;
			case 18:
				mDECPFF = inSet;
				break;
			case 19:
				mDECPEX = inSet;
				break;
			case 25:
				mDECTCEM = inSet;
				break;
			case 42:
				mDECNRCM = inSet;
				break;
			case 66:
				mDECNMK = inSet;
				break;
			case 67:
				mDECBKM = inSet;
				break;

			case 69:
				mDECVSSM = inSet;
				if (not mDECVSSM)
				{
					mMarginLeft = 0;
					mMarginRight = mTerminalWidth - 1;
				}
				break;

			case 9:
			case 1000:
			case 1001:
			case 1002:
			case 1003:
				PRINT(("%s mouse mode for %d", inSet ? "set" : "reset", inMode));
				if (inSet)
					mMouseMode = (MouseTrackingMode)inMode;
				else
					mMouseMode = eTrackMouseNone;
				break;

			case 47: // alternate screen buffer support
			case 1047:
				if (inSet)
					SwitchToAlternateScreen();
				else
					SwitchToRegularScreen();
				break;

			case 1048:
				if (inSet)
					SaveCursor();
				else
					RestoreCursor();
				break;

			case 1049:
				if (inSet)
				{
					SaveCursor();
					SwitchToAlternateScreen();
				}
				else
				{
					SwitchToRegularScreen();
					RestoreCursor();
				}
				break;

			case 2004:
				mBracketedPaste = inSet;
				break;

			default:
				PRINT(("Ignored %s of option %d", inSet ? "set" : "reset", inMode));
				break;
		}
	}
}

bool MTerminalView::GetMode(uint32_t inMode, bool inANSI)
{
	bool result = false;

	if (inANSI)
	{
		switch (inMode)
		{
			case 2:
				result = mKAM;
				break;
			case 4:
				result = mIRM;
				break;
			case 12:
				result = mSRM;
				break;
			case 20:
				result = mLNM;
				break;
		}
	}
	else
	{
		switch (inMode)
		{
			case 1:
				result = mDECCKM;
				break;
			case 2:
				result = mDECANM;
				break;
			case 3:
				result = mTerminalWidth == 132;
				break;
			case 4:
				result = mDECSCLM;
				break;
			case 5:
				result = mDECSCNM;
				break;
			case 6:
				result = mCursor.DECOM;
				break;
			case 7:
				result = mCursor.DECAWM;
				break;
			case 8:
				result = mDECARM;
				break;
			case 12:
				result = mCursor.blink;
				break;
			case 18:
				result = mDECPFF;
				break;
			case 19:
				result = mDECPEX;
				break;
			case 25:
				result = mDECTCEM;
				break;
			case 42:
				result = mDECNRCM;
				break;
			case 66:
				result = mDECNMK;
				break;
			case 67:
				result = mDECBKM;
				break;
			case 69:
				result = mDECVSSM;
				break;
			case 2004:
				result = mBracketedPaste;
				break;
		}
	}

	return result;
}

// --------------------------------------------------------------------

void MTerminalView::DownloadFile(const std::string &path)
{
	if (mTerminalChannel->CanDownloadFiles())
	{
		MFileDialogs::SaveFileAs(GetWindow(), path, [path, channel = mTerminalChannel](std::filesystem::path file)
			{ channel->DownloadFile(path, file); });
	}
}

void MTerminalView::UploadFile(const std::string &path)
{
	if (mTerminalChannel->CanDownloadFiles())
	{
		MFileDialogs::ChooseOneFile(GetWindow(), [path, channel = mTerminalChannel](std::filesystem::path file)
			{ channel->UploadFile(path, file); });
	}
}
