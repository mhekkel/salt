//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MExploreBrowserView.hpp"

class MExploreBrowserImpl;

class MWinExploreBrowserView : public MExploreBrowserView
{
  public:
					MWinExploreBrowserView(const std::string& inID, MRect inBounds);
					~MWinExploreBrowserView();

	virtual void	AddedToWindow();
	virtual void	ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	virtual void	AddItem(const std::string& inName, const std::string& inOwner,
						int64_t inFileSize, std::time_t inModificationTime);

  protected:
	
	MExploreBrowserImpl*	mImpl;
};

