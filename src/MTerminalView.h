// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include <deque>
#include <list>
#include <map>

#include <boost/function.hpp>
#include <boost/format.hpp>

//#include <assh/terminal_channel.hpp>

#include "MCanvas.h"
#include "MHandler.h"
#include "MP2PEvents.h"
#include "MUnicode.h"
#include "MColor.h"
#include "MTerminalBuffer.h"
#include "MTerminalChannel.h"
#include "MSearchPanel.h"

class MStatusbar;
class MScrollbar;
class MAnimationVariable;
class MAnimationManager;

class MTerminalView : public MCanvas
{
  public:
					MTerminalView(const std::string& inID, MRect inBounds, MStatusbar* inStatusbar, MScrollbar* inScrollbar,
						MSearchPanel* inSearchPanel, MTerminalChannel* inTerminalChannel, const std::string& inSSHCommand);
	virtual			~MTerminalView();

	static MTerminalView*
					GetFrontTerminal();
	
	MEventIn<void(MScrollMessage)>		eScroll;
	
	// What lines are visible:
	int32			GetTopLine() const;
	
	// virtual void	Draw(MRect inUpdate);
	virtual void	Draw(cairo_t* inCairo);
	virtual void	MouseDown(int32 inX, int32 inY, uint32 inClickCount, uint32 inModifiers);
	virtual void	MouseMove(int32 inX, int32 inY, uint32 inModifiers);
	virtual void	MouseExit();
	virtual void	MouseUp(int32 inX, int32 inY, uint32 inModifiers);
	virtual void	MouseWheel(int32 inX, int32 inY, int32 inDeltaX, int32 inDeltaY, uint32 inModifiers);
	virtual void	ShowContextMenu(int32 inX, int32 inY);
	static void		GetTerminalMetrics(uint32 inColumns, uint32 inRows, bool inStatusLine,
						uint32& outWidth, uint32& outHeight);
	static MRect	GetIdealTerminalBounds(uint32 inColumns, uint32 inRows);
	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);

	bool			IsOpen() const					{ return mTerminalChannel->IsOpen(); }
	void			Open();
	void			Close();
	void			Destroy();

	virtual void	Opened();
	virtual void	Closed();

	virtual void	SendCommand(std::string inData);
	virtual void	SendCommand(const boost::format& inData)
						{ SendCommand(inData.str()); }
	virtual void	SendMouseCommand(int32 inButton, int32 inX, int32 inY, uint32 inModifiers);

	void			HandleOpened(const boost::system::error_code& ec);
	void			HandleWritten(const boost::system::error_code& ec, std::size_t inBytesReceived);
	void			HandleReceived(const boost::system::error_code& ec, std::streambuf& inData);

	virtual bool	UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex, bool& outEnabled, bool& outChecked);
	virtual bool	ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex, uint32 inModifiers);
	virtual bool	HandleKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
	virtual bool	HandleCharacter(const std::string& inText, bool inRepeat);

	void			HandleMessage(const std::string& inMessage, const std::string& inLanguage);
	
	void			EnterTOTP(uint32 inItemIndex);

	MEventIn<void(MSearchDirection)> eSearch;
	void			FindNext(MSearchDirection inSearchDirection);

	void			ResizeTerminal(uint32 inColumns, uint32 inRows, bool inResetCursor = false, bool inResizeWindow = true);

	MEventIn<void()>		ePreferencesChanged;
	MEventIn<void(MColor)>	ePreviewColor;

	virtual bool	PastePrimaryBuffer(const std::string& inText);

  private:

	virtual void	ActivateSelf();
	virtual void	DeactivateSelf();

	void			AdjustScrollbar(int32 inTopLine);
	void			Scroll(MScrollMessage inMessage);

	virtual void	AdjustCursor(int32 inX, int32 inY, uint32 inModifiers);

	void			ReadPreferences();
	void			PreferencesChanged();
	void			PreviewColor(MColor inColor);
	
	MEventIn<void(uint32,MRect)>	eStatusPartClicked;
	void			StatusPartClicked(uint32 inNr, MRect);
	uint32			mStatusInfo;

	MStatusbar*		mStatusbar;
	MScrollbar*		mScrollbar;
	MSearchPanel*	mSearchPanel;

	MTerminalChannel*
					mTerminalChannel;
	std::string		mSSHCommand;
	int32			mTerminalWidth, mTerminalHeight;

	MTerminalBuffer	mScreenBuffer, mAlternateBuffer, mStatusLineBuffer;
	MTerminalBuffer* mBuffer;

	// emulation support
	void			SoftReset();
	void			Reset();
	void			Emulate();
	
	void			WriteChar(unicode inChar);
	void			EraseInDisplay(uint32 inMode, bool inSelective = false);
	void			EraseInLine(uint32 inMode, bool inSelective = false);

	void			ScrollForward();
	void			ScrollBackward();
	
