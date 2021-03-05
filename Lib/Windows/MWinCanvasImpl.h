//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>
#include "comptr.h"

#include <boost/filesystem/path.hpp>

#include "MWinProcMixin.h"
#include "MCanvasImpl.h"

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

	virtual void	MoveFrame(int32 inXDelta, int32 inYDelta);

	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);

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

	void			MapXY(int32& ioX, int32& ioY);

  private:

	ID2D1HwndRenderTargetPtr
					mRenderTarget;

	double			mLastClickTime;
	uint32			mClickCount;
	HMONITOR		mMonitor;
	
	MIDropTargetPtr	mDropTarget;
};
