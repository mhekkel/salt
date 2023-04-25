////          Copyright Maarten L. Hekkelman 2006-2010
//// Distributed under the Boost Software License, Version 1.0.
////    (See accompanying file LICENSE_1_0.txt or copy at
////          http://www.boost.org/LICENSE_1_0.txt)
//
//#ifndef MLISTVIEW_H
//#define MLISTVIEW_H
//
//#include "MView.hpp"
//#include "MHandler.hpp"
//#include "MColor.hpp"
//
//#include <boost/type_traits/conditional.hpp>
//#include <boost/fusion/support/pair.hpp>
//#include <boost/fusion/include/pair.hpp>
//#include <boost/fusion/sequence.hpp>
//#include <boost/fusion/container/map.hpp>
//#include <boost/fusion/algorithm/iteration/for_each.hpp>
//#include <boost/fusion/include/for_each.hpp>
//#include <boost/fusion/sequence/sequence_facade.hpp>
//#include <boost/fusion/include/sequence_facade.hpp>
//
//// --------------------------------------------------------------------
////
//// There are two versions of MListView here. A simple one using plain
//// old objects. And a templated one. To use the templated one, do this:
////
////	typedef MListViewT<string,int64_t>	CMyList;
////	typedef CMyList::MListRowT			CMyRow;
////
////	const char* labels[] = { "Column 1", "Column 2" };
////	uint32_t widths[] = { 200, 100 };
////
////	AddChild(list = new CMyList("list-1", MRect(...), labels, widths);
////	list->AddRow(new CMyRow("one", 1));
////	list->AddRow(new CMyRow("two", 2));
////
//// --------------------------------------------------------------------
//
//enum MListViewFlags
//{
//	lvNone				= 0,
//	lvHeader			= (1 << 0),
//	lvMultipleSelect	= (1 << 1),
//};
//
//class MListRow
//{
//  public:
//					MListRow();
//	virtual			~MListRow();
//	
//	bool			IsSelected() const				{ return mSelected; }
//	void			SetSelected(bool inSelected)	{ mSelected = inSelected; }
//	
//	uint32_t			GetHeight() const				{ return mHeight; }
//	void			SetHeight(uint32_t inHeight)		{ mHeight = inHeight; }
//	
//	virtual void	GetValue(uint32_t inColumn, int64_t& outValue) const = 0;
//	virtual void	SetValue(uint32_t inColumn, int64_t inValue) = 0;
//
//	virtual void	GetValue(uint32_t inColumn, std::string& outValue) const = 0;
//	virtual void	SetValue(uint32_t inColumn, const std::string& inValue) = 0;
//	
//	virtual void	GetValue(uint32_t inColumn, MColor& outValue) const = 0;
//	virtual void	SetValue(uint32_t inColumn, const MColor& inValue) = 0;
//	
//  protected:
//
//	  bool			mSelected;
//	  uint32_t		mHeight;
//
//  private:
//					MListRow(const MListRow&);
//	MListRow&		operator=(const MListRow&);
//};
//
//class MListColumn
//{
//  public:
//					MListColumn(const std::string& inLabel,
//						uint32_t inWidth);
//	virtual			~MListColumn();
//	
//	virtual void	Render(MDevice& inDevice, const MListRow& inRow,
//						uint32_t inColumnNr, MRect inBounds) = 0;
//
//	virtual uint32_t	CalculateHeight(const MListRow& inRow,
//						uint32_t inColumnNr) = 0;
//
//	std::string		GetLabel() const					{ return mLabel; }
//
//	uint32_t			GetWidth() const					{ return mWidth; }
//	void			SetWidth(uint32_t inWidth)			{ mWidth = inWidth; }
//
//	std::string		GetFont() const						{ return mFont; }
//	void			SetFont(const std::string& inFont)	{ mFont = inFont; }
//	
//	void			SetResizing(bool inResizing)		{ mResizing = inResizing; }
//	bool			IsResizing() const					{ return mResizing; }
//
//  protected:
//	std::string		mLabel;
//	std::string		mFont;
//	uint32_t			mWidth;
//	bool			mResizing;
//
//  private:
//					MListColumn(const MListColumn&);
//	MListColumn&	operator=(const MListColumn&);
//};
//
//class MListTextColumn : public MListColumn
//{
//  public:
//					MListTextColumn(const std::string& inLabel,
//						uint32_t inWidth);
//	
//	virtual void	Render(MDevice& inDevice, const MListRow& inRow,
//						uint32_t inColumnNr, MRect inBounds);	
//
//	virtual uint32_t	CalculateHeight(const MListRow& inRow,
//						uint32_t inColumnNr);	
//};
//
//class MListNumberColumn : public MListTextColumn
//{
//  public:
//					MListNumberColumn(const std::string& inLabel,
//						uint32_t inWidth, const std::string& inFormat);
//	
//	virtual void	Render(MDevice& inDevice, const MListRow& inRow,
//						uint32_t inColumnNr, MRect inBounds);	
//
//  protected:
//	std::string		mFormat;
//};
//
//// A coloured dot (disc). Height is same as width
//class MListDotColumn : public MListColumn
//{
//  public:
//					MListDotColumn(const std::string& inLabel,
//						uint32_t inWidth);
//	
//	virtual void	Render(MDevice& inDevice, const MListRow& inRow,
//						uint32_t inColumnNr, MRect inBounds);	
//
//	virtual uint32_t	CalculateHeight(const MListRow& inRow,
//						uint32_t inColumnNr);	
//};
//
//class MListViewImpl;
//class MListHeader;
//
//class MListView : public MView, public MHandler
//{
//  public:
//					MListView(const std::string& inID,
//						MRect inBounds, MListViewFlags inFlags = lvNone);
//					~MListView();
//
//	virtual bool	HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
//
//	virtual void	ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);
//			
//	virtual void	AddedToWindow();
//
//	virtual void	AddColumn(MListColumn* inColumn);
//	void			SetResizingColumn(uint32_t inColumnNr);
//	MListColumn*	GetColumn(uint32_t inColumnNr);
//	
//	virtual void	AddRow(MListRow* inRow, MListRow* inBefore = nullptr);
//	virtual void	RemoveRow(uint32_t inIndex);
//	virtual void	RemoveRow(MListRow* inRow);
//	virtual void	RemoveAll();
//	
//	virtual void	SelectRow(uint32_t inIndex, bool inSelect = true);
//	virtual void	SelectRow(MListRow* inRow, bool inSelect = true);
//	virtual MListRow*
//					GetFirstSelectedRow() const;
//	virtual MListRow*
//					GetNextSelectedRow(MListRow* inFrom) const;
//
//					// returns -1 if not found:	
//	virtual int32_t	GetRowIndex(MListRow* inRow) const;
//	virtual MListRow*
//					GetRow(uint32_t inIndex);
//
//  protected:
//	friend class MListViewImpl;
//
//	virtual void	EmitRowSelected(MListRow* inRow) {}
//	virtual void	EmitRowInvoked(MListRow* inRow)	{}
//	virtual void	EmitRowDragged(MListRow* inRow) {}
//
//	MListHeader*	mHeader;
//	MListViewImpl*	mImpl;
//	MListViewFlags	mFlags;
//};
//
//// --------------------------------------------------------------------
//// Templated version of MListView
//
//// --------------------------------------------------------------------
//// Column traits, mapping properties to recognized C++ types
//template<class Arg> struct MListColumnTraitsT
//{
//	static MListColumn*	CreateColumn(const std::string& inLabel, uint32_t inWidth)
//	{
//		return nullptr;
//	}
//};
//
//template<> struct MListColumnTraitsT<std::string>
//{
//	static MListColumn*	CreateColumn(const std::string& inLabel, uint32_t inWidth)
//	{
//		return new MListTextColumn(inLabel, inWidth);
//	}
//};
//
//template<> struct MListColumnTraitsT<int64_t>
//{
//	static MListColumn*	CreateColumn(const std::string& inLabel, uint32_t inWidth)
//	{
//		return new MListNumberColumn(inLabel, inWidth, "%Ld");
//	}
//};
//
//template<> struct MListColumnTraitsT<MColor>
//{
//	static MListColumn*	CreateColumn(const std::string& inLabel, uint32_t inWidth)
//	{
//		return new MListDotColumn(inLabel, inWidth);
//	}
//};
//
//// --------------------------------------------------------------------
//// Templated rows, first some magic to access the data
//
//template<class A, int I>
//struct is_valid_index
//{
//	enum { value = boost::fusion::result_of::size<A>::value > I };
//};
//
//template<int I, class A, class V, class E = void>
//struct MValueAccessT
//{
//	static void		get(const A& inValues, V& outValue)		{  }
//	static void		set(A& inValues, const V& inValue)		{  }
//};
//
//template<int I, class A, class V>
//struct MValueAccessT<I, A, V,
//	typename boost::enable_if<typename boost::is_same<typename boost::fusion::result_of::value_at_c<A,I>::type, V>>::type>
//{
//	static void		get(const A& inValues, V& outValue)		{ outValue = boost::fusion::at_c<I>(inValues); }
//	static void		set(A& inValues, const V& inValue)		{ boost::fusion::at_c<I>(inValues) = inValue; }
//};
//
//template<int I, class A, class V, class E = void>
//struct MValueAccess
//{
//	static void		get(const A& inValues, V& outValue)		{  }
//	static void		set(A& inValues, const V& inValue)		{  }
//};
//
//template<int I, class A, class V>
//struct MValueAccess<I, A, V,
//	typename boost::enable_if<is_valid_index<A, I>>::type>
//	: public MValueAccessT<I, A, V> {};
//
//template<class Args>
//class MListRowBaseT : public MListRow
//{
//  public:
//	typedef Args		MListColumnTypes;
//
//	enum { kColumnCount = boost::fusion::result_of::size<Args>::value };
//	
//						MListRowBaseT(const MListColumnTypes& inValues)
//							: mValues(inValues) {}
//
//	virtual void		GetValue(uint32_t inColumn, int64_t& outValue) const;
//	virtual void		SetValue(uint32_t inColumn, int64_t inValue);
//	virtual void		GetValue(uint32_t inColumn, std::string& outValue) const;
//	virtual void		SetValue(uint32_t inColumn, const std::string& inValue);
//	virtual void		GetValue(uint32_t inColumn, MColor& outValue) const;
//	virtual void		SetValue(uint32_t inColumn, const MColor& inValue);
//
//  protected:
//	MListColumnTypes	mValues;
//};
//
//template<class Args>
//void MListRowBaseT<Args>::GetValue(uint32_t inColumn, int64_t& outValue) const
//{
//	switch (inColumn)
//	{
//		case 0:	MValueAccess<0,MListColumnTypes,int64_t>::get(mValues, outValue); break;
//		case 1:	MValueAccess<1,MListColumnTypes,int64_t>::get(mValues, outValue); break;
//		case 2:	MValueAccess<2,MListColumnTypes,int64_t>::get(mValues, outValue); break;
//		case 3:	MValueAccess<3,MListColumnTypes,int64_t>::get(mValues, outValue); break;
//	}
//}
//
//template<class Args>
//void MListRowBaseT<Args>::SetValue(uint32_t inColumn, int64_t inValue)
//{
//	switch (inColumn)
//	{
//		case 0:	MValueAccess<0,MListColumnTypes,int64_t>::set(mValues, inValue); break;
//		case 1:	MValueAccess<1,MListColumnTypes,int64_t>::set(mValues, inValue); break;
//		case 2:	MValueAccess<2,MListColumnTypes,int64_t>::set(mValues, inValue); break;
//		case 3:	MValueAccess<3,MListColumnTypes,int64_t>::set(mValues, inValue); break;
//	}
//}
//
//template<class Args>
//void MListRowBaseT<Args>::GetValue(uint32_t inColumn, std::string& outValue) const
//{
//	switch (inColumn)
//	{
//		case 0:	MValueAccess<0,MListColumnTypes,std::string>::get(mValues, outValue); break;
//		case 1:	MValueAccess<1,MListColumnTypes,std::string>::get(mValues, outValue); break;
//		case 2:	MValueAccess<2,MListColumnTypes,std::string>::get(mValues, outValue); break;
//		case 3:	MValueAccess<3,MListColumnTypes,std::string>::get(mValues, outValue); break;
//	}
//}
//
//template<class Args>
//void MListRowBaseT<Args>::SetValue(uint32_t inColumn, const std::string& inValue)
//{
//	switch (inColumn)
//	{
//		case 0:	MValueAccess<0,MListColumnTypes,std::string>::set(mValues, inValue); break;
//		case 1:	MValueAccess<1,MListColumnTypes,std::string>::set(mValues, inValue); break;
//		case 2:	MValueAccess<2,MListColumnTypes,std::string>::set(mValues, inValue); break;
//		case 3:	MValueAccess<3,MListColumnTypes,std::string>::set(mValues, inValue); break;
//	}
//}
//
//template<class Args>
//void MListRowBaseT<Args>::GetValue(uint32_t inColumn, MColor& outValue) const
//{
//	switch (inColumn)
//	{
//		case 0:	MValueAccess<0,MListColumnTypes,MColor>::get(mValues, outValue); break;
//		case 1:	MValueAccess<1,MListColumnTypes,MColor>::get(mValues, outValue); break;
//		case 2:	MValueAccess<2,MListColumnTypes,MColor>::get(mValues, outValue); break;
//		case 3:	MValueAccess<3,MListColumnTypes,MColor>::get(mValues, outValue); break;
//	}
//}
//
//template<class Args>
//void MListRowBaseT<Args>::SetValue(uint32_t inColumn, const MColor& inValue)
//{
//	switch (inColumn)
//	{
//		case 0:	MValueAccess<0,MListColumnTypes,MColor>::set(mValues, inValue); break;
//		case 1:	MValueAccess<1,MListColumnTypes,MColor>::set(mValues, inValue); break;
//		case 2:	MValueAccess<2,MListColumnTypes,MColor>::set(mValues, inValue); break;
//		case 3:	MValueAccess<3,MListColumnTypes,MColor>::set(mValues, inValue); break;
//	}
//}
//
//template<class Arg0, class Arg1 = void, class Arg2 = void, class Arg3 = void>
//class MListRowT : public MListRowBaseT<boost::fusion::vector<Arg0,Arg1,Arg2,Arg3>>
//{
//  public:
//	typedef typename MListRowBaseT::MListColumnTypes	MListColumnTypes;
//	
//					MListRowT(const Arg0& inValue0, const Arg1& inValue1, const Arg2& inValue2, const Arg3& inValue3)
//						: MListRowBaseT(MListColumnTypes(inValue0, inValue1, inValue2, inValue3)) {}
//};
//
//template<class Arg0, class Arg1, class Arg2>
//class MListRowT<Arg0,Arg1,Arg2,void> : public MListRowBaseT<boost::fusion::vector<Arg0,Arg1,Arg2>>
//{
//  public:
//	typedef typename MListRowBaseT::MListColumnTypes	MListColumnTypes;
//	
//					MListRowT(const Arg0& inValue0, const Arg1& inValue1, const Arg2& inValue2)
//						: MListRowBaseT(MListColumnTypes(inValue0, inValue1, inValue2)) {}
//};
//
//template<class Arg0, class Arg1>
//class MListRowT<Arg0,Arg1,void,void> : public MListRowBaseT<boost::fusion::vector<Arg0,Arg1>>
//{
//  public:
//	typedef typename MListRowBaseT::MListColumnTypes	MListColumnTypes;
//	
//					MListRowT(const Arg0& inValue0, const Arg1& inValue1)
//						: MListRowBaseT(MListColumnTypes(inValue0, inValue1)) {}
//};
//
//template<class Arg0>
//class MListRowT<Arg0,void,void,void> : public MListRowBaseT<boost::fusion::vector<Arg0>>
//{
//  public:
//	typedef typename MListRowBaseT::MListColumnTypes	MListColumnTypes;
//	
//					MListRowT(const Arg0& inValue0)
//						: MListRowBaseT(MListColumnTypes(inValue0)) {}
//};
//
//// --------------------------------------------------------------------
//
//template<class Arg0, class Arg1 = void, class Arg2 = void, class Arg3 = void>
//class MListViewT : public MListView
//{
//  public:
//	typedef typename MListRowT<Arg0,Arg1,Arg2,Arg3>		MListRowT;
//	static const uint32_t									kColumnCount = MListRowT::kColumnCount;
//
//					MListViewT(const std::string& inID, MRect inBounds,
//						const char* inLabels[kColumnCount], uint32_t inWidths[kColumnCount],
//							MListViewFlags inFlags = lvNone)
//						: MListView(inID, inBounds, inFlags)
//					{
//						std::copy(inLabels, inLabels + kColumnCount, mLabels);
//						std::copy(inWidths, inWidths + kColumnCount, mWidths);
//					}
//
//	virtual void	AddedToWindow();
//
//	MEventOut<void(MListRowT*)>	eRowSelected;
//	MEventOut<void(MListRowT*)>	eRowInvoked;
//	MEventOut<void(MListRowT*)>	eRowDragged;
//
//  protected:
//
//	virtual void	EmitRowSelected(MListRow* inRow)	{ eRowSelected(static_cast<MListRowT*>(inRow)); }
//	virtual void	EmitRowInvoked(MListRow* inRow)		{ eRowInvoked(static_cast<MListRowT*>(inRow)); }
//	virtual void	EmitRowDragged(MListRow* inRow)		{ eRowDragged(static_cast<MListRowT*>(inRow)); }
//
//	const char*		mLabels[kColumnCount];
//	uint32_t			mWidths[kColumnCount];
//};
//
//template<class Arg0, class Arg1, class Arg2, class Arg3>
//void MListViewT<Arg0, Arg1, Arg2, Arg3>::AddedToWindow()
//{
//	MListView::AddedToWindow();
//
//	AddColumn(MListColumnTraitsT<Arg0>::CreateColumn(mLabels[0], mWidths[0]));
//	if (kColumnCount > 1)
//		AddColumn(MListColumnTraitsT<Arg1>::CreateColumn(mLabels[1], mWidths[1]));
//	if (kColumnCount > 2)
//		AddColumn(MListColumnTraitsT<Arg2>::CreateColumn(mLabels[2], mWidths[2]));
//	if (kColumnCount > 3)
//		AddColumn(MListColumnTraitsT<Arg3>::CreateColumn(mLabels[3], mWidths[3]));
//}
//
//
//#endif
