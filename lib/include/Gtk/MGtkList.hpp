//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <vector>
#include <set>

#include "MView.hpp"
#include "MP2PEvents.hpp"
#include "MError.hpp"
#include "MTimer.hpp"

template<class R>
class MList;
class MListBase;
class MListColumnEditedListener;

typedef std::tuple<GtkCellRenderer*,const char*,bool,bool,bool>	MGtkRendererInfo;

template<typename T>
struct g_type_mapped
{
	typedef T					value_type;
	static const GType g_type = G_TYPE_STRING;
	enum { can_toggle = false, can_edit = false, can_change = false };
	static MGtkRendererInfo	GetRenderer()
		{ return MGtkRendererInfo(gtk_cell_renderer_text_new(), "text", can_toggle, can_edit, can_change); }
	static void to_g_value(GValue& gv, T v)
	{
		g_value_set_string(&gv, std::to_string(v).c_str());
	}
};

template<> struct g_type_mapped<std::string>
{
	typedef std::string			value_type;
	static const GType g_type = G_TYPE_STRING;
	enum { can_toggle = false, can_edit = true, can_change = false };
	static MGtkRendererInfo	GetRenderer()
		{ return MGtkRendererInfo(gtk_cell_renderer_text_new(), "text", can_toggle, can_edit, can_change); }
	static void to_g_value(GValue& gv, const std::string& s)
	{
		g_value_set_string(&gv, s.c_str());
	}
};
template<> struct g_type_mapped<std::vector<std::string>>
{
	typedef std::string			value_type;
	static const GType g_type = G_TYPE_STRING;
	enum { can_toggle = false, can_edit = true, can_change = true };
	static MGtkRendererInfo	GetRenderer()
		{ return MGtkRendererInfo(gtk_cell_renderer_combo_new(), "text", can_toggle, can_edit, can_change); }
	static void to_g_value(GValue& gv, const std::string& s)
	{
		g_value_set_string(&gv, s.c_str());
	}
	static GtkListStore* get_options();
};
template<> struct g_type_mapped<bool>
{
	typedef bool				value_type;
	static const GType g_type = G_TYPE_BOOLEAN;
	enum { can_toggle = true, can_edit = false, can_change = false };
	static MGtkRendererInfo	GetRenderer()
		{ return MGtkRendererInfo(gtk_cell_renderer_toggle_new(), "active", can_toggle, can_edit, can_change); }
	static void to_g_value(GValue& gv, bool v)
	{
		g_value_set_boolean(&gv, v);
	}
};
template<> struct g_type_mapped<GdkPixbuf*>
{
	typedef GdkPixbuf*			value_type;
	static const GType g_type = G_TYPE_OBJECT;
	enum { can_toggle = false, can_edit = false, can_change = false };
	static MGtkRendererInfo	GetRenderer()
		{ return MGtkRendererInfo(gtk_cell_renderer_pixbuf_new(), "pixbuf", can_toggle, can_edit, can_change); }
	static void to_g_value(GValue& gv, GdkPixbuf* pixbuf)
	{
		g_value_set_object(&gv, pixbuf);
	}
};

//---------------------------------------------------------------------
// MListRow

class MListRowBase : public boost::noncopyable
{
	friend class MListBase;
	friend class MListColumnEditedListener;

  public:
						MListRowBase();
	virtual				~MListRowBase();
	
	virtual bool		RowDropPossible() const		{ return false; }

	// call this to update the contents in the list:
	virtual void		RowChanged();

  protected:

	bool				GetModelAndIter(GtkTreeStore*& outTreeStore, GtkTreeIter& outTreeIter);

	virtual void		UpdateDataInTreeStore() = 0;

	bool				GetParentAndPosition(MListRowBase*& outParent, uint32_t& outPosition, uint32_t inObjectColumn);

	void				UpdateRowReference(GtkTreeModel* inNewModel, GtkTreePath* inNewPath);

	virtual void		ColumnEdited(uint32_t inColumnNr, const std::string& inNewText)		{}

	virtual void		ColumnToggled(uint32_t inColumnNr)		{}

  private:

	GtkTreePath*		GetTreePath() const;

	GtkTreeRowReference*
						mRowReference;
};

template<class I, typename... Args>
class MListRow : public MListRowBase
{
  public:

	typedef I							impl_type;

  private:
	friend class MList<MListRow>;

	typedef 		MGtkRendererInfo					renderer_info;
	
	template<class base_type, class N, class T, int P>
	struct column_type
	{
		typedef typename base_type::template type<P-1>	base_column_type;
		typedef typename base_column_type::name_type	name_type;
		typedef typename base_column_type::value_type	value_type;
	};
	
	template<class base_type, class N, class T>
	struct column_type<base_type,N,T,0>
	{
		typedef N										name_type;
		typedef T										value_type;
	};
	
	template<class... Args1>
	struct column_type_traits
	{
		static GType	GetGType(int inColumnNr)			{ THROW(("error")); }

