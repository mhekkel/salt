// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include <deque>
#include <list>
#include <map>

#include <boost/format.hpp>

#include <pinch/terminal_channel.hpp>

#include "MCanvas.hpp"
#include "MColor.hpp"
#include "MHandler.hpp"
#include "MP2PEvents.hpp"
#include "MSearchPanel.hpp"
#include "MTerminalBuffer.hpp"
#include "MTerminalChannel.hpp"
#include "MUnicode.hpp"

class MStatusbar;
class MScrollbar;
class MAnimationVariable;
class MAnimationManager;

class MTerminalView : public MCanvas
{
  public:
	MTerminalView(const std::string &inID, MRect inBounds, MStatusbar *inStatusbar, MScrollbar *inScrollbar,
	              MSearchPanel *inSearchPanel, MTerminalChannel *inTerminalChannel, const std::string &inSSHCommand);
	virtual ~MTerminalView();

	static MTerminalView *
	GetFrontTerminal();

	MEventIn<void(MScrollMessage)> eScroll;

	// What lines are visible:
	int32_t GetTopLine() const;

	virtual void Draw();
	virtual void MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers);
	virtual void MouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void MouseExit();
	virtual void MouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void MouseWheel(int32_t inX, int32_t inY, int32_t inDeltaX, int32_t inDeltaY, uint32_t inModifiers);
	virtual void ShowContextMenu(int32_t inX, int32_t inY);
	static void GetTerminalMetrics(uint32_t inColumns, uint32_t inRows, bool inStatusLine,
	                               uint32_t &outWidth, uint32_t &outHeight);
	static MRect GetIdealTerminalBounds(uint32_t inColumns, uint32_t inRows);
	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	bool IsOpen() const { return mTerminalChannel->IsOpen(); }
	void Open();
	void Close();
	void Destroy();

	virtual void Opened();
	virtual void Closed();

	virtual void SendCommand(std::string inData);
	virtual void SendCommand(const boost::format &inData)
	{
		SendCommand(inData.str());
	}
	virtual void SendMouseCommand(int32_t inButton, int32_t inX, int32_t inY, uint32_t inModifiers);

	void HandleOpened(const boost::system::error_code &ec);
	void HandleReceived(const boost::system::error_code &ec, std::streambuf &inData);

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked);
	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers);
	virtual bool HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
	virtual bool HandleCharacter(const std::string &inText, bool inRepeat);

	void HandleMessage(const std::string &inMessage, const std::string &inLanguage);

	void EnterTOTP(uint32_t inItemIndex);

	MEventIn<void(MSearchDirection)> eSearch;
	void FindNext(MSearchDirection inSearchDirection);

	void ResizeTerminal(uint32_t inColumns, uint32_t inRows, bool inResetCursor = false, bool inResizeWindow = true);

	MEventIn<void()> ePreferencesChanged;
	MEventIn<void(MColor)> ePreviewColor;

	virtual bool PastePrimaryBuffer(const std::string &inText);

  private:
	virtual void ActivateSelf();
	virtual void DeactivateSelf();

	void AdjustScrollbar(int32_t inTopLine);
	void Scroll(MScrollMessage inMessage);

	virtual void AdjustCursor(int32_t inX, int32_t inY, uint32_t inModifiers);

	void ReadPreferences();
	void PreferencesChanged();
	void PreviewColor(MColor inColor);

	MEventIn<void(uint32_t, MRect)> eStatusPartClicked;
	void StatusPartClicked(uint32_t inNr, MRect);
	uint32_t mStatusInfo;

	MStatusbar *mStatusbar;
	MScrollbar *mScrollbar;
	MSearchPanel *mSearchPanel;

	MTerminalChannel *
		mTerminalChannel;
	std::string mSSHCommand;
	int32_t mTerminalWidth, mTerminalHeight;

	MTerminalBuffer mScreenBuffer, mAlternateBuffer, mStatusLineBuffer;
	MTerminalBuffer *mBuffer;

	// emulation support
	void SoftReset();
	void Reset();
	void Emulate();

	void WriteChar(unicode inChar);
	void EraseInDisplay(uint32_t inMode, bool inSelective = false);
	void EraseInLine(uint32_t inMode, bool inSelective = false);

	void ScrollForward();
	void ScrollBackward();

	//	typedef std::function<void(char)>	EscapeHandler;
	void EscapeStart(uint8_t inChar);
	void EscapeVT52(uint8_t inChar);
	void SelectCharSet(uint8_t inChar);
	void SelectControlTransmission(uint8_t inChar);
	void SelectDouble(uint8_t inChar);
	void EscapeCSI(uint8_t inChar);
	void ProcessCSILevel1(uint32_t inCmd);
	void ProcessCSILevel4(uint32_t inCmd);
	void EscapeDCS(uint8_t inChar);
	void CommitPFK();
	void EscapeOSC(uint8_t inChar);

	void SaveCursor();
	void RestoreCursor();
	void ResetCursor();

	void SwitchToAlternateScreen();
	void SwitchToRegularScreen();

	void Beep();

	enum MCursorMovement
	{
		kMoveUp,
		kMoveDown,
		kMoveLeft,
		kMoveRight,
		kMoveIND,
		kMoveRI,
		kMoveCR,
		kMoveLF,
		kMoveCRLF,
		kMoveHT,
		kMoveCBT,
		kMoveBI,
		kMoveFI,
		kMoveSL,
		kMoveSR
	};

	void MoveCursor(MCursorMovement inDirection);
	void MoveCursorTo(int32_t inX, int32_t inY);

	bool CursorIsInMargins() const
	{
		return mCursor.y >= mMarginTop and mCursor.y <= mMarginBottom and mCursor.x >= mMarginLeft and mCursor.x <= mMarginRight;
	}

	void SetTabstop();
	uint32_t GetParam(uint32_t inParamNr, uint32_t inDefaultValue);
	void GetRectParam(uint32_t inParamOffset,
	                  int32_t &outTop, int32_t &outLeft, int32_t &outBottom, int32_t &outRight);

	void SetResetMode(uint32_t inMode, bool inANSI, bool inSet);
	bool GetMode(uint32_t inMode, bool inANSI);

	MRect GetCharacterBounds(uint32_t inLine, uint32_t inColumn);
	bool GetCharacterForPosition(int32_t inX, int32_t inY, int32_t &outLine, int32_t &outColumn);

	void Idle(double inTime);

	MEventIn<void(double)>
		eIdle;

	std::deque<char>
		mInputBuffer;

	static std::string sSelectBuffer;
	static std::list<MTerminalView *>
		sTerminalList;

	enum MColorType
	{
		eText,
		eBack,
		eBold
	};
	MColor mTerminalColors[3];

	std::string mFont;
	float mCharWidth;
	int32_t mLineHeight;
	int32_t mMarginLeft, mMarginTop, mMarginRight, mMarginBottom;
	MEncoding mEncoding;
	std::vector<bool>
		mTabStops;

	struct MCursor
	{
		bool saved;
		int x, y;
		const wchar_t *charSetG[4];
		char charSetGSel[4];
		int CSGL, CSGR;
		int SS;
		MStyle style;
		bool DECOM, DECAWM;
		bool blink, block;
	} mCursor, mSaved, mAlternate, mSavedSL;

	std::map<int, bool>
		mSavedPrivateMode;

	// DEC modes
	bool mDECCKM;
	bool mDECANM;
	bool mDECSCLM;
	bool mDECSCNM;
	bool mDECARM;
	bool mDECPFF;
	bool mDECPEX;
	bool mDECNMK;
	bool mDECBKM;
	// VT220
	int mDECSCL;
	bool mDECTCEM;
	bool mDECNRCM;
	// VT420
	bool mDECVSSM;

	// ANSI modes
	bool mIRM;
	bool mKAM;
	bool mSRM;
	bool mLNM;

	// VT220 support
	bool mS8C1T;
	struct MPFK *mPFK; // device control strings
	struct MPFK *mNewPFK;
	bool mUDKWithShift;

	// handling of escape sequences
	enum
	{
		eESC_NONE,
		eESC_SEEN,
		eCSI,
		eDCS,
		eDCS_ESC,
		eOSC,
		eOSC_ESC,
		ePM,
		ePM_ESC,
		eAPC,
		eAPC_ESC,

		eSELECT_CHAR_SET,
		eSELECT_DOUBLE,
		eSELECT_CTRL_TRANSM,

		eVT52_LINE,
		eVT52_COLUMN
	} mEscState;

	int mState;
	std::vector<uint32_t>
		mArgs;
	std::string mArgString;
	std::string mCtrlSeq;
	uint32_t mCSICmd;
	uint32_t mPFKKeyNr;

	// requests coming from host
	std::string mDECRQSS;

	// some other state information
	double mLastBeep;
	bool mBlockCursor, mBlinkCursor, mBlinkOn;
	double mLastBlink, mNextSmoothScroll;
	int32_t mScrollForwardCount;

	enum
	{
		eNoClick,
		eWaitClick,
		eSingleClick,
		eDoubleClick,
		eTripleClick,
		eTrackClick
	} mMouseClick;
	bool mMouseBlockSelect;
	double mLastMouseDown;
	int32_t mLastMouseX, mLastMouseY;
	int32_t mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol;

	// graphical beep support
	MEventIn<void()> eAnimate;
	void Animate();

	// status line
	bool mDECSASD;
	int mDECSSDT;

	// rectangle extend
	bool mDECSACE;

	wchar_t mLastChar;
	bool mLastCtrl;

	// xterm key handling
	bool mAltSendsEscape;
	bool mDeleteIsDel;
	bool mOldFnKeys;
	bool mXTermKeys;

	std::string ProcessKeyCommon(uint32_t inKeyCode, uint32_t inModifiers);
	std::string ProcessKeyVT52(uint32_t inKeyCode, uint32_t inModifiers);
	std::string ProcessKeyANSI(uint32_t inKeyCode, uint32_t inModifiers);
	std::string ProcessKeyXTerm(uint32_t inKeyCode, uint32_t inModifiers);

	MAnimationManager *mAnimationManager;
	MAnimationVariable *mGraphicalBeep;
	bool mAudibleBeep;
	MAnimationVariable *mDisabledFactor;
	bool mIgnoreColors;

	enum MouseTrackingMode
	{
		eTrackMouseNone,
		eTrackMouseSendXYOnClick = 9,
		eTrackMouseSendXYOnButton = 1000,
		eTrackMouseHilightTracking = 1001,
		eTrackMouseCellMotionTracking = 1002,
		eTrackMouseAllMotionTracking = 1003
	} mMouseMode;
	int32_t mMouseTrackX, mMouseTrackY;

#if DEBUG
	bool mDebugUpdate;
#endif
};
