//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MView.hpp"

class MExploreBrowserView : public MView
{
  public:

	static MExploreBrowserView* Create(const std::string& inID, MRect inBounds);

	virtual void	AddItem(const std::string& inName, const std::string& inOwner,
						int64_t inFileSize, std::time_t inModificationTime) = 0;

  protected:

			MExploreBrowserView(const std::string& inID, MRect inBounds)
				: MView(inID, inBounds) {}
};
