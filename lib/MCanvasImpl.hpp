//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MCanvas.hpp"
#include "MControlsImpl.hpp"
#include "MControls.inl"

class MCanvasImpl : public MControlImpl<MCanvas>
{
  public:
					MCanvasImpl(MCanvas* inCanvas)
						: MControlImpl(inCanvas)			{}
	virtual			~MCanvasImpl() {}

	virtual void	Invalidate()	{ mControl->MView::Invalidate(); }

	virtual void	AcceptDragAndDrop(bool inFiles, bool inText) = 0;
	virtual void	StartDrag() = 0;

	static MCanvasImpl*
					Create(MCanvas* inCanvas, uint32_t inWidth, uint32_t inHeight);
};