		static renderer_info
						GetRenderer(int inColumnNr)			{ THROW(("error")); }

		static void		GetGValue(impl_type* inProvider, int inColumnNr, GValue& outValue)			{ THROW(("error")); }
	};
	
	template<class N, class T, class... Args1>
	struct column_type_traits<N, T, Args1...>
	{
		static const int count = 1 + sizeof...(Args1) / 2;
		typedef column_type_traits<Args1...>	base_type;
		
		template<int P>
		struct type : public column_type<base_type,N,T,P>	{};
		
		static GType	GetGType(int inColumnNr)
						{
							GType result;
							if (inColumnNr == 0)
								result = g_type_mapped<T>::g_type;
							else
								result = base_type::GetGType(inColumnNr - 1);
							return result;
						}

		static renderer_info
						GetRenderer(int inColumnNr)
						{
							MGtkRendererInfo result;
							if (inColumnNr == 0)
								result = g_type_mapped<T>::GetRenderer();
							else
								result = base_type::GetRenderer(inColumnNr - 1);
							return result;
						}

		static void		GetGValue(impl_type* inProvider, int inColumnNr, GValue& outValue)
						{
							if (inColumnNr == 0)
							{
								typename g_type_mapped<T>::value_type v;
								inProvider->GetData(N(), v);
								g_value_init(&outValue, g_type_mapped<T>::g_type);
								g_type_mapped<T>::to_g_value(outValue, v);
							}
							else
								base_type::GetGValue(inProvider, inColumnNr - 1, outValue);
						}
	};

  protected:

	virtual void		UpdateDataInTreeStore()
						{
							GtkTreeStore* treeStore;
							GtkTreeIter treeIter;
							
							if (GetModelAndIter(treeStore, treeIter))
							{
								std::vector<GValue> v(column_count + 1);
								std::vector<gint> c(column_count + 1);
							
								for (int j = 0; j < column_count; ++j)
								{
									c[j] = j;
									traits::GetGValue(static_cast<impl_type*>(this), j, v[j]);
								}
								c[column_count] = column_count;
								
								g_value_init(&v[column_count], G_TYPE_POINTER);
								g_value_set_pointer(&v[column_count], this);
								
								gtk_tree_store_set_valuesv(treeStore, &treeIter, &c[0], &v[0], column_count + 1);
							}
						}

  public:

	typedef column_type_traits<Args...>	traits;
	static const int					column_count = traits::count;

	using MListRowBase::GetParentAndPosition;

	bool				GetParentAndPosition(impl_type*& outParent, uint32_t& outPosition)
						{
							bool result = false;
							MListRowBase* parent;
							
							if (GetParentAndPosition(parent, outPosition, column_count))
							{
								outParent = static_cast<impl_type*>(parent);
								result = true;
							}
							
							return result;
						}
};

//---------------------------------------------------------------------
// MList

class MListBase : public MView
{
	friend class MListColumnEditedListener;
	
  public:
					MListBase(GtkWidget* inTreeView);

	virtual			~MListBase();

	void			AllowMultipleSelectedItems();

	void			SetColumnTitle(uint32_t inColumnNr, const std::string& inTitle);

	void			SetColumnAlignment(uint32_t inColumnNr, float inAlignment);		

	void			SetExpandColumn(uint32_t inColumnNr);		

	void			SetColumnEditable(uint32_t inColumnNr, bool inEditable);

	void			SetColumnToggleable(uint32_t inColumnNr, bool inToggleable);

					// turn column into a popup/combo item
	void			SetListOfOptionsForColumn(uint32_t inColumnNr, const std::vector<std::string>& inOptions);

	void			SelectRowAndStartEditingColumn(MListRowBase* inRow, uint32_t inColumnNr);

	virtual MListRowBase*
					GetCursorRow() const;

	void			GetSelectedRows(std::list<MListRowBase*>& outRows) const;

	void			SelectRow(MListRowBase* inRow);

	void			CollapseRow(MListRowBase* inRow);
	
	void			ExpandRow(MListRowBase* inRow, bool inExpandAll);

	void			CollapseAll();
	
	void			ExpandAll();

	virtual int		GetColumnCount() const = 0;

	virtual void	AppendRow(MListRowBase* inRow, MListRowBase* inParent);

	virtual void	InsertRow(MListRowBase* inRow, MListRowBase* inBefore);

	virtual void	RemoveRow(MListRowBase* inRow);

	virtual void	RemoveAll();

	MEventOut<void()>
					eRowsReordered;

  protected:

	void			CreateTreeStore(std::vector<GType>& inTypes, std::vector<MGtkRendererInfo>& inRenderers);

	GtkTreePath*	GetTreePathForRow(MListRowBase* inRow);

	bool			GetTreeIterForRow(MListRowBase* inRow, GtkTreeIter* outIter);

	static gboolean	RemoveAllCB(GtkTreeModel* inModel, GtkTreePath* inPath, GtkTreeIter* inIter, gpointer inData);

	MListRowBase*	GetRowForPath(GtkTreePath* inPath) const;

