//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

//#include <gdk/gdkkeysyms.h>

#include <iostream>
#include <algorithm>
#include <cstring>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include <zeep/xml/document.hpp>

#include "MMenuImpl.hpp"
#include "MWindow.hpp"
#include "MAcceleratorTable.hpp"
#include "MFile.hpp"
#include "MStrings.hpp"
#include "MResources.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
//#include "MX11WidgetMixin.hpp"
#include "MX11WindowImpl.hpp"

using namespace std;
namespace xml = zeep::xml;
namespace io = boost::iostreams;

namespace
{

struct MCommandToString
{
	char mCommandString[10];
	
	MCommandToString(
		uint32_t			inCommand)
	{
		strcpy(mCommandString, "MCmd_xxxx");
		
		mCommandString[5] = ((inCommand & 0xff000000) >> 24) & 0x000000ff;
		mCommandString[6] = ((inCommand & 0x00ff0000) >> 16) & 0x000000ff;
		mCommandString[7] = ((inCommand & 0x0000ff00) >>  8) & 0x000000ff;
		mCommandString[8] = ((inCommand & 0x000000ff) >>  0) & 0x000000ff;
	}
	
	operator const char*() const	{ return mCommandString; }
};

//struct MRecentItems
//{
//	static MRecentItems&
//						Instance();
//
//						operator GtkRecentManager* ()	{ return mRecentMgr; }
//
//  private:
//	
//						MRecentItems();
//						~MRecentItems();
//
//	GtkRecentManager*	mRecentMgr;
//};
//
//MRecentItems& MRecentItems::Instance()
//{
//	static MRecentItems sInstance;
//	return sInstance;
//}
//
//MRecentItems::MRecentItems()
//{
//	mRecentMgr = gtk_recent_manager_get_default();
//}
//
//MRecentItems::~MRecentItems()
//{
//	g_object_unref(mRecentMgr);
//}

}