//	typedef boost::function<void(char)>	EscapeHandler;
	void			EscapeStart(uint8 inChar);
	void			EscapeVT52(uint8 inChar);
	void			SelectCharSet(uint8 inChar);
	void			SelectControlTransmission(uint8 inChar);
	void			SelectDouble(uint8 inChar);
	void			EscapeCSI(uint8 inChar);
	void			ProcessCSILevel1(uint32 inCmd);
	void			ProcessCSILevel4(uint32 inCmd);
	void			EscapeDCS(uint8 inChar);
	void			CommitPFK();
	void			EscapeOSC(uint8 inChar);

	void			SaveCursor();
	void			RestoreCursor();
	void			ResetCursor();

	void			SwitchToAlternateScreen();
	void			SwitchToRegularScreen();
	
	void			Beep();

	enum MCursorMovement {
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

	void			MoveCursor(MCursorMovement inDirection);
	void			MoveCursorTo(int32 inX, int32 inY);
	
	bool			CursorIsInMargins() const
	{
		return mCursor.y >= mMarginTop and mCursor.y <= mMarginBottom
			and mCursor.x >= mMarginLeft and mCursor.x <= mMarginRight;
	}
	
	void			SetTabstop();
	uint32			GetParam(uint32 inParamNr, uint32 inDefaultValue);
	void			GetRectParam(uint32 inParamOffset,
						int32& outTop, int32& outLeft, int32& outBottom, int32& outRight);

	void			SetResetMode(uint32 inMode, bool inANSI, bool inSet);
	bool			GetMode(uint32 inMode, bool inANSI);

	MRect			GetCharacterBounds(uint32 inLine, uint32 inColumn);
	bool			GetCharacterForPosition(int32 inX, int32 inY, int32& outLine, int32& outColumn);

	void			Idle(double inTime);
	
	MEventIn<void(double)>
					eIdle;

	std::deque<char>
					mInputBuffer;

	static std::string	sSelectBuffer;
	static std::list<MTerminalView*>
						sTerminalList;

	enum MColorType { eText, eBack, eBold }; 
	MColor			mTerminalColors[3];

	std::string		mFont;
	float			mCharWidth;
	int32			mLineHeight;
	int32			mMarginLeft, mMarginTop, mMarginRight, mMarginBottom;
	MEncoding		mEncoding;
	std::vector<bool>
					mTabStops;
	
	struct MCursor
	{
		bool			saved;
		int				x, y;
		const wchar_t*	charSetG[4];
		char			charSetGSel[4];
		int				CSGL, CSGR;
		int				SS;
		MStyle			style;
		bool			DECOM, DECAWM;
		bool			blink, block;
	}				mCursor, mSaved, mAlternate, mSavedSL;
	
	std::map<int,bool>
					mSavedPrivateMode;
	
	// DEC modes
	bool			mDECCKM;
	bool			mDECANM;
	bool			mDECSCLM;
	bool			mDECSCNM;
	bool			mDECARM;
	bool			mDECPFF;
	bool			mDECPEX;
	bool			mDECNMK;
	bool			mDECBKM;
	// VT220
	int				mDECSCL;
	bool			mDECTCEM;
	bool			mDECNRCM;
	// VT420
	bool			mDECVSSM;

	// ANSI modes
	bool			mIRM;
	bool			mKAM;
	bool			mSRM;
	bool			mLNM;
	
	// VT220 support
	bool			mS8C1T;
	struct MPFK*	mPFK;		// device control strings
	struct MPFK*	mNewPFK;
	bool			mUDKWithShift;
	
	// handling of escape sequences
	enum
	{
		eESC_NONE,
		eESC_SEEN,
		eCSI,
		eDCS, eDCS_ESC,
		eOSC, eOSC_ESC,
		ePM, ePM_ESC,
		eAPC, eAPC_ESC,
		
		eSELECT_CHAR_SET,
		eSELECT_DOUBLE,
		eSELECT_CTRL_TRANSM,
		
		eVT52_LINE, eVT52_COLUMN
	}				mEscState;

	int				mState;
	std::vector<uint32>
					mArgs;
	std::string		mArgString;
	std::string		mCtrlSeq;
	uint32			mCSICmd;
	uint32			mPFKKeyNr;
	
	// requests coming from host
	std::string		mDECRQSS;

	// some other state information
	double			mLastBeep;
	bool			mBlockCursor, mBlinkCursor, mBlinkOn;
	double			mLastBlink, mNextSmoothScroll;
	int32			mScrollForwardCount;
	
	enum { eNoClick, eWaitClick, eSingleClick, eDoubleClick, eTripleClick, eTrackClick }
					mMouseClick;
	bool			mMouseBlockSelect;
	double			mLastMouseDown;
	int32			mLastMouseX, mLastMouseY;
	int32			mMinSelLine, mMinSelCol, mMaxSelLine, mMaxSelCol;

	// graphical beep support
	MEventIn<void()>	eAnimate;
	void				Animate();

	// status line
	bool				mDECSASD;
	int					mDECSSDT;
	
	// rectangle extend
	bool				mDECSACE;

	wchar_t				mLastChar;
	bool				mLastCtrl;
	
	// xterm key handling
	bool				mAltSendsEscape;
	bool				mDeleteIsDel;
	bool				mOldFnKeys;
	bool				mXTermKeys;

	std::string			ProcessKeyCommon(uint32 inKeyCode, uint32 inModifiers);
	std::string			ProcessKeyVT52(uint32 inKeyCode, uint32 inModifiers);
	std::string			ProcessKeyANSI(uint32 inKeyCode, uint32 inModifiers);
	std::string			ProcessKeyXTerm(uint32 inKeyCode, uint32 inModifiers);

	MAnimationManager*	mAnimationManager;
	MAnimationVariable*	mGraphicalBeep;
	bool				mAudibleBeep;
	MAnimationVariable*	mDisabledFactor;
	bool				mIgnoreColors;
	
	enum MouseTrackingMode {
		eTrackMouseNone,
		eTrackMouseSendXYOnClick		= 9,
		eTrackMouseSendXYOnButton		= 1000,
		eTrackMouseHilightTracking		= 1001,
		eTrackMouseCellMotionTracking	= 1002,
		eTrackMouseAllMotionTracking	= 1003
	}					mMouseMode;
	int32				mMouseTrackX, mMouseTrackY;

#if DEBUG
	bool				mDebugUpdate;
#endif
};
