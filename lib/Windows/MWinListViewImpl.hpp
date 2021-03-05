//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MListViewImpl.hpp"
#include "MWinProcMixin.hpp"
#include "MControls.hpp"

class MWinListViewImpl : public MListViewImpl
{
  public:
					MWinListViewImpl(MListView* inListView);

					~MWinListViewImpl();

	virtual void	AppendColumn(const std::string& inLabel, int inWidth);

//	virtual void	AddedToWindow();

	MEventIn<void(uint32_t,uint32_t)>
					eColumnResized;

	void			ColumnResized(uint32_t inItemNr, uint32_t inWidth);

  protected:
	MListHeader*	mListHeader;
	std::vector<uint32_t>
					mColumnWidths;
};