//// --------------------------------------------------------------------
//// MMenuItem
//
//struct MMenuItem;
//typedef list<MMenuItem*> MMenuItemList;
//
//struct MMenuItem
//{
//  public:
//	MMenuItem(MMenu* inMenu, const string& inLabel, uint32_t inCommand);
//
//	void CreateWidget();						// plain, simple item
//	void CreateWidget(GSList*& ioRadioGroup);	// radio menu item
//	void CreateCheckWidget();
//	void SetChecked(bool inChecked);
//	void ItemCallback();
//	void ItemToggled();
////	void RecentItemActivated();
//
//	MSlot<void()>	mCallback;
////	MSlot<void()>	mRecentItemActivated;
//
//	GtkWidget*		mGtkMenuItem;
//	string			mLabel;
//	uint32_t			mCommand;
//	uint32_t			mIndex;
//	MMenu*			mMenu;
//	MMenu*			mSubMenu;
//	bool			mEnabled;
//	bool			mCanCheck;
//	bool			mChecked;
//	bool			mInhibitCallBack;
//};
//
//MMenuItem::MMenuItem(MMenu* inMenu, const string& inLabel, uint32_t inCommand)
//	: mCallback(this, &MMenuItem::ItemCallback)
////	, mRecentItemActivated(this, &MMenuItem::RecentItemActivated)
//	, mGtkMenuItem(nullptr)
//	, mLabel(inLabel)
//	, mCommand(inCommand)
//	, mIndex(0)
//	, mMenu(inMenu)
//	, mSubMenu(nullptr)
//	, mEnabled(true)
//	, mCanCheck(false)
//	, mChecked(false)
//	, mInhibitCallBack(false)
//{
//}
//
//void MMenuItem::CreateWidget()
//{
//	if (mLabel == "-")
//		mGtkMenuItem = gtk_separator_menu_item_new();
//	else
//	{
//		mGtkMenuItem = gtk_menu_item_new_with_label(_(mLabel.c_str()));
//		if (mCommand != 0)	
//			mCallback.Connect(mGtkMenuItem, "activate");
//	}
//}
//
//void MMenuItem::CreateWidget(GSList*& ioRadioGroup)
//{
//	mGtkMenuItem = gtk_radio_menu_item_new_with_label(ioRadioGroup, _(mLabel.c_str()));
//
//	ioRadioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mGtkMenuItem));
//
//	if (mCommand != 0)
//		mCallback.Connect(mGtkMenuItem, "toggled");
//
//	mCanCheck = true;
//}
//
//void MMenuItem::CreateCheckWidget()
//{
//	mGtkMenuItem = gtk_check_menu_item_new_with_label(_(mLabel.c_str()));
//
//	if (mCommand != 0)
//		mCallback.Connect(mGtkMenuItem, "toggled");
//
////	mCanCheck = true;
//}
//
//void MMenuItem::ItemCallback()
//{
//	try
//	{
//		if (mMenu != nullptr and
//			mMenu->GetTarget() != nullptr and
//			not mInhibitCallBack)
//		{
//			bool process = true;
//			
//			if (mCanCheck)
//			{
//				mChecked = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mGtkMenuItem));
//				process = mChecked;
//			}
//
//			GdkModifierType modifiers;
//			gdk_window_get_pointer(mGtkMenuItem->window, nullptr, nullptr, &modifiers);
//			
//			MHandler* target = mMenu->GetTarget();
//			MWindow* window = dynamic_cast<MWindow*>(target);
//			if (window != nullptr)
//				target = window->FindFocus();
//
//			if (process and not target->ProcessCommand(mCommand, mMenu, mIndex, modifiers))
//				PRINT(("Unhandled command: %s", (const char*)MCommandToString(mCommand)));
//		}
//	}
//	catch (exception& e)
//	{
//		DisplayError(e);
//	}
//	catch (...) {}
//}
//
//void MMenuItem::SetChecked(bool inChecked)
//{
//	if (inChecked != mChecked and GTK_IS_CHECK_MENU_ITEM(mGtkMenuItem))
//	{
//		mInhibitCallBack = true;
//		mChecked = inChecked;
//		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mGtkMenuItem), mChecked);
//		mInhibitCallBack = false;
//	}
//}
//
//// --------------------------------------------------------------------
//// MX11MenuImpl
//
//class MX11MenuImpl : public MMenuImpl
//{
//  public:
//	MX11MenuImpl(MMenu* inMenu);
//
//	virtual void SetTarget(MHandler* inHandler);
//	virtual void SetItemState(uint32_t inItem, bool inEnabled, bool inChecked);
//	virtual void AppendItem(const string& inLabel, uint32_t inCommand);
//	virtual void AppendSubmenu(MMenu* inSubmenu);
//	virtual void AppendSeparator();
//	virtual void AppendCheckbox(const string& inLabel, uint32_t inCommand);
//	virtual void AppendRadiobutton(const string& inLabel, uint32_t inCommand);
//	virtual uint32_t CountItems() const;
//	virtual void RemoveItems(uint32_t inFirstIndex, uint32_t inCount);
//	virtual string GetItemLabel(uint32_t inIndex) const;
//	virtual void SetItemCommand(uint32_t inIndex, uint32_t inCommand);
//	virtual uint32_t GetItemCommand(uint32_t inIndex) const;
//	virtual MMenu* GetSubmenu(uint32_t inIndex) const;
//	virtual void Popup(MWindow* inHandler, int32_t inX, int32_t inY, bool inBottomMenu);
//	virtual void AddToWindow(MWindowImpl* inWindow);
//	virtual void MenuUpdated();
//
//	void SetAcceleratorGroup(GtkAccelGroup* inAcceleratorGroup);
//
//	virtual bool OnDestroy();
//	virtual void OnSelectionDone();
//
//	MSlot<bool()>	mOnDestroy;
//	MSlot<void()>	mOnSelectionDone;
//
//  protected:
//
//	MMenuItem* CreateNewItem(const std::string& inLabel, uint32_t inCommand, GSList** ioRadioGroup);
//	
//	// for the menubar
//	MX11MenuImpl(MMenu* inMenu, GtkWidget* inWidget)
//		: MX11MenuImpl(inMenu)
//	{
//		mGtkMenu = inWidget;
//	}
//
//	GtkWidget*		mGtkMenu;
//	GtkAccelGroup*	mGtkAccel;
//	std::string		mLabel;
//	MMenuItemList	mItems;
//	MHandler*		mTarget;
//	GSList*			mRadioGroup;
//	int32_t			mPopupX, mPopupY;
//};
//
//MX11MenuImpl::MX11MenuImpl(MMenu* inMenu)
//	: MMenuImpl(inMenu)
//	, mOnDestroy(this, &MX11MenuImpl::OnDestroy)
//	, mOnSelectionDone(this, &MX11MenuImpl::OnSelectionDone)
//	, mGtkMenu(gtk_menu_new())
//	, mGtkAccel(nullptr)
//	, mTarget(nullptr)
//	, mRadioGroup(nullptr)
//{
//}
//
//void MX11MenuImpl::SetTarget(MHandler* inHandler)
//{
//	mTarget = inHandler;
//
//	for (auto mi: mItems)
//	{
//		if (mi->mSubMenu != nullptr)
//			mi->mSubMenu->SetTarget(inHandler);
//	}
//}
//
//void MX11MenuImpl::SetItemState(uint32_t inIndex, bool inEnabled, bool inChecked)
//{
//	if (inIndex >= mItems.size())
//		THROW(("Item index out of range"));
//	
//	MMenuItemList::iterator item = mItems.begin();
//	advance(item, inIndex);
//
//	if (inEnabled != (*item)->mEnabled)
//	{
//		gtk_widget_set_sensitive((*item)->mGtkMenuItem, inEnabled);
//		(*item)->mEnabled = inEnabled;
//	}
//	
//	if ((*item)->mCanCheck)
//		(*item)->SetChecked(inChecked);
//}
//
//MMenuItem* MX11MenuImpl::CreateNewItem(const string& inLabel, uint32_t inCommand, GSList** ioRadioGroup)
//{
//	MMenuItem* item = new MMenuItem(mMenu, inLabel, inCommand);
//
//	if (ioRadioGroup != nullptr)
//		item->CreateWidget(*ioRadioGroup);
//	else
//	{
//		item->CreateWidget();
//		
//		if (inLabel != "-")
//			mRadioGroup = nullptr;
//	}
//
//	item->mIndex = mItems.size();
//	mItems.push_back(item);
//	gtk_menu_shell_append(GTK_MENU_SHELL(mGtkMenu), item->mGtkMenuItem);
//	
//	return item;
//}
//
//void MX11MenuImpl::AppendItem(const string& inLabel, uint32_t inCommand)
//{
//	CreateNewItem(inLabel, inCommand, nullptr);
//}
//
//void MX11MenuImpl::AppendSubmenu(MMenu* inSubmenu)
//{
//	MMenuItem* item = CreateNewItem(inSubmenu->GetLabel(), 0, nullptr);
//	item->mSubMenu = inSubmenu;
//
////	if (inSubmenu->IsRecentMenu())
////		item->mRecentItemActivated.Connect(inSubmenu->GetGtkMenu(), "item-activated");
//
//	MX11MenuImpl* subImpl = dynamic_cast<MX11MenuImpl*>(inSubmenu->impl());	
//
//	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item->mGtkMenuItem), subImpl->mGtkMenu);
//	
//	if (mGtkAccel)
//		subImpl->SetAcceleratorGroup(mGtkAccel);
//}
//
//void MX11MenuImpl::AppendSeparator()
//{
//	CreateNewItem("-", 0, nullptr);
//}
//
//void MX11MenuImpl::AppendCheckbox(const string& inLabel, uint32_t inCommand)
//{
//}
//
//void MX11MenuImpl::AppendRadiobutton(const string& inLabel, uint32_t inCommand)
//{
//}
//
//uint32_t MX11MenuImpl::CountItems() const
//{
//	return mItems.size();
//}
//
//void MX11MenuImpl::RemoveItems(uint32_t inFirstIndex, uint32_t inCount)
//{
//	if (inFirstIndex < mItems.size())
//	{
//		MMenuItemList::iterator b = mItems.begin();
//		advance(b, inFirstIndex);
//
//		if (inFirstIndex + inCount > mItems.size())
//			inCount = mItems.size() - inFirstIndex;
//
//		MMenuItemList::iterator e = b;
//		advance(e, inCount);	
//		
//		for (MMenuItemList::iterator mi = b; mi != e; ++mi)
//			gtk_widget_destroy((*mi)->mGtkMenuItem);
//		
//		mItems.erase(b, e);
//	}
//}
//
//string MX11MenuImpl::GetItemLabel(uint32_t inIndex) const
//{
//	if (inIndex >= mItems.size())
//		THROW(("Item index out of range"));
//	
//	MMenuItemList::const_iterator i = mItems.begin();
//	advance(i, inIndex);
//	
//	return (*i)->mLabel;
//}
//
//void MX11MenuImpl::SetItemCommand(uint32_t inIndex, uint32_t inCommand)
//{
//	if (inIndex >= mItems.size())
//		THROW(("Item index out of range"));
//	
//	MMenuItemList::iterator i = mItems.begin();
//	advance(i, inIndex);
//	
//	(*i)->mCommand = inCommand;
//}
//
//uint32_t MX11MenuImpl::GetItemCommand(uint32_t inIndex) const
//{
//	if (inIndex >= mItems.size())
//		THROW(("Item index out of range"));
//	
//	MMenuItemList::const_iterator i = mItems.begin();
//	advance(i, inIndex);
//	
//	return (*i)->mCommand;
//}
//
//MMenu* MX11MenuImpl::GetSubmenu(uint32_t inIndex) const
//{
//	if (inIndex >= mItems.size())
//		THROW(("Item index out of range"));
//	
//	MMenuItemList::const_iterator i = mItems.begin();
//	advance(i, inIndex);
//	
//	return (*i)->mSubMenu;
//}
//
//void MX11MenuImpl::Popup(MWindow* inHandler, int32_t inX, int32_t inY, bool inBottomMenu)
//{
//}
//
//void MX11MenuImpl::AddToWindow(MWindowImpl* inWindowImpl)
//{
//	MX11WindowImpl* impl = dynamic_cast<MX11WindowImpl*>(inWindowImpl);
//	impl->AddMenubarWidget(mGtkMenu);
//}
//
//void MX11MenuImpl::MenuUpdated()
//{
//	gtk_widget_show_all(mGtkMenu);
//}
//
//bool MX11MenuImpl::OnDestroy()
//{
//	return false;
//}
//
//void MX11MenuImpl::OnSelectionDone()
//{
//}
//
//void MX11MenuImpl::SetAcceleratorGroup(GtkAccelGroup* inAcceleratorGroup)
//{
//	MAcceleratorTable& at = MAcceleratorTable::Instance();
//	
//	gtk_menu_set_accel_group(GTK_MENU(mGtkMenu), inAcceleratorGroup);
//	
//	for (auto& item : mItems)
//	{
//		uint32_t key, mod;
//		
//		if (at.GetAcceleratorKeyForCommand(item->mCommand, key, mod))
//		{
//			int m = 0;
//			
//			if (mod & kShiftKey)	m |= GDK_SHIFT_MASK;
//			if (mod & kControlKey)	m |= GDK_CONTROL_MASK;
//			if (mod & kOptionKey)	m |= GDK_MOD1_MASK;
//			
//			switch (key)
//			{
//				case kTabKeyCode:	key = GDK_KEY_Tab; break;
//				case kF3KeyCode:	key = GDK_KEY_F3; break;
//				default: break;
//			}
//			
//			gtk_widget_add_accelerator(item->mGtkMenuItem, "activate", inAcceleratorGroup,
//				key, GdkModifierType(m), GTK_ACCEL_VISIBLE);
//		}
//		
//		if (item->mSubMenu != nullptr)
//		{
//			MX11MenuImpl* impl = dynamic_cast<MX11MenuImpl*>(item->mSubMenu->impl());
//			if (impl != nullptr)
//				impl->SetAcceleratorGroup(inAcceleratorGroup);
//		}
//	}
//}