	virtual void	CursorChanged();
	MSlot<void()>	mCursorChanged;
	virtual void	RowSelected(MListRowBase* inRow) = 0;

	virtual void	RowActivated(GtkTreePath* inTreePath, GtkTreeViewColumn* inColumn);
	MSlot<void(GtkTreePath*,GtkTreeViewColumn*)>
					mRowActivated;
	virtual void	RowActivated(MListRowBase* inRow) = 0;

	virtual void	RowChanged(GtkTreePath* inTreePath, GtkTreeIter* inTreeIter);
	MSlot<void(GtkTreePath*,GtkTreeIter*)>
					mRowChanged;

	virtual void	RowDeleted(GtkTreePath* inTreePath);
	MSlot<void(GtkTreePath*)>
					mRowDeleted;

	virtual void	RowInserted(GtkTreePath* inTreePath, GtkTreeIter* inTreeIter);
	MSlot<void(GtkTreePath*,GtkTreeIter*)>
					mRowInserted;
	
	virtual void	RowsReordered(GtkTreePath* inTreePath, GtkTreeIter* inTreeIter, gint* inNewOrder);
	MSlot<void(GtkTreePath*,GtkTreeIter*,gint*)>
					mRowsReordered;
	
	// tree store overrides
	
	typedef gboolean (*RowDropPossibleFunc)(GtkTreeDragDest*, GtkTreePath*, GtkSelectionData*);
	static gboolean RowDropPossibleCallback(GtkTreeDragDest*, GtkTreePath*, GtkSelectionData*);
	
	static RowDropPossibleFunc
					sSavedRowDropPossible;

	virtual bool	RowDropPossible(GtkTreePath* inTreePath, GtkSelectionData* inSelectionData);

	typedef gboolean (*DragDataReceivedFunc)(GtkTreeDragDest*, GtkTreePath*, GtkSelectionData*);
	static gboolean DragDataReceivedCallback(GtkTreeDragDest*, GtkTreePath*, GtkSelectionData*);

	static DragDataReceivedFunc
					sSavedDragDataReceived;

	virtual bool	DragDataReceived(GtkTreePath* inTreePath, GtkSelectionData* inSelectionData);

	virtual void	RowDragged(MListRowBase* inRow) = 0;

	GtkTreeStore*	mTreeStore;
	std::vector<GtkCellRenderer*>
					mRenderers;
	std::vector<MListColumnEditedListener*>
					mListeners;
	
	void			RowSelectedTimeOutWaited();
	void			DisableEditingAndTimer();
 	
	MTimeOut		mRowSelectedTimer;
	MTimeOut		mRowEditingTimedOut;
	std::set<uint32_t>
					mEnabledEditColumns;
};

template<class R>
class MList : public MListBase
{
  private:
	typedef R								row_type;
	typedef std::list<row_type*>			row_list;
	typedef typename row_type::traits		column_type_traits;
	static const int						column_count = column_type_traits::count;
	
  public:

					MList(GtkWidget* inWidget);

	void			AppendRow(row_type* inRow, row_type* inParentRow = nullptr)
														{ MListBase::AppendRow(inRow, inParentRow); }

	virtual void	InsertRow(row_type* inRow, row_type* inBefore = nullptr)
														{ MListBase::InsertRow(inRow, inBefore); }

	virtual row_type*
					GetCursorRow() const				{ return static_cast<row_type*>(MListBase::GetCursorRow()); }
	
	row_list		GetSelectedRows() const
					{
						std::list<MListRowBase*> selected;
						MListBase::GetSelectedRows(selected);

						row_list result;
						for (auto s = selected.begin(); s != selected.end(); ++s)
							result.push_back(static_cast<row_type*>(*s));
						
						return result;
					}

	virtual int		GetColumnCount() const				{ return column_type_traits::count; }

	MEventOut<void(row_type*)>				eRowSelected;
	MEventOut<void(row_type*)>				eRowInvoked;
	MEventOut<void(row_type*)>				eRowDragged;
	
  protected:
	
	virtual void	RowSelected(MListRowBase* inRow)		{ eRowSelected(static_cast<row_type*>(inRow)); }

	virtual void	RowActivated(MListRowBase* inRow)		{ eRowInvoked(static_cast<row_type*>(inRow)); }

	virtual void	RowDragged(MListRowBase* inRow)		{ eRowDragged(static_cast<row_type*>(inRow)); }
};

template<typename R>
MList<R>::MList(GtkWidget* inTreeView)
	: MListBase(inTreeView)
{
    std::vector<GType> types(GetColumnCount() + 1);
    std::vector<MGtkRendererInfo> renderers(GetColumnCount());
    
    for (int i = 0; i < GetColumnCount(); ++i)
    {
    	types[i] = column_type_traits::GetGType(i);
    	renderers[i] = column_type_traits::GetRenderer(i);
    }
    types[GetColumnCount()] = G_TYPE_POINTER;
    
    CreateTreeStore(types, renderers);
}
