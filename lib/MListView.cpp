//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include "MListView.hpp"
#include "MControls.hpp"
#include "MCanvas.hpp"
#include "MDevice.hpp"
#include "MWindow.hpp"

using namespace std;

//---------------------------------------------------------------------

MListRow::MListRow()
	: mSelected(false)
	, mHeight(0)
{
}

MListRow::~MListRow()
{
}

//---------------------------------------------------------------------

class MListViewImpl : public MCanvas
{
  public:
					MListViewImpl(MListView& inList, const string& inID,
						MRect inBounds);
					~MListViewImpl();

	virtual void	ResizeFrame(
						int32_t inWidthDelta, int32_t inHeightDelta);

	// virtual void	Draw(MRect inUpdate);
	virtual void	Draw(cairo_t* inCairo);

	virtual bool	HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
	
	virtual void	MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers);

	void			AddRow(MListRow* inRow, MListRow* inBefore = nullptr);
	void			RemoveRow(MListRow* inRow);
	void			RemoveAllRows();
	
	void			AddColumn(MListColumn* inColumn);
	void			SetResizingColumn(uint32_t inColumnNr);
	MListColumn*	GetColumn(uint32_t inColumnNr);
	
	MListRow*		GetRowForPoint(int32_t inX, int32_t inY);

	void			SelectRow(MListRow* inRow, bool inSelect);
	MListRow*		GetNextSelectedRow(MListRow* inRow);

	int32_t			GetRowIndex(MListRow* inRow) const;
	MListRow*		GetRow(uint32_t inIndex);
	
  private:
	vector<MListRow*>
					mRows;
	vector<MListColumn*>
					mColumns;
	MListView&		mList;
	MListViewFlags	mFlags;
};

MListViewImpl::MListViewImpl(MListView& inList, const string& inID, MRect inBounds)
	: MCanvas(inID, inBounds, false, false)
	, mList(inList)
	, mFlags(mList.mFlags)
{
	//SetViewSize(inBounds.width, 0);
}

MListViewImpl::~MListViewImpl()
{
	for (MListRow* row: mRows)
		delete row;
	for (MListColumn* col: mColumns)
		delete col;
}

void MListViewImpl::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	MCanvas::ResizeFrame(inWidthDelta, inHeightDelta);
	
	switch (mColumns.size())
	{
		case 0:
			break;

		case 1:
			mColumns.front()->SetWidth(mBounds.width);
			break;

		default:
		{
			int32_t w = mBounds.width;
			MListColumn* resizingColumn = nullptr;
			for (MListColumn* col: mColumns)
			{
				if (col->IsResizing())
					resizingColumn = col;
				else
					w -= col->GetWidth();
			}
	
			if (resizingColumn != nullptr)
			{
				if (w < 0)
					w = 0;
				resizingColumn->SetWidth(w);
			}
			break;
		}
	}
}

bool MListViewImpl::HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	bool handled = true;
	switch (inKeyCode)
	{
		case kUpArrowKeyCode:
			break;

		case kDownArrowKeyCode:
			break;
		
		default:
			handled = false;
			break;
	}
	return handled;
}

void MListViewImpl::AddRow(MListRow* inRow, MListRow* inBefore)
{
	vector<MListRow*>::iterator i = find(mRows.begin(), mRows.end(), inBefore);
	mRows.insert(i, inRow);
	
	uint32_t height = 0;
	for (uint32_t i = 0; i < mColumns.size(); ++i)
	{
		uint32_t columnHeight = mColumns[i]->CalculateHeight(*inRow, i);
		if (height < columnHeight)
			height = columnHeight;
	}
	
	inRow->SetHeight(height);
	
	// update scroller
	ResizeFrame(0, height);
	SetScrollUnit(1, height);
}

void MListViewImpl::RemoveRow(MListRow* inRow)
{
	vector<MListRow*>::iterator i = find(mRows.begin(), mRows.end(), inRow);
	if (i != mRows.end())
	{
		mRows.erase(i);
		ResizeFrame(0, -static_cast<int32_t>(inRow->GetHeight()));
		delete inRow;
		
		Invalidate();
	}
}

void MListViewImpl::RemoveAllRows()
{
	for (MListRow* row: mRows)
		delete row;
	mRows.clear();
	ResizeFrame(0, mFrame.height);

	Invalidate();
}

void MListViewImpl::AddColumn(MListColumn* inColumn)
{
	mColumns.push_back(inColumn);	

	// recalculate heights for all rows
	for (uint32_t i = 0; i < mRows.size(); ++i)
	{
		uint32_t height = 0;
		for (uint32_t j = 0; j < mColumns.size(); ++j)
		{
			uint32_t columnHeight = mColumns[j]->CalculateHeight(*mRows[i], j);
			if (height < columnHeight)
				height = columnHeight;
		}
	
		mRows[i]->SetHeight(height);
	}

	Invalidate();
}