MMenuImpl* MMenuImpl::Create(MMenu* inMenu, bool inPopup)
{
//	return new MX11MenuImpl(inMenu);
	return nullptr;
}

//// --------------------------------------------------------------------
//
//class MX11MenuBarImpl : public MX11MenuImpl
//{
//  public:
//	MX11MenuBarImpl(MMenu* inMenu)
//		: MX11MenuImpl(inMenu, gtk_menu_bar_new())
//		, mOnButtonPressEvent(this, &MX11MenuBarImpl::OnButtonPress)
//	{
//		mGtkAccel = gtk_accel_group_new();
//		mOnButtonPressEvent.Connect(mGtkMenu, "button-press-event");
//	}
//
//	bool OnButtonPress(GdkEventButton* inEvent);
//	MSlot<bool(GdkEventButton*)> mOnButtonPressEvent;
//};

MMenuImpl* MMenuImpl::CreateBar(MMenu* inMenu)
{
//	return new MX11MenuBarImpl(inMenu);
	return nullptr;
}

//bool MX11MenuBarImpl::OnButtonPress(GdkEventButton* inEvent)
//{
//	mMenu->UpdateCommandStatus();
//
////	gtk_widget_show_all(mGtkMenu);
//
//	return false;
//}
//
////{
////  public:
////					MMenu(const string& inLabel, GtkWidget* inMenuWidget = nullptr);
////
////	virtual			~MMenu();
////
////	static void		AddToRecentMenu(const MFile& inFileRef);
////
////	static MMenu*	CreateFromResource(const char* inResourceName);
////
////	void			AppendItem(const std::string& inLabel, uint32_t inCommand);
////	
////	void			AppendRadioItem(const std::string& inLabel, uint32_t inCommand);
////	
////	void			AppendCheckItem(const std::string& inLabel, uint32_t inCommand);
////	
////	void			AppendSeparator();
////
////	virtual void	AppendMenu(MMenu* inMenu);
////
////	uint32_t			CountItems();
////	
////	void			RemoveItems(uint32_t inFromIndex, uint32_t inCount);
////
////	std::string		GetItemLabel(uint32_t inIndex) const;
////
////	bool			GetRecentItem(uint32_t inIndex, MFile& outURL) const;
////
////	void			SetTarget(MHandler* inHandler);
////
////	void			UpdateCommandStatus();
////
////	GtkWidget*		GetGtkMenu()			{ return mGtkMenu; }
////	std::string		GetLabel()				{ return mLabel; }
////	MHandler*		GetTarget()				{ return mTarget; }
////	
////	void			SetAcceleratorGroup(GtkAccelGroup* inAcceleratorGroup);
////
////	void			Popup(MHandler* inTarget, GdkEventButton* inEvent, int32_t inX, int32_t inY, bool inBottomMenu);
////	
////	bool			IsRecentMenu() const;
////	
////  protected:
////
////	static MMenu*	Create(zeep::xml::element* inXMLNode);
////
////	static void		MenuPosition(GtkMenu* inMenu, gint* inX, gint* inY, gboolean* inPushIn, gpointer inUserData);
////
////	MMenuItem*		CreateNewItem(const std::string& inLabel, uint32_t inCommand, GSList** ioRadioGroup);
////
////	virtual bool	OnDestroy();
////	virtual void	OnSelectionDone();
////
////	MSlot<bool()>	mOnDestroy;
////	MSlot<void()>	mOnSelectionDone;
////
////	GtkWidget*		mGtkMenu;
////	std::string		mLabel;
////	MMenuItemList	mItems;
////	MHandler*		mTarget;
////	GSList*			mRadioGroup;
////	int32_t			mPopupX, mPopupY;
////};
////
////class MMenubar
////{
////  public:
////					MMenubar(MHandler* inTarget);
////
////	void			AddMenu(MMenu* inMenu);
////
////	void			Initialize(GtkWidget* inMBarWidget, const char* inResourceName);
////
////	void			SetTarget(MHandler* inTarget);
////
////  private:
////	
////	MMenu*			CreateMenu(zeep::xml::element* inXMLNode);
////	
////	bool			OnButtonPress(GdkEventButton* inEvent);
////
////	MSlot<bool(GdkEventButton*)>
////					mOnButtonPressEvent;
////	GtkWidget*		mGtkMenubar;
////	GtkAccelGroup*	mGtkAccel;
////	MHandler*		mTarget;
////	std::list<MMenu*>
////					mMenus;
////
////	MEventOut<void(const std::string&,MMenu*)>	eUpdateSpecialMenu;
////
////	typedef std::list<std::pair<std::string,MMenu*> > MSpecialMenus;
////
////	MSpecialMenus	mSpecialMenus;
////};
////
////MMenu::MMenu(
////	const string&	inLabel,
////	GtkWidget*		inMenuWidget)
////	: mOnDestroy(this, &MMenu::OnDestroy)
////	, mOnSelectionDone(this, &MMenu::OnSelectionDone)
////	, mGtkMenu(inMenuWidget)
////	, mLabel(inLabel)
////	, mTarget(nullptr)
////	, mRadioGroup(nullptr)
////	, mPopupX(-1)
////	, mPopupY(-1)
////{
////	if (mGtkMenu == nullptr)
////		mGtkMenu = gtk_menu_new();
////
////	mOnDestroy.Connect(mGtkMenu, "destroy");
////}
////
////MMenu::~MMenu()
////{
////	for (MMenuItemList::iterator mi = mItems.begin(); mi != mItems.end(); ++mi)
////		delete *mi;
////}
////
////MMenu* MMenu::CreateFromResource(
////	const char*			inResourceName)
////{
////	MMenu* result = nullptr;
////	
////	mrsrc::rsrc rsrc(
////		string("Menus/") + inResourceName + ".xml");
////	
////	if (not rsrc)
////		THROW(("Menu resource not found: %s", inResourceName));
////
////	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
////	xml::document doc(data);
////	
////	// build a menu from the resource XML
////	xml::element* root = doc.find_first("/menu");
////
////	if (root != nullptr)
////		result = Create(root);
////
////	return result;
////}
////
////MMenu* MMenu::Create(
////	xml::element*	inXMLNode)
////{
////	string label = inXMLNode->get_attribute("label");
////	if (label.length() == 0)
////		THROW(("Invalid menu specification, label is missing"));
////	
////	string special = inXMLNode->get_attribute("special");
////
////	MMenu* menu;
////
////	if (special == "recent")
////		menu = new MMenu(label, gtk_recent_chooser_menu_new_for_manager(MRecentItems::Instance()));
////	else
////	{
////		menu = new MMenu(label);
////		
////		foreach (xml::element* item: inXMLNode->children<xml::element>())
////		{
////			if (item->qname() == "item")
////			{
////				label = item->get_attribute("label");
////				
////				if (label == "-")
////					menu->AppendSeparator();
////				else
////				{
////					string cs = item->get_attribute("cmd").c_str();
////	
////					if (cs.length() != 4)
////						THROW(("Invalid menu item specification, cmd is not correct"));
////					
////					uint32_t cmd = 0;
////					for (int i = 0; i < 4; ++i)
////						cmd |= cs[i] << ((3 - i) * 8);
////					
////					if (item->get_attribute("check") == "radio")
////						menu->AppendRadioItem(label, cmd);
////					else if (item->get_attribute("check") == "checkbox")
////						menu->AppendCheckItem(label, cmd);
////					else
////						menu->AppendItem(label, cmd);
////				}
////			}
////			else if (item->qname() == "menu")
////				menu->AppendMenu(Create(item));
////		}
////	}
////	
////	return menu;
////}
////
////bool MMenu::IsRecentMenu() const
////{
////	return GTK_IS_RECENT_CHOOSER_MENU(mGtkMenu);
////}
////
////MMenuItem* MMenu::CreateNewItem(
////	const string&	inLabel,
////	uint32_t			inCommand,
////	GSList**		ioRadioGroup)
////{
////	MMenuItem* item = new MMenuItem(this, inLabel, inCommand);
////
////	if (ioRadioGroup != nullptr)
////		item->CreateWidget(*ioRadioGroup);
////	else
////	{
////		item->CreateWidget();
////		
////		if (inLabel != "-")
////			mRadioGroup = nullptr;
////	}
////
////	item->mIndex = mItems.size();
////	mItems.push_back(item);
////	gtk_menu_shell_append(GTK_MENU_SHELL(mGtkMenu), item->mGtkMenuItem);
////	
////	return item;
////}
////
////void MMenu::AppendItem(
////	const string&	inLabel,
////	uint32_t			inCommand)
////{
////	CreateNewItem(inLabel, inCommand, nullptr);
////}
////
////void MMenu::AppendRadioItem(
////	const string&	inLabel,
////	uint32_t			inCommand)
////{
////	CreateNewItem(inLabel, inCommand, &mRadioGroup);
////}
////
////void MMenu::AppendCheckItem(
////	const string&	inLabel,
////	uint32_t			inCommand)
////{
////	MMenuItem* item = new MMenuItem(this, inLabel, inCommand);
////
////	item->CreateCheckWidget();
////
////	item->mIndex = mItems.size();
////	mItems.push_back(item);
////	gtk_menu_shell_append(GTK_MENU_SHELL(mGtkMenu), item->mGtkMenuItem);
////}
////
////void MMenu::AppendSeparator()
////{
////	CreateNewItem("-", 0, nullptr);
////}
////
////void MMenu::AppendMenu(
////	MMenu*			inMenu)
////{
////	MMenuItem* item = CreateNewItem(inMenu->GetLabel(), 0, nullptr);
////	item->mSubMenu = inMenu;
////
////	if (inMenu->IsRecentMenu())
////		item->mRecentItemActivated.Connect(inMenu->GetGtkMenu(), "item-activated");
////
////	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item->mGtkMenuItem), inMenu->mGtkMenu);
////}
////
//////void MMenu::AppendRecentMenu(
//////	const string&	inLabel)
//////{
//////	MMenuItem* item = CreateNewItem(inLabel, 0, nullptr);
//////	
//////	GtkWidget* recMenu = gtk_recent_chooser_menu_new_for_manager(MRecentItems::Instance());
//////	item->mSubMenu = new MMenu(inLabel, recMenu);;
//////
//////	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item->mGtkMenuItem), recMenu);
//////	item->mRecentItemActivated.Connect(recMenu, "item-activated");
//////}
//////
////uint32_t MMenu::CountItems()
////{
////	return mItems.size();
////}
////
////void MMenu::RemoveItems(
////	uint32_t			inFromIndex,
////	uint32_t			inCount)
////{
////	if (inFromIndex < mItems.size())
////	{
////		MMenuItemList::iterator b = mItems.begin();
////		advance(b, inFromIndex);
////
////		if (inFromIndex + inCount > mItems.size())
////			inCount = mItems.size() - inFromIndex;
////
////		MMenuItemList::iterator e = b;
////		advance(e, inCount);	
////		
////		for (MMenuItemList::iterator mi = b; mi != e; ++mi)
////			gtk_widget_destroy((*mi)->mGtkMenuItem);
////		
////		mItems.erase(b, e);
////	}
////}
////
////string MMenu::GetItemLabel(
////	uint32_t				inIndex) const
////{
////	if (inIndex >= mItems.size())
////		THROW(("Item index out of range"));
////	
////	MMenuItemList::const_iterator i = mItems.begin();
////	advance(i, inIndex);
////	
////	return (*i)->mLabel;
////}
////
////bool MMenu::GetRecentItem(
////	uint32_t				inIndex,
////	MFile&				outURL) const
////{
////	if (inIndex >= mItems.size())
////		THROW(("Item index out of range"));
////	
////	MMenuItemList::const_iterator i = mItems.begin();
////	advance(i, inIndex);
////	
////	MMenuItem* item = *i;
////	
////	if (item->mSubMenu == nullptr or not GTK_IS_RECENT_CHOOSER(item->mSubMenu->GetGtkMenu()))
////		THROW(("Invalid item/menu"));
////
////	bool result = false;
////	char* uri = gtk_recent_chooser_get_current_uri(GTK_RECENT_CHOOSER(item->mSubMenu->GetGtkMenu()));
////	
////	if (uri != nullptr)
////	{
////		outURL = MFile(uri);
////		g_free(uri);
////		
////		result = true;
////	}
////
////	return result;
////}
////
////void MMenu::AddToRecentMenu(
////	const MFile&		inFileRef)
////{
////	string uri = inFileRef.GetURI();
////	
////	if (gtk_recent_manager_has_item(MRecentItems::Instance(), uri.c_str()))
////		gtk_recent_manager_remove_item(MRecentItems::Instance(), uri.c_str(), nullptr);
////	
////	gtk_recent_manager_add_item(MRecentItems::Instance(), uri.c_str());
////}
////
////void MMenu::SetTarget(
////	MHandler*		inTarget)
////{
////	mTarget = inTarget;
////	
////	for (MMenuItemList::iterator mi = mItems.begin(); mi != mItems.end(); ++mi)
////	{
////		if ((*mi)->mSubMenu != nullptr)
////			(*mi)->mSubMenu->SetTarget(inTarget);
////	}
////}
////
////void MMenu::UpdateCommandStatus()
////{
////	if (mTarget == nullptr)
////		return;
////	
////	for (MMenuItemList::iterator mi = mItems.begin(); mi != mItems.end(); ++mi)
////	{
////		MMenuItem* item = *mi;
////		
////		if (item->mCommand != 0)
////		{
////			bool enabled = item->mEnabled;
////			bool checked = item->mChecked;
////			
////			if (mTarget->UpdateCommandStatus(item->mCommand, this,
////				distance(mItems.begin(), mi), enabled, checked))
////			{
////				if (enabled != item->mEnabled)
////				{
////					gtk_widget_set_sensitive(item->mGtkMenuItem, enabled);
////					item->mEnabled = enabled;
////				}
////				
//////				if (item->mCanCheck)
////					item->SetChecked(checked);
////			}
////		}
////		
////		if ((*mi)->mSubMenu != nullptr)
////			(*mi)->mSubMenu->UpdateCommandStatus();
////	}
////}
////
////void MMenu::SetAcceleratorGroup(
////	GtkAccelGroup*		inAcceleratorGroup)
////{
////	MAcceleratorTable& at = MAcceleratorTable::Instance();
////	
////	gtk_menu_set_accel_group(GTK_MENU(mGtkMenu), inAcceleratorGroup);
////	
////	for (MMenuItemList::iterator mi = mItems.begin(); mi != mItems.end(); ++mi)
////	{
////		MMenuItem* item = *mi;
////		
////		uint32_t key, mod;
////		
////		if (at.GetAcceleratorKeyForCommand(item->mCommand, key, mod))
////		{
////			gtk_widget_add_accelerator(item->mGtkMenuItem, "activate", inAcceleratorGroup,
////				key, GdkModifierType(mod), GTK_ACCEL_VISIBLE);
////		}
////		
////		if (item->mSubMenu != nullptr)
////			item->mSubMenu->SetAcceleratorGroup(inAcceleratorGroup);
////	}
////}
////
////void MMenu::MenuPosition(
////	GtkMenu*			inMenu,
////	gint*				inX,
////	gint*				inY,
////	gboolean*			inPushIn,
////	gpointer			inUserData)
////{
////	MMenu* self = reinterpret_cast<MMenu*>(g_object_get_data(G_OBJECT(inMenu), "MMenu"));	
////	
////	*inX = self->mPopupX;
////	*inY = self->mPopupY;
////	*inPushIn = true;
////}
////
////void MMenu::Popup(
////	MHandler*			inHandler,
////	GdkEventButton*		inEvent,
////	int32_t				inX,
////	int32_t				inY,
////	bool				inBottomMenu)
////{
////	SetTarget(inHandler);
////	
////	mOnSelectionDone.Connect(mGtkMenu, "selection-done");
////	
////	mPopupX = inX;
////	mPopupY = inY;
////	
////	g_object_set_data(G_OBJECT(mGtkMenu), "MMenu", this);
////
////	gtk_widget_show_all(mGtkMenu);
////
////	int32_t button = 0;
////	uint32_t time = 0;
////	if (inEvent != nullptr)
////	{
////		button = inEvent->button;
////		time = inEvent->time;
////	}
////	
////	UpdateCommandStatus();
////
////	gtk_menu_popup(GTK_MENU(mGtkMenu), nullptr, nullptr,
////		&MMenu::MenuPosition, nullptr, button, time);
////}
////
////bool MMenu::OnDestroy()
////{
////	mGtkMenu = nullptr;
////	
////	delete this;
////	
////	return false;
////}
////
////void MMenu::OnSelectionDone()
////{
////	gtk_widget_destroy(mGtkMenu);
////}
//
////MMenubar::MMenubar(
////	MHandler*		inTarget)
////	: mOnButtonPressEvent(this, &MMenubar::OnButtonPress)
////	, mGtkMenubar(nullptr)
////	, mTarget(inTarget)
////{
////	mGtkAccel = gtk_accel_group_new();
////	
////	AddRoute(eUpdateSpecialMenu, gApp->eUpdateSpecialMenu);
////}
////
////void MMenubar::Initialize(
////	GtkWidget*		inMBarWidget,
////	const char*		inResourceName)
////{
////	mGtkMenubar = inMBarWidget;
////	mOnButtonPressEvent.Connect(mGtkMenubar, "button-press-event");
////	
////	mrsrc::rsrc rsrc(string("Menus/") + inResourceName + ".xml");
////	
////	if (not rsrc)
////		THROW(("Menu resource not found: %s", inResourceName));
////
////	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
////	xml::document doc(data);
////	
////	// build a menubar from the resource XML
////	foreach (xml::element* menu: doc.find("/menubar/menu"))
////	{
////		MMenu* obj = CreateMenu(menu);
////		AddMenu(obj);
////	}
////	
////	gtk_widget_show_all(mGtkMenubar);
////}
////
////MMenu* MMenubar::CreateMenu(
////	xml::element*	inXMLNode)
////{
////	string label = inXMLNode->get_attribute("label");
////	if (label.length() == 0)
////		THROW(("Invalid menu specification, label is missing"));
////	
////	string special = inXMLNode->get_attribute("special");
////
////	MMenu* menu;
////
////	if (special == "recent")
////		menu = new MMenu(label, gtk_recent_chooser_menu_new_for_manager(MRecentItems::Instance()));
////	else
////	{
////		menu = new MMenu(label);
////		
////		foreach (xml::element* item: inXMLNode->children<xml::element>())
////		{
////			if (item->qname() == "item")
////			{
////				label = item->get_attribute("label");
////				
////				if (label == "-")
////					menu->AppendSeparator();
////				else
////				{
////					string cs = item->get_attribute("cmd").c_str();
////	
////					if (cs.length() != 4)
////						THROW(("Invalid menu item specification, cmd is not correct"));
////					
////					uint32_t cmd = 0;
////					for (int i = 0; i < 4; ++i)
////						cmd |= cs[i] << ((3 - i) * 8);
////					
////					if (item->get_attribute("check") == "radio")
////						menu->AppendRadioItem(label, cmd);
////					else if (item->get_attribute("check") == "checkbox")
////						menu->AppendCheckItem(label, cmd);
////					else
////						menu->AppendItem(label, cmd);
////				}
////			}
////			else if (item->qname() == "menu")
////				menu->AppendMenu(CreateMenu(item));
////		}
////	}
////	
////	if (not special.empty() and special != "recent")
////		mSpecialMenus.push_back(make_pair(special, menu));
////	
////	return menu;
////}
