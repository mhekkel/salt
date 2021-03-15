//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <filesystem>

#include "MControls.hpp"
#include "MHandler.hpp"

class MCanvasImpl;

class MCanvas : public MControl<MCanvasImpl>
{
  public:
	typedef MCanvasImpl MImpl;

	MCanvas(const std::string &inID, MRect inBounds, bool inAcceptDropFiles, bool inAcceptDropText);
	virtual ~MCanvas();

	virtual void AddedToWindow();

	virtual void Invalidate();

	virtual void DragEnter();
	virtual bool DragWithin(int32_t inX, int32_t inY);
	virtual void DragLeave();
	virtual bool Drop(bool inMove, int32_t inX, int32_t inY,
	                  const std::string &inText);
	virtual bool Drop(int32_t inX, int32_t inY,
	                  const std::filesystem::path &inFile);

	virtual void StartDrag();
	virtual void DragSendData(std::string &outData);
	virtual void DragDeleteData();

  protected:
	bool mAcceptDropFiles;
	bool mAcceptDropText;
};
