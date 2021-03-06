//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>
#include "comptr.hpp"

#include <filesystem>

#include "MWinProcMixin.hpp"
#include "MCanvasImpl.hpp"

typedef ComPtr<ID2D1RenderTarget>		ID2D1RenderTargetPtr;
typedef ComPtr<ID2D1HwndRenderTarget>	ID2D1HwndRenderTargetPtr;
typedef ComPtr<IDropTarget>				MIDropTargetPtr;

class MWinCanvasImpl : public MCanvasImpl, public MWinProcMixin
{
  public:
					MWinCanvasImpl(MCanvas* inCanvas);

	virtual 		~MWinCanvasImpl();
	
	ID2D1RenderTargetPtr
					GetRenderTarget();

	virtual void	MoveFrame(int32_t inXDelta, int32_t inYDelta);

	virtual void	ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	virtual void	AddedToWindow();

	virtual void	AcceptDragAndDrop(bool inFiles, bool inText);
	virtual void	StartDrag();

	virtual void	SetFocus();
	virtual void	ReleaseFocus();
	virtual bool	IsFocus() const;

	virtual void	TrackMouse(bool inTrackMove, bool inTrackExit);

	virtual MWinProcMixin* GetWinProcMixin()	{ return this;  }

  protected:

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	RegisterParams(UINT& outStyle, int& outWndExtra, HCURSOR& outCursor,
						HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground);

	virtual bool	WMDestroy(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);

	virtual bool	WMPaint(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMEraseBkgnd(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMSize(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMWindowPosChanged(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	virtual bool	WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	virtual bool	WMSetCursor(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult);

	void			MapXY(int32_t& ioX, int32_t& ioY);

  private:

	ID2D1HwndRenderTargetPtr
					mRenderTarget;

	double			mLastClickTime;
	uint32_t			mClickCount;
	HMONITOR		mMonitor;
	
	MIDropTargetPtr	mDropTarget;
};
