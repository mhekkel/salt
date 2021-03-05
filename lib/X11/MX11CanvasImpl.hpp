//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MGTKCANVASIMPL_H
#define MGTKCANVASIMPL_H

#include "MCanvasImpl.hpp"
#include "MX11ControlsImpl.hpp"

class MX11CanvasImpl// : public MX11ControlImpl<MCanvas>
{
  public:
					MX11CanvasImpl(MCanvas* inCanvas, uint32_t inWidth, uint32_t inHeight);
	virtual 		~MX11CanvasImpl();
	
	virtual void	CreateWidget();

	virtual bool	OnMouseDown(int32_t inX, int32_t inY, uint32_t inButtonNr, uint32_t inClickCount, uint32_t inModifiers);
	virtual bool	OnMouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool	OnMouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool	OnMouseExit();
	
//	virtual void	MoveFrame(int32_t inXDelta, int32_t inYDelta);
//	virtual void	ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);
//	virtual void	AddedToWindow();
//
//	void			GetBounds(// MRect& outBounds) const;
//	void			SetBounds(// const MRect& inBounds);
//	void			ResizeTo(// int32_t inWidth, // int32_t inHeight);
//	void			ConvertToGlobal(// int32_t& ioX, // int32_t& ioY);
//
//	void			UpdateNow();
//
	void			Invalidate();
//	void			Invalidate(const MRect& inRect);
//
//	void			Scroll(const MRect& inRect, int32_t inX, int32_t inY);
//	void			Scroll(int32_t inX, int32_t inY);
//
//	virtual void	Add(MView* inSubView);
//
//	void			SetCursor(MCursor inCursor);
//
//	void			ObscureCursor();
//
	// MCanvasImpl overrides
	virtual void	AcceptDragAndDrop(bool inFiles, bool inText);
	virtual void	StartDrag();
//	virtual void	SetFocus();
//	virtual void	ReleaseFocus();
//	virtual bool	IsFocus() const;
//	virtual void	TrackMouse(bool inTrackMove, bool inTrackExit);
//
//	// called for printing
//	virtual uint32_t	CountPages(MDevice& inDevice);

  protected:
//	virtual bool	OnExposeEvent(GdkEventExpose* inEvent);
//	virtual bool	OnConfigureEvent(GdkEventConfigure* inEvent);
//	
//	virtual bool	OnKeyPressEvent(GdkEventKey* inEvent);
//	virtual bool	OnCommit(gchar* inText);
//
//	virtual bool	OnScrollEvent(GdkEventScroll* inEvent);
};

#endif