void MListViewImpl::SetResizingColumn(uint32_t inColumnNr)
{
	for (MListColumn* col: mColumns)
		col->SetResizing(false);
	
	if (inColumnNr < mColumns.size())
		mColumns[inColumnNr]->SetResizing(true);
}

MListColumn* MListViewImpl::GetColumn(uint32_t inColumnNr)
{
	MListColumn* result = nullptr;
	if (inColumnNr < mColumns.size())
		result = mColumns[inColumnNr];
	return result;
}

void MListViewImpl::Draw(cairo_t* inCairo)
{
	MDevice dev(this, inCairo);
	
	dev.EraseRect(mBounds);
	
	MRect r(0, 0, mBounds.width, dev.GetLineHeight() + 3);

	for (uint32_t i = 0; i < mRows.size() and r.y < inUpdate.y + inUpdate.height; ++i)
	{
		MListRow* row = mRows[i];
		
		if (r & inUpdate)
		{
			MListItemState state = eLIS_Normal;
			if (row->IsSelected())
				state = eLIS_Selected;
			
			dev.DrawListItemBackground(r, state);
			
			MRect b = r;
	
			for (uint32_t j = 0; j < mColumns.size(); ++j)
			{
				b.width = mColumns[j]->GetWidth();
				mColumns[j]->Render(dev, *row, j, b);
				b.x += b.width;
			}
		}
		
		r.y += r.height;
	}
}

void MListViewImpl::MouseDown(int32_t inX, int32_t inY, uint32_t inClickCount, uint32_t inModifiers)
{
	MListRow* row = GetRowForPoint(inX, inY);
	if (row != nullptr)
	{
		if (row->IsSelected())
		{
			if (inClickCount > 1)
				mList.EmitRowInvoked(row);
			else if (mFlags & lvMultipleSelect)
				row->SetSelected(false);
		}
		else
		{
			MListRow* old = GetNextSelectedRow(nullptr);
			if (old != nullptr)
				old->SetSelected(false);

			row->SetSelected(true);
			mList.EmitRowSelected(row);
		}

		Invalidate();
	}
}

MListRow* MListViewImpl::GetRowForPoint(int32_t inX, int32_t inY)
{
	MListRow* result = nullptr;
	if (inX >= 0 and inX < mBounds.width and inY >= 0)
	{
		for (uint32_t i = 0; i < mRows.size(); ++i)
		{
			inY -= mRows[i]->GetHeight();
			if (inY <= 0)
			{
				result = mRows[i];
				break;
			}
		}
	}
	
	return result;
}

void MListViewImpl::SelectRow(MListRow* inRow, bool inSelect)
{
	if (inRow != nullptr and
		find(mRows.begin(), mRows.end(), inRow) != mRows.end() and
		inRow->IsSelected() != inSelect)
	{
		inRow->SetSelected(inSelect);
		
		if (inSelect)
			mList.EmitRowSelected(inRow);
		
		Invalidate();
	}
}

MListRow* MListViewImpl::GetNextSelectedRow(MListRow* inRow)
{
	MListRow* result = nullptr;

	vector<MListRow*>::iterator b = mRows.begin();
	if (inRow != nullptr)
		b = find(mRows.begin(), mRows.end(), inRow);
	vector<MListRow*>::iterator s = find_if(b, mRows.end(), [](MListRow* row) { return row->IsSelected(); });
	if (s != mRows.end())
		result = *s;

	return result;
}

int32_t MListViewImpl::GetRowIndex(MListRow* inRow) const
{
	int32_t result = -1;
	vector<MListRow*>::const_iterator i = find(mRows.begin(), mRows.end(), inRow);
	if (i != mRows.end())
		result = i - mRows.begin();
	return result;
}

MListRow* MListViewImpl::GetRow(uint32_t inIndex)
{
	MListRow* result = nullptr;
	if (inIndex < mRows.size())
		result = mRows[inIndex];
	return result;
}

//---------------------------------------------------------------------

MListColumn::MListColumn(const string& inLabel, uint32_t inWidth)
	: mLabel(inLabel)
	, mFont("Segoe UI 9")
	, mWidth(inWidth)
	, mResizing(false)
{
}

MListColumn::~MListColumn()
{
}

//---------------------------------------------------------------------

MListTextColumn::MListTextColumn(const string& inLabel, uint32_t inWidth)
	: MListColumn(inLabel, inWidth)
{
}

void MListTextColumn::Render(MDevice& inDevice, const MListRow& inRow,
	uint32_t inColumnNr, MRect inBounds)
{
	string text;
	inRow.GetValue(inColumnNr, text);
	
	inDevice.SetFont(mFont);
	
	inBounds.InsetBy(4, 1);
	inDevice.DrawString(text, inBounds);
}

uint32_t MListTextColumn::CalculateHeight(const MListRow& inRow, uint32_t inColumnNr)
{
	MDevice dev;
	dev.SetFont(mFont);
	return dev.GetLineHeight() + 3;
}

//---------------------------------------------------------------------

