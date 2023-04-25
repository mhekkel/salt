//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>

#include "MCanvasImpl.hpp"
#include "MGtkControlsImpl.hpp"

class MGtkCanvasImpl : public MGtkControlImpl<MCanvas>
{
  public:
	MGtkCanvasImpl(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight);
	virtual ~MGtkCanvasImpl();

	cairo_t* GetCairo() const
	{
		assert(mCurrentCairo);
		return mCurrentCairo;
	}

	virtual void CreateWidget();

	virtual bool OnMouseDown(int32_t inX, int32_t inY, uint32_t inButtonNr, uint32_t inClickCount, uint32_t inModifiers);
	virtual bool OnMouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool OnMouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool OnMouseExit();

	void Invalidate();

	// MCanvasImpl overrides
	virtual void AcceptDragAndDrop(bool inFiles, bool inText);
	virtual void StartDrag();

  protected:
	virtual bool OnDrawEvent(cairo_t *inCairo);
	virtual bool OnConfigureEvent(GdkEventConfigure *inEvent);

	virtual bool OnKeyPressEvent(GdkEventKey *inEvent);
	virtual bool OnCommit(gchar *inText);

	virtual bool OnScrollEvent(GdkEventScroll *inEvent);

	cairo_t* mCurrentCairo = nullptr;
};
