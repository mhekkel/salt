//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "MWinListViewImpl.hpp"
#include "MWinControlsImpl.hpp"
#include "MWinUtils.hpp"

using namespace std;

// --------------------------------------------------------------------

MWinListViewImpl::MWinListViewImpl(
	MListView*		inListView)
	: MListViewImpl(inListView)
	, eColumnResized(this, &MWinListViewImpl::ColumnResized)
{
	MRect bounds;
	mListView->GetBounds(bounds);
	bounds.x = bounds.y = 0;
	bounds.height = 16;

	mListHeader = new MListHeader("header", bounds);
	mListView->AddChild(mListHeader);

	AddRoute(eColumnResized, mListHeader->eColumnResized);
}

MWinListViewImpl::~MWinListViewImpl()
{
	delete mListHeader;
}

void MWinListViewImpl::AppendColumn(
	const string&	inLabel,
	int				inWidth)
{
	mListHeader->AppendColumn(inLabel, inWidth);
	mColumnWidths.push_back(inWidth);
}

void MWinListViewImpl::ColumnResized(
	uint32_t			inColumnNr,
	uint32_t			inColumnWidth)
{
	if (inColumnNr < mColumnWidths.size())
		mColumnWidths[inColumnNr] = inColumnWidth;
}

MListViewImpl* MListViewImpl::Create(MListView* inListView)
{
	return new MWinListViewImpl(inListView);
}