MListNumberColumn::MListNumberColumn(const string& inLabel,
	uint32_t inWidth, const string& inFormat)
	: MListTextColumn(inLabel, inWidth)
	, mFormat(inFormat)
{
}

void MListNumberColumn::Render(MDevice& inDevice, const MListRow& inRow,
	uint32_t inColumnNr, MRect inBounds)
{
	int64_t v;
	inRow.GetValue(inColumnNr, v);
	
	inDevice.SetFont(mFont);
	
	inBounds.InsetBy(4, 1);
	inDevice.DrawString((boost::format(mFormat) % v).str(), inBounds, eAlignRight);
}

//---------------------------------------------------------------------

MListDotColumn::MListDotColumn(const string& inLabel, uint32_t inWidth)
	: MListColumn(inLabel, inWidth + 6)
{
}

void MListDotColumn::Render(MDevice& inDevice, const MListRow& inRow,
	uint32_t inColumnNr, MRect inBounds)
{
	MColor v;
	inRow.GetValue(inColumnNr, v);
	
	inBounds.InsetBy(3, 1);
	if (inBounds.height > inBounds.width)
		inBounds.InsetBy(0, (inBounds.height - inBounds.width) / 2);
	
	inDevice.SetForeColor(v);
	inDevice.FillEllipse(inBounds);
	inDevice.SetForeColor(kBlack);
}

uint32_t MListDotColumn::CalculateHeight(const MListRow& inRow, uint32_t inColumnNr)
{
	return GetWidth() - 4;
}

//---------------------------------------------------------------------

MListView::MListView(const string& inID, MRect inBounds, MListViewFlags inFlags)
	: MView(inID, inBounds)
	, MHandler(nullptr)
	, mHeader(nullptr)
	, mImpl(nullptr)
	, mFlags(inFlags)
{
}

MListView::~MListView()
{
	//delete mHeader;
	//delete mScroller;
	//delete mImpl;
}

bool MListView::HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
{
	return mImpl->HandleKeyDown(inKeyCode, inModifiers, inRepeat);
}

void MListView::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	MView::ResizeFrame(inWidthDelta, inHeightDelta);
//	mImpl->FrameResized();
}

void MListView::AddedToWindow()
{
	MRect bounds;
	GetBounds(bounds);

	if (mFlags & lvHeader)
	{
		MRect r(bounds);

		r.height = 24;
		mHeader = new MListHeader(GetID() + "-header", r);
		AddChild(mHeader);
		mHeader->SetBindings(true, true, true, false);
		
		bounds.y += r.height - 1;
		bounds.height -= r.height - 1;
	}

	mImpl = new MListViewImpl(*this, GetID() + "-impl", bounds);
	MViewScroller* scroller = new MViewScroller(GetID() + "-scroller", mImpl, false, true);
	scroller->SetBindings(true, true, true, true);
	AddChild(scroller);

	MView::AddedToWindow();
}

void MListView::AddColumn(MListColumn* inColumn)
{
	if (mHeader != nullptr)
		mHeader->AppendColumn(inColumn->GetLabel(), inColumn->GetWidth());
	mImpl->AddColumn(inColumn);
}

void MListView::SetResizingColumn(uint32_t inColumnNr)
{
	mImpl->SetResizingColumn(inColumnNr);
}

MListColumn* MListView::GetColumn(uint32_t inColumnNr)
{
	return mImpl->GetColumn(inColumnNr);
}

void MListView::AddRow(MListRow* inRow, MListRow* inBefore)
{
	mImpl->AddRow(inRow, inBefore);
}

void MListView::RemoveAll()
{
	mImpl->RemoveAllRows();
}

void MListView::RemoveRow(uint32_t inIndex)
{
	MListRow* row = GetRow(inIndex);
	if (row != nullptr)
		RemoveRow(row);
}

void MListView::RemoveRow(MListRow* inRow)
{
	mImpl->RemoveRow(inRow);
}

void MListView::SelectRow(MListRow* inRow, bool inSelect)
{
	mImpl->SelectRow(inRow, inSelect);
}

void MListView::SelectRow(uint32_t inIndex, bool inSelect)
{
	SelectRow(GetRow(inIndex), inSelect);
}

MListRow* MListView::GetFirstSelectedRow() const
{
	return GetNextSelectedRow(nullptr);
}

MListRow* MListView::GetNextSelectedRow(MListRow* inFrom) const
{
	return mImpl->GetNextSelectedRow(inFrom);
}

int32_t MListView::GetRowIndex(MListRow* inRow) const
{
	return mImpl->GetRowIndex(inRow);
}

MListRow* MListView::GetRow(uint32_t inIndex)
{
	return mImpl->GetRow(inIndex);
}

