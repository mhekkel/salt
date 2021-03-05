//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MGTKCANVASIMPL_H
#define MGTKCANVASIMPL_H

#include "MCanvasImpl.hpp"
#include "MXcbControlsImpl.hpp"

class MXcbCanvasImpl// : public MXcbControlImpl<MCanvas>
{
  public:
					MXcbCanvasImpl(MCanvas* inCanvas, uint32 inWidth, uint32 inHeight);
	virtual 		~MXcbCanvasImpl();
	
	virtual void	CreateWidget();

	virtual bool	OnMouseDown(int32 inX, int32 inY, uint32 inButtonNr, uint32 inClickCount, uint32 inModifiers);
	virtual bool	OnMouseMove(int32 inX, int32 inY, uint32 inModifiers);
	virtual bool	OnMouseUp(int32 inX, int32 inY, uint32 inModifiers);
	virtual bool	OnMouseExit();
	
//	virtual void	MoveFrame(int32 inXDelta, int32 inYDelta);
//	virtual void	ResizeFrame(int32 inWidthDelta, int32 inHeightDelta);
//	virtual void	AddedToWindow();
//
//	void			GetBounds(// MRect& outBounds) const;
//	void			SetBounds(// const MRect& inBounds);
//	void			ResizeTo(// int32 inWidth, // int32 inHeight);
//	void			ConvertToGlobal(// int32& ioX, // int32& ioY);
//
//	void			UpdateNow();
//
	void			Invalidate();
//	void			Invalidate(const MRect& inRect);
//
//	void			Scroll(const MRect& inRect, int32 inX, int32 inY);
//	void			Scroll(int32 inX, int32 inY);
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
//	virtual uint32	CountPages(MDevice& inDevice);

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
