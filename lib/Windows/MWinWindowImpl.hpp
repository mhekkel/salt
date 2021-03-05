//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <map>

#include "MWindowImpl.hpp"
#include "MWinProcMixin.hpp"

class MWinWindowImpl : public MWindowImpl, public MWinProcMixin
{
  public:
					MWinWindowImpl(MWindowFlags inFlags,
						const std::string& inMenu, MWindow* inWindow);
	virtual			~MWinWindowImpl();

	virtual void	Create(MRect inBounds, const std::wstring& inTitle);

	virtual void	SetTransparency(float inAlpha);

	virtual void	SetTitle(std::string inTitle);

	virtual void	Show();
	virtual void	Hide();

	virtual bool	Visible() const;

	virtual void	Select();
	virtual void	Close();

//	virtual void	SetFocus(MView* inFocus);
	virtual MHandler*
					GetFocus();

	virtual void	ResizeWindow(int32 inWidthDelta, int32 inHeightDelta);

	virtual void	SetWindowPosition(MRect inBounds, bool inTransition);
	virtual void	GetWindowPosition(MRect& outBounds) const;
	
	virtual void	Invalidate(MRect inRect);
	virtual void	Validate(MRect inRect);
	virtual void	UpdateNow();

	virtual void	ScrollRect(MRect inRect, int32 inDeltaH, int32 inDeltaV);
	
	virtual bool	GetMouse(int32& outX, int32& outY, uint32& outModifiers);
	virtual bool	WaitMouseMoved(int32 inX, int32 inY);

	virtual uint32	GetModifiers() const;

	virtual void	SetCursor(MCursor inCursor);
	virtual void	ObscureCursor();

	virtual void	ConvertToScreen(int32& ioX, int32& ioY) const;
	virtual void	ConvertFromScreen(int32& ioX, int32& ioY) const;

	MWindow*		GetWindow() const						{ return mWindow; }

	virtual bool	IsDialogMessage(MSG& inMesssage);

  protected:

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	RegisterParams(UINT& outStyle, int& outWndExtra, HCURSOR& outCursor,
						HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground);

	virtual LRESULT	WinProc(HWND inHWnd, UINT inUMsg,
						WPARAM inWParam, LPARAM inLParam);

	virtual bool	WMCreate(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMClose(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMDestroy(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMActivate(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseActivate(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMSize(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMSizing(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMPaint(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMEraseBkgnd(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMInitMenu(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
//	virtual bool	WMMenuChar(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMenuCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseWheel(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMContextMenu(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMSetCursor(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
//	virtual bool	WMImeComposition(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
//	virtual bool	WMImeStartComposition(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
//	virtual bool	WMImeRequest(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMDropFiles(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	virtual bool	WMThemeChanged(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMDwmCompositionChanged(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	//virtual bool	WMPositionChanged(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMNCCalcSize(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMNCHitTest(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
	virtual bool	DispatchCharacter(const std::string& inText, bool inRepeat);
	virtual bool	DispatchCommand(uint32 inCommand, uint32 inModifiers);

	bool			UpdateNonClientMargins();
	void			PaintCustomCaption(HDC inHdc);

	HWND			mSizeBox;
	HWND			mStatus;
	int32			mMinWidth, mMinHeight;
	MMenu*			mMenubar;
	int32			mLastGetMouseX, mLastGetMouseY;
	MView*			mMousedView;
	uint32			mClickCount;
	double			mLastClickTime;
	IDropTarget*	mDropTarget;
	
	struct { int32 left, top, right, bottom; }
					mNonClientOffset, mNonClientMargin;
	bool			mCustomNonClient, mCallDWP;
	int32			mCaptionHeight;
	int32			mHorizontalResizeMargin, mVerticalResizeMargin;
};