////---------------------------------------------------------------------
//// MListRowBase
//
//MListRowBase::MListRowBase()
//	: mRowReference(NULL)
//{
//}
//
//MListRowBase::~MListRowBase()
//{
//	if (mRowReference != NULL)
//		gtk_tree_row_reference_free(mRowReference);
//}
//
//GtkTreePath* MListRowBase::GetTreePath() const
//{
//	return gtk_tree_row_reference_get_path(mRowReference);
//}
//
//void MListRowBase::UpdateRowReference(
//	GtkTreeModel*	inNewModel,
//	GtkTreePath*	inNewPath)
//{
//	if (mRowReference != nullptr)
//		gtk_tree_row_reference_free(mRowReference);
//	mRowReference = gtk_tree_row_reference_new(inNewModel, inNewPath);
//}
//
//bool MListRowBase::GetModelAndIter(
//	GtkTreeStore*&	outTreeStore,
//	GtkTreeIter&	outTreeIter)
//{
//	bool result = false;
//	if (mRowReference != nullptr and gtk_tree_row_reference_valid(mRowReference))
//	{
//		outTreeStore = GTK_TREE_STORE(gtk_tree_row_reference_get_model(mRowReference));
//		if (outTreeStore != nullptr)
//		{
//			GtkTreePath* path = gtk_tree_row_reference_get_path(mRowReference);
//			if (path != nullptr)
//			{
//				if (gtk_tree_model_get_iter(GTK_TREE_MODEL(outTreeStore), &outTreeIter, path))
//					result = true;
//				gtk_tree_path_free(path);
//			}
//		}
//	}
//	
//	return result;
//}
//
//void MListRowBase::RowChanged()
//{
//	UpdateDataInTreeStore();
//	
//	if (mRowReference != nullptr and gtk_tree_row_reference_valid(mRowReference))
//	{
//		GtkTreeModel* model = gtk_tree_row_reference_get_model(mRowReference);
//		if (model != nullptr)
//		{
//			GtkTreePath* path = gtk_tree_row_reference_get_path(mRowReference);
//			if (path != nullptr)
//			{
//				GtkTreeIter iter;
//				if (gtk_tree_model_get_iter(model, &iter, path))
//					gtk_tree_model_row_changed(model, path, &iter);
//				gtk_tree_path_free(path);
//			}
//		}
//	}
//}
//
//bool MListRowBase::GetParentAndPosition(
//	MListRowBase*&		outParent,
//	uint32_t&				outPosition,
//	uint32_t				inObjectColumn)
//{
//	bool result = false;
//
//	outPosition = 0;
//	outParent = nullptr;
//	
//	if (mRowReference != nullptr and gtk_tree_row_reference_valid(mRowReference))
//	{
//		GtkTreeModel* model = gtk_tree_row_reference_get_model(mRowReference);
//		if (model != nullptr)
//		{
//			GtkTreePath* path = gtk_tree_row_reference_get_path(mRowReference);
//			if (path != nullptr)
//			{
//				result = true;
//				
//				int depth = gtk_tree_path_get_depth(path);
//				gint* indices = gtk_tree_path_get_indices(path);
//
//				if (depth > 0)
//				{
//					outPosition = indices[depth - 1];
//					
//					if (depth > 1)
//					{
//						GtkTreeIter iter;
//						if (gtk_tree_path_up(path) and gtk_tree_model_get_iter(model, &iter, path))
//							gtk_tree_model_get(model, &iter, inObjectColumn, &outParent, -1);
//					}
//				}
//
//				gtk_tree_path_free(path);
//			}
//		}
//	}
//	
//	return result;
//}
//
////---------------------------------------------------------------------
//// MListColumnEditedListener
//
//class MListColumnEditedListener
//{
//  public:
//					MListColumnEditedListener(
//						uint32_t				inColumnNr,
//						MListBase*			inList,
//						GtkCellRenderer*	inRenderer,
//						bool				inListenToToggle,
//						bool				inListenToEdited,
//						bool				inListenToChanged = false)
//						: eToggled(this, &MListColumnEditedListener::Toggled)
//						, eEdited(this, &MListColumnEditedListener::Edited)
//						, mColumnNr(inColumnNr)
//						, mList(inList)
//					{
//						if (inListenToToggle)
//							eToggled.Connect(G_OBJECT(inRenderer), "toggled");
//						if (inListenToEdited)
//							eEdited.Connect(G_OBJECT(inRenderer), "edited");
//						if (inListenToChanged)
//							eEdited.Connect(G_OBJECT(inRenderer), "changed");
//					}
//
//  private:
//
//	void			Toggled(
//						gchar*				inPath)
//					{
//						GtkTreePath* path = gtk_tree_path_new_from_string(inPath);
//						if (path != nullptr)
//						{
//							MListRowBase* row = mList->GetRowForPath(path);
//							row->ColumnToggled(mColumnNr);
//							gtk_tree_path_free(path);
//						}
//					}
//					
//	MSlot<void(gchar*)>						eToggled;
//
//	void			Edited(
//						gchar*				inPath,
//						gchar*				inNewText)
//					{
//						string newText;
//						if (inNewText != nullptr)
//							newText = inNewText;
//						
//						GtkTreePath* path = gtk_tree_path_new_from_string(inPath);
//						if (path != nullptr)
//						{
//							MListRowBase* row = mList->GetRowForPath(path);
//							row->ColumnEdited(mColumnNr, newText);
//							gtk_tree_path_free(path);
//						}
//					}
//					
//	MSlot<void(gchar*,gchar*)>				eEdited;
//
//	uint32_t			mColumnNr;
//	MListBase*		mList;
//};
//
////---------------------------------------------------------------------
//// MListBase
//
//MListBase::RowDropPossibleFunc MListBase::sSavedRowDropPossible;
//MListBase::DragDataReceivedFunc MListBase::sSavedDragDataReceived;
//
//MListBase::MListBase(
//	GtkWidget*	inTreeView)
//	: MView(inTreeView, false)
//	, mCursorChanged(this, &MListBase::CursorChanged)
//	, mRowActivated(this, &MListBase::RowActivated)
//	, mRowChanged(this, &MListBase::RowChanged)
//	, mRowDeleted(this, &MListBase::RowDeleted)
//	, mRowInserted(this, &MListBase::RowInserted)
//	, mRowsReordered(this, &MListBase::RowsReordered)
//{
//	mCursorChanged.Connect(inTreeView, "cursor-changed");
//	mRowActivated.Connect(inTreeView, "row-activated");
//	
//	mRowSelectedTimer.eTimedOut.SetProc(this, &MListBase::RowSelectedTimeOutWaited);
//	mRowEditingTimedOut.eTimedOut.SetProc(this, &MListBase::DisableEditingAndTimer);
//
//	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(GetGtkWidget()), true);
//
////	gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(inTreeView),
////		kTreeDragTargets, kTreeDragTargetCount,
////		GdkDragAction(GDK_ACTION_COPY|GDK_ACTION_MOVE));
////	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(inTreeView), GDK_BUTTON1_MASK,
////		kTreeDragTargets, kTreeDragTargetCount,
////		GdkDragAction(GDK_ACTION_COPY|GDK_ACTION_MOVE));
//}
//
//MListBase::~MListBase()
//{
//	cout << "list deleted" << endl;
//}
//
//void MListBase::CreateTreeStore(
//	std::vector<GType>&		inTypes,
//	std::vector<MGtkRendererInfo>&
//							inRenderers)
//{
//    mTreeStore = gtk_tree_store_newv(inTypes.size(), &inTypes[0]);
//
//	gtk_tree_view_set_model(GTK_TREE_VIEW(GetGtkWidget()), GTK_TREE_MODEL(mTreeStore));
//
//	g_object_set_data(G_OBJECT(mTreeStore), "mlistbase", this);
//	GtkTreeDragDestIface* dragIface = GTK_TREE_DRAG_DEST_GET_IFACE(mTreeStore);
//	if (sSavedRowDropPossible == nullptr)
//	{
//		sSavedRowDropPossible = dragIface->row_drop_possible;
//		dragIface->row_drop_possible = &MListBase::RowDropPossibleCallback;
//		
//		sSavedDragDataReceived = dragIface->drag_data_received;
//		dragIface->drag_data_received = &MListBase::DragDataReceivedCallback;
//	}
//
//	mRowChanged.Connect(G_OBJECT(mTreeStore), "row-changed");
//	mRowDeleted.Connect(G_OBJECT(mTreeStore), "row-deleted");
//	mRowInserted.Connect(G_OBJECT(mTreeStore), "row-inserted");
//	mRowsReordered.Connect(G_OBJECT(mTreeStore), "rows-reordered");
//	
//	for (size_t i = 0; i < inRenderers.size(); ++i)
//	{
//	    GtkCellRenderer* renderer = get<0>(inRenderers[i]);
//	    
//	    mRenderers.push_back(renderer);
//	    
//	    const char* attribute = get<1>(inRenderers[i]);
//	    GtkTreeViewColumn* column = gtk_tree_view_column_new();
//	    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), renderer, TRUE);
//	    gtk_tree_view_column_set_attributes(column, renderer, attribute, i, NULL);
//	    gtk_tree_view_append_column(GTK_TREE_VIEW(GetGtkWidget()), column);
//
//		mListeners.push_back(new MListColumnEditedListener(i, this, renderer,
//					get<2>(inRenderers[i]), get<3>(inRenderers[i])));
//	}
//}
//
//void MListBase::AllowMultipleSelectedItems()
//{
//	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetGtkWidget()));
//	if (selection != nullptr)
//		gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
//}
//
//MListRowBase* MListBase::GetRowForPath(
//	GtkTreePath*		inPath) const
//{
//	MListRowBase* row = nullptr;
//	
//	GtkTreeIter iter;
//	if (inPath != nullptr and gtk_tree_model_get_iter(GTK_TREE_MODEL(mTreeStore), &iter, inPath))
//		gtk_tree_model_get(GTK_TREE_MODEL(mTreeStore), &iter, GetColumnCount(), &row, -1);
//	
//	return row;
//}
//
//GtkTreePath* MListBase::GetTreePathForRow(
//	MListRowBase*		inRow)
//{
//	return inRow->GetTreePath();
//}
//
//bool MListBase::GetTreeIterForRow(
//	MListRowBase*		inRow,
//	GtkTreeIter*		outIter)
//{
//	bool result = false;
//	GtkTreePath* path = GetTreePathForRow(inRow);
//	if (path != nullptr)
//	{
//		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mTreeStore), outIter, path))
//			result = true;
//		gtk_tree_path_free(path);
//	}
//	return result;
//}
//
//MListRowBase* MListBase::GetCursorRow() const
//{
//	MListRowBase* row = nullptr;
//	
//	GtkTreePath* path;
//	gtk_tree_view_get_cursor(GTK_TREE_VIEW(GetGtkWidget()), &path, nullptr);
//	if (path != nullptr)
//	{
//		row = GetRowForPath(path);
//		gtk_tree_path_free(path);
//	}
//	
//	return row;
//}
//
//void MListBase::GetSelectedRows(
//	list<MListRowBase*>&	outRows) const
//{
//	GList* rows = gtk_tree_selection_get_selected_rows(
//		gtk_tree_view_get_selection(GTK_TREE_VIEW(GetGtkWidget())), nullptr);
//					
//	if (rows != nullptr)
//	{
//		GList* row = rows;
//		while (row != nullptr)
//		{
//			outRows.push_back(GetRowForPath((GtkTreePath*)row->data));
//			row = row->next;
//		}
//		
//		g_list_for (rows : (GFunc)(gtk_tree_path_free), nullptr);
//		g_list_free(rows);
//	}
//}
//
//void MListBase::SetColumnTitle(
//	uint32_t			inColumnNr,
//	const string&	inTitle)
//{
//	GtkTreeViewColumn* column = gtk_tree_view_get_column(GTK_TREE_VIEW(GetGtkWidget()), inColumnNr);
//	if (column == NULL)
//		throw "column not found";
//	gtk_tree_view_column_set_title(column, inTitle.c_str());
//}
//
//void MListBase::SetExpandColumn(
//	uint32_t			inColumnNr)
//{
//	GtkTreeViewColumn* column = gtk_tree_view_get_column(GTK_TREE_VIEW(GetGtkWidget()), inColumnNr);
//	if (column == NULL)
//		throw "column not found";
//	g_object_set(G_OBJECT(column), "expand", true, nullptr);
//}
//
//void MListBase::SetColumnAlignment(
//	uint32_t				inColumnNr,
//	float				inAlignment)
//{
//	GtkTreeViewColumn* column = gtk_tree_view_get_column(GTK_TREE_VIEW(GetGtkWidget()), inColumnNr);
//	if (column == NULL)
//		throw "column not found";
//	gtk_tree_view_column_set_alignment(column, inAlignment);
//	if (inColumnNr < mRenderers.size())
//		g_object_set(G_OBJECT(mRenderers[inColumnNr]), "xalign", inAlignment, nullptr);
//}
//
//void MListBase::SetColumnEditable(
//	uint32_t			inColumnNr,
//	bool			inEditable)
//{
//	if (inColumnNr < mRenderers.size())
//	{
//		if (inEditable)
//			mEnabledEditColumns.insert(inColumnNr);
//		else
//		{
//			mEnabledEditColumns.erase(inColumnNr);
//			g_object_set(G_OBJECT(mRenderers[inColumnNr]), "editable", false, nullptr);
//		}
//	}
//}
//
//void MListBase::SetColumnToggleable(
//	uint32_t			inColumnNr,
//	bool			inToggleable)
//{
//	if (inColumnNr < mRenderers.size())
//		 g_object_set(G_OBJECT(mRenderers[inColumnNr]), "activatable", inToggleable, nullptr);
//}
//
//void MListBase::SetListOfOptionsForColumn(
//	uint32_t					inColumnNr,
//	const vector<string>&	inOptions)
//{
//	if (inColumnNr >= mRenderers.size())
//		THROW(("Invalid column specified for SetListOfOptionsForColumn"));
//	
//	GtkListStore* model = gtk_list_store_new(1, G_TYPE_STRING);
//	
//	for (auto option = inOptions.begin(); option != inOptions.end(); ++option)
//	{
//		GtkTreeIter iter;
//		gtk_list_store_append(model, &iter);
//		gtk_list_store_set(model, &iter, 0, option->c_str(), -1);
//	}
//
//	g_object_set(G_OBJECT(mRenderers[inColumnNr]),
//		"text-column", 0,
//		"editable", true,
//		"has-entry", true,
//		"model", model,
//		nullptr);
//	
////	GtkTreeViewColumn* column = gtk_tree_view_get_column(GTK_TREE_VIEW(GetGtkWidget()), inColumnNr);
////	gtk_cell_layout_clear(GTK_CELL_LAYOUT(column));
////
////	GtkCellRenderer* renderer = gtk_cell_renderer_combo_new();
////    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), renderer, TRUE);
////    gtk_tree_view_column_set_attributes(column, renderer, "text", inColumnNr, nullptr);
////	g_object_set(G_OBJECT(renderer), "text-column", 0, "editable", true,
////		"has-entry", true, "model", model, nullptr);
////
////	delete mListeners[inColumnNr];
////	mListeners[inColumnNr] = new MListColumnEditedListener(inColumnNr, this, renderer,
////		false, false, true);
//}
//
//void MListBase::CollapseRow(
//	MListRowBase*		inRow)
//{
//	GtkTreePath* path = GetTreePathForRow(inRow);
//	if (path != nullptr)
//	{
//		gtk_tree_view_collapse_row(GTK_TREE_VIEW(GetGtkWidget()), path);
//		gtk_tree_path_free(path);
//	}
//}
//
//void MListBase::ExpandRow(
//	MListRowBase*		inRow,
//	bool				inExpandAll)
//{
//	GtkTreePath* path = GetTreePathForRow(inRow);
//	if (path != nullptr)
//	{
//		gtk_tree_view_expand_row(GTK_TREE_VIEW(GetGtkWidget()), path, inExpandAll);
//		gtk_tree_path_free(path);
//	}
//}
//
//void MListBase::CollapseAll()
//{
//	gtk_tree_view_collapse_all(GTK_TREE_VIEW(GetGtkWidget()));
//}
//
//void MListBase::ExpandAll()
//{
//	gtk_tree_view_expand_all(GTK_TREE_VIEW(GetGtkWidget()));
//}
//
//void MListBase::AppendRow(
//	MListRowBase*		inRow,
//	MListRowBase*		inParentRow)
//{
//	// store in tree
//	GtkTreeIter iter, parent;
//	
//	if (inParentRow != nullptr and GetTreeIterForRow(inParentRow, &parent))
//		gtk_tree_store_append(mTreeStore, &iter, &parent);
//	else
//		gtk_tree_store_append(mTreeStore, &iter, NULL);
//
//	GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(mTreeStore), &iter);
//	if (path != nullptr)
//	{
//		inRow->mRowReference = gtk_tree_row_reference_new(GTK_TREE_MODEL(mTreeStore), path);
//		gtk_tree_path_free(path);
//	}
//
//	inRow->UpdateDataInTreeStore();
//}
//
//void MListBase::InsertRow(
//	MListRowBase*		inRow,
//	MListRowBase*		inBefore)
//{
//	// store in tree
//	GtkTreeIter iter, siblingIter;
//	
//	if (inBefore != nullptr and GetTreeIterForRow(inBefore, &siblingIter))
//		gtk_tree_store_insert_before(mTreeStore, &iter, nullptr, &siblingIter);
//	else
//		gtk_tree_store_append(mTreeStore, &iter, NULL);
//
//	GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(mTreeStore), &iter);
//	if (path != nullptr)
//	{
//		inRow->mRowReference = gtk_tree_row_reference_new(GTK_TREE_MODEL(mTreeStore), path);
//		gtk_tree_path_free(path);
//	}
//
//	inRow->UpdateDataInTreeStore();
//}
//
//void MListBase::RemoveRow(
//	MListRowBase*		inRow)
//{
//	GtkTreeIter iter;
//	if (GetTreeIterForRow(inRow, &iter))
//		gtk_tree_store_remove(mTreeStore, &iter);
//}
//
//gboolean MListBase::RemoveAllCB(
//	GtkTreeModel*		inModel,
//	GtkTreePath*		inPath,
//	GtkTreeIter*		inIter,
//	gpointer			inData)
//{
//	MListRowBase* row = reinterpret_cast<MListBase*>(inData)->GetRowForPath(inPath);
//	delete row;
//	return false;
//}
//
//void MListBase::RemoveAll()
//{
//	gtk_tree_model_for (GTK_TREE_MODEL(mTreeStore) : &MListBase::RemoveAllCB, this);
//	
//	gtk_tree_store_clear(mTreeStore);
//}
//
//void MListBase::SelectRow(
//	MListRowBase*		inRow)
//{
//	GtkTreePath* path = inRow->GetTreePath();
//	if (path != nullptr)
//	{
//		GtkTreeViewColumn* column = gtk_tree_view_get_column(GTK_TREE_VIEW(GetGtkWidget()), 0);
//		
//		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(GetGtkWidget()),
//			path, column, false, 0, 0);
//
//		gtk_tree_view_set_cursor(GTK_TREE_VIEW(GetGtkWidget()),
//			path, column, false);
//
//		gtk_tree_path_free(path);
//	}
//}
//
//void MListBase::SelectRowAndStartEditingColumn(
//	MListRowBase*		inRow,
//	uint32_t				inColumnNr)
//{
//	GtkTreePath* path = inRow->GetTreePath();
//	if (path != nullptr)
//	{
//		GtkTreeViewColumn* column = gtk_tree_view_get_column(GTK_TREE_VIEW(GetGtkWidget()), 0);
//		
//		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(GetGtkWidget()),
//			path, column, false, 0, 0);
//
//		gtk_tree_view_set_cursor(GTK_TREE_VIEW(GetGtkWidget()),
//			path, column, true);
//
//		gtk_tree_path_free(path);
//	}
//}
//
//gboolean MListBase::RowDropPossibleCallback(
//	GtkTreeDragDest*	inTreeDragDest,
//	GtkTreePath*		inPath,
//	GtkSelectionData*	inSelectionData)
//{
//	bool result = (*MListBase::sSavedRowDropPossible)(inTreeDragDest, inPath, inSelectionData);
//	if (result)
//	{
//		MListBase* self = reinterpret_cast<MListBase*>(g_object_get_data(G_OBJECT(inTreeDragDest), "mlistbase"));
//		if (self != nullptr)
//			result = self->RowDropPossible(inPath, inSelectionData);
//	}
//
//	return result;
//}
//
//bool MListBase::RowDropPossible(
//	GtkTreePath*		inTreePath,
//	GtkSelectionData*	inSelectionData)
//{
//	bool result = false;
//
//	GtkTreePath* path = gtk_tree_path_copy(inTreePath);
//
//	if (gtk_tree_path_get_depth(path) <= 1)
//		result = true;
//	else if (gtk_tree_path_up(path))
//	{
//		GtkTreeIter iter;
//		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(mTreeStore), &iter, path))
//		{
//			MListRowBase* row;
//			gtk_tree_model_get(GTK_TREE_MODEL(mTreeStore), &iter, GetColumnCount(), &row, -1);
//			
//			if (row != NULL)
//				result = row->RowDropPossible();
//		}
//	}
//	
//	gtk_tree_path_free(path);
//	
//	return result;
//}
//
//gboolean MListBase::DragDataReceivedCallback(
//	GtkTreeDragDest*	inTreeDragDest,
//	GtkTreePath*		inPath,
//	GtkSelectionData*	inSelectionData)
//{
//	bool result = (*MListBase::sSavedDragDataReceived)(inTreeDragDest, inPath, inSelectionData);
//	if (result)
//	{
//		MListBase* self = reinterpret_cast<MListBase*>(g_object_get_data(G_OBJECT(inTreeDragDest), "mlistbase"));
//		if (self != nullptr)
//			result = self->DragDataReceived(inPath, inSelectionData);
//	}
//
//	return result;
//}
//
//bool MListBase::DragDataReceived(
//	GtkTreePath*		inTreePath,
//	GtkSelectionData*	inSelectionData)
//{
//	RowDragged(GetRowForPath(inTreePath));
//	return true;
//}	
//
//void MListBase::CursorChanged()
//{
//	MListRowBase* row = GetCursorRow();
//	
//	if (row != nullptr)
//	{
//		mRowSelectedTimer.Stop();
//		mRowEditingTimedOut.Stop();
//		
//		RowSelected(row);
//		mRowSelectedTimer.Start(0.3);
//	}
//}
//
//void MListBase::RowSelectedTimeOutWaited()
//{
//	for (auto col = mEnabledEditColumns.begin(); col != mEnabledEditColumns.end(); ++col)
//		 g_object_set(G_OBJECT(mRenderers[*col]), "editable", true, nullptr);
//	
//	mRowEditingTimedOut.Start(2.0);
//}
//
//void MListBase::DisableEditingAndTimer()
//{
//	for (auto col = mEnabledEditColumns.begin(); col != mEnabledEditColumns.end(); ++col)
//		 g_object_set(G_OBJECT(mRenderers[*col]), "editable", false, nullptr);
//	mEnabledEditColumns.clear();
//}
//
//void MListBase::RowActivated(
//	GtkTreePath*		inTreePath,
//	GtkTreeViewColumn*	inColumn)
//{
//	mRowSelectedTimer.Stop();
//	mRowEditingTimedOut.Stop();
//
//	mEnabledEditColumns.clear();
//	
//	MListRowBase* row = GetRowForPath(inTreePath);
//	if (row != nullptr)
//		RowActivated(row);
//}
//
//void MListBase::RowChanged(
//	GtkTreePath*		inTreePath,
//	GtkTreeIter*		inTreeIter)
//{
//	MListRowBase* row = GetRowForPath(inTreePath);
//	if (row != nullptr)
//		row->UpdateRowReference(GTK_TREE_MODEL(mTreeStore), inTreePath);
//}
//
//void MListBase::RowDeleted(
//	GtkTreePath*		inTreePath)
//{
//	eRowsReordered();
//}
//
//void MListBase::RowInserted(
//	GtkTreePath*		inTreePath,
//	GtkTreeIter*		inTreeIter)
//{
//	eRowsReordered();
//	
//	MListRowBase* row = GetRowForPath(inTreePath);
//	if (row != nullptr)
//		row->UpdateRowReference(GTK_TREE_MODEL(mTreeStore), inTreePath);
//}
//
//void MListBase::RowsReordered(
//	GtkTreePath*		inTreePath,
//	GtkTreeIter*		inTreeIter,
//	gint*				inNewOrder)
//{
//	eRowsReordered();
//}
//
