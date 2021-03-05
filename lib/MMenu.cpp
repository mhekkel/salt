//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <iostream>
#include <algorithm>
#include <cstring>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem/fstream.hpp>

#include <zeep/xml/document.hpp>

#include "MMenu.hpp"
#include "MMenuImpl.hpp"
#include "MWindow.hpp"
#include "MAcceleratorTable.hpp"
#include "MFile.hpp"
#include "MStrings.hpp"
#include "MResources.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MApplication.hpp"

using namespace std;
namespace xml = zeep::xml;
namespace io = boost::iostreams;

namespace
{

struct MCommandToString
{
	char mCommandString[10];
	
	MCommandToString(
		uint32			inCommand)
	{
		strcpy(mCommandString, "MCmd_xxxx");
		
		mCommandString[5] = ((inCommand & 0xff000000) >> 24) & 0x000000ff;
		mCommandString[6] = ((inCommand & 0x00ff0000) >> 16) & 0x000000ff;
		mCommandString[7] = ((inCommand & 0x0000ff00) >>  8) & 0x000000ff;
		mCommandString[8] = ((inCommand & 0x000000ff) >>  0) & 0x000000ff;
	}
	
	operator const char*() const	{ return mCommandString; }
};

}

// --------------------------------------------------------------------

MMenu::MMenu(
	const string&	inLabel,
	bool			inPopup)
	: mImpl(MMenuImpl::Create(this, inPopup))
	, mLabel(inLabel)
	, mTarget(nullptr)
{
}

MMenu::MMenu(MMenuImpl* inImpl)
	: mImpl(inImpl)
	, mTarget(nullptr)
{
}

MMenu::~MMenu()
{
	delete mImpl;
}

MMenu* MMenu::CreateFromResource(
	const char*			inResourceName,
	bool				inPopup)
{
	MMenu* result = nullptr;
	
	string resource = string("Menus/") + inResourceName + ".xml";
	mrsrc::rsrc rsrc(resource);
	
	if (not rsrc)
		THROW(("Menu resource not found: %s", resource.c_str()));

	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
	xml::document doc(data);
	
	// build a menu from the resource XML
	xml::element* root = doc.find_first("/menu");

	if (root != nullptr)
		result = Create(root, inPopup);

	return result;
}

MMenu* MMenu::Create(
	xml::element*		inXMLNode,
	bool				inPopup)
{
	string label;

	if (inXMLNode->name() == "menu")
	{
		label = GetLocalisedStringForContext("menu", inXMLNode->attr("label"));
		if (label.length() == 0)
			THROW(("Invalid menu specification, label is missing"));
	}
	
	string special = inXMLNode->attr("special");

	MMenu* menu = new MMenu(label, inPopup);
	menu->mSpecial = special;

	for (auto item: *inXMLNode)
	{
		if (item.qname() == "item")
		{
			label = GetLocalisedStringForContext("menu", item.attr("label"));
			
			if (label == "-")
				menu->AppendSeparator();
			else
			{
				string cs = item.attr("cmd").c_str();

				if (cs.length() != 4)
					THROW(("Invalid menu item specification, cmd is not correct"));
				
				uint32 cmd = 0;
				for (int i = 0; i < 4; ++i)
					cmd |= cs[i] << ((3 - i) * 8);
				
				if (item.attr("check") == "radio")
					menu->AppendRadioItem(label, cmd);
				else if (item.attr("check") == "checkbox")
					menu->AppendCheckItem(label, cmd);
				else
					menu->AppendItem(label, cmd);
			}
		}
		else if (item.qname() == "menu")
			menu->AppendMenu(Create(&item, false));
	}
	
	return menu;
}

void MMenu::AppendItem(
	const string&	inLabel,
	uint32			inCommand)
{
	mImpl->AppendItem(inLabel, inCommand);
}

void MMenu::AppendRadioItem(
	const string&	inLabel,
	uint32			inCommand)
{
	mImpl->AppendRadiobutton(inLabel, inCommand);
}

void MMenu::AppendCheckItem(
	const string&	inLabel,
	uint32			inCommand)
{
	mImpl->AppendCheckbox(inLabel, inCommand);
}

void MMenu::AppendSeparator()
{
	mImpl->AppendSeparator();
}

void MMenu::AppendMenu(
	MMenu*			inMenu)
{
	mImpl->AppendSubmenu(inMenu);
}

uint32 MMenu::CountItems()
{
	return mImpl->CountItems();
}

void MMenu::RemoveItems(
	uint32			inFromIndex,
	uint32			inCount)
{
	mImpl->RemoveItems(inFromIndex, inCount);
}

string MMenu::GetItemLabel(
	uint32				inIndex) const
{
	return mImpl->GetItemLabel(inIndex);
}

void MMenu::SetItemCommand(uint32 inIndex, uint32 inCommand)
{
	mImpl->SetItemCommand(inIndex, inCommand);
}

uint32 MMenu::GetItemCommand(
	uint32				inIndex) const
{
	return mImpl->GetItemCommand(inIndex);
}

void MMenu::SetTarget(MHandler* inTarget)
{
	mTarget = inTarget;
	mImpl->SetTarget(inTarget);

	for (uint32 i = 0; i < CountItems(); ++i)
	{
		MMenu* subMenu = mImpl->GetSubmenu(i);
		if (subMenu != nullptr)
			subMenu->SetTarget(inTarget);
	}
}

void MMenu::UpdateCommandStatus()
{
	if (not mSpecial.empty())
		gApp->UpdateSpecialMenu(mSpecial, this);

	for (uint32 i = 0; i < CountItems(); ++i)
	{
		MMenu* subMenu = mImpl->GetSubmenu(i);
		if (subMenu != nullptr)
			subMenu->UpdateCommandStatus();
		else
		{
			bool enabled = false, checked = false;
			if (mTarget != nullptr)
			{
				MWindow* window = dynamic_cast<MWindow*>(mTarget);
				if (window != nullptr)
					window->FindFocus()->UpdateCommandStatus(GetItemCommand(i), this, i, enabled, checked);
				else
					mTarget->UpdateCommandStatus(GetItemCommand(i), this, i, enabled, checked);
			}
			mImpl->SetItemState(i, enabled, checked);
		}
	}
	
	mImpl->MenuUpdated();
}

//void MMenu::SetAcceleratorGroup(
//	GtkAccelGroup*		inAcceleratorGroup)
//{
//	MAcceleratorTable& at = MAcceleratorTable::Instance();
//	
//	gtk_menu_set_accel_group(GTK_MENU(mGtkMenu), inAcceleratorGroup);
//	
//	for (MMenuItemList::iterator mi = mItems.begin(); mi != mItems.end(); ++mi)
//	{
//		MMenuItem* item = *mi;
//		
//		uint32 key, mod;
//		
//		if (at.GetAcceleratorKeyForCommand(item.mCommand, key, mod))
//		{
//			gtk_widget_add_accelerator(item.mGtkMenuItem, "activate", inAcceleratorGroup,
//				key, GdkModifierType(mod), GTK_ACCEL_VISIBLE);
//		}
//		
//		if (item.mSubMenu != nullptr)
//			item.mSubMenu->SetAcceleratorGroup(inAcceleratorGroup);
//	}
//}

//void MMenu::MenuPosition(
//	GtkMenu*			inMenu,
//	gint*				inX,
//	gint*				inY,
//	gboolean*			inPushIn,
//	gpointer			inUserData)
//{
//	MMenu* self = reinterpret_cast<MMenu*>(g_object_get_data(G_OBJECT(inMenu), "MMenu"));	
//	
//	*inX = self->mPopupX;
//	*inY = self->mPopupY;
//	*inPushIn = true;
//}

void MMenu::Popup(
	MWindow*			inHandler,
	int32				inX,
	int32				inY,
	bool				inBottomMenu)
{
	mImpl->Popup(inHandler, inX, inY, inBottomMenu);
	//SetTarget(inHandler);
	//
	//mOnSelectionDone.Connect(mGtkMenu, "selection-done");
	//
	//mPopupX = inX;
	//mPopupY = inY;
	//
	//g_object_set_data(G_OBJECT(mGtkMenu), "MMenu", this);

	//gtk_widget_show_all(mGtkMenu);

	//int32 button = 0;
	//uint32 time = 0;
	//if (inEvent != nullptr)
	//{
	//	button = inEvent->button;
	//	time = inEvent->time;
	//}
	//
	//UpdateCommandStatus();

	//gtk_menu_popup(GTK_MENU(mGtkMenu), nullptr, nullptr,
	//	&MMenu::MenuPosition, nullptr, button, time);
}

//bool MMenu::OnDestroy()
//{
//	mGtkMenu = nullptr;
//	
//	delete this;
//	
//	return false;
//}
//
//void MMenu::OnSelectionDone()
//{
//	gtk_widget_destroy(mGtkMenu);
//}

// --------------------------------------------------------------------

MMenuBar::MMenuBar()
	: MMenu(MMenuImpl::CreateBar(this))
{
}

MMenuBar* MMenuBar::Create(zeep::xml::element* inXMLNode)
{
	if (inXMLNode->name() != "menubar")
		THROW(("Invalid menubar specification"));
	
	MMenuBar* result = new MMenuBar();
	
	for (auto item: *inXMLNode)
	{
		if (item.qname() == "menu")
			result->AppendMenu(MMenu::Create(&item, false));
	}
	
	return result;
}

void MMenuBar::AddToWindow(MWindowImpl* inWindowImpl)
{
	mImpl->AddToWindow(inWindowImpl);
}

//void MMenubar::Initialize(
//	GtkWidget*		inMBarWidget,
//	const char*		inResourceName)
//{
//	mGtkMenubar = inMBarWidget;
//	mOnButtonPressEvent.Connect(mGtkMenubar, "button-press-event");
//	
//	mrsrc::rsrc rsrc(string("Menus/") + inResourceName + ".xml");
//	
//	if (not rsrc)
//		THROW(("Menu resource not found: %s", inResourceName));
//
//	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
//	xml::document doc(data);
//	
//	// build a menubar from the resource XML
//	for (xml::element* menu : doc.find("/menubar/menu"))
//	{
//		MMenu* obj = CreateMenu(menu);
//		AddMenu(obj);
//	}
//	
//	gtk_widget_show_all(mGtkMenubar);
//}
//
//MMenu* MMenubar::CreateMenu(
//	xml::element*	inXMLNode)
//{
//	string label = inXMLNode->attr("label");
//	if (label.length() == 0)
//		THROW(("Invalid menu specification, label is missing"));
//	
//	string special = inXMLNode->attr("special");
//
//	MMenu* menu;
//
//	if (special == "recent")
//		menu = new MMenu(label, gtk_recent_chooser_menu_new_for_manager(MRecentItems::Instance()));
//	else
//	{
//		menu = new MMenu(label);
//		
//		for (xml::element* item : inXMLNode->children<xml::element>())
//		{
//			if (item.qname() == "item")
//			{
//				label = item.attr("label");
//				
//				if (label == "-")
//					menu->AppendSeparator();
//				else
//				{
//					string cs = item.attr("cmd").c_str();
//	
//					if (cs.length() != 4)
//						THROW(("Invalid menu item specification, cmd is not correct"));
//					
//					uint32 cmd = 0;
//					for (int i = 0; i < 4; ++i)
//						cmd |= cs[i] << ((3 - i) * 8);
//					
//					if (item.attr("check") == "radio")
//						menu->AppendRadioItem(label, cmd);
//					else if (item.attr("check") == "checkbox")
//						menu->AppendCheckItem(label, cmd);
//					else
//						menu->AppendItem(label, cmd);
//				}
//			}
//			else if (item.qname() == "menu")
//				menu->AppendMenu(CreateMenu(item));
//		}
//	}
//	
//	if (not special.empty() and special != "recent")
//		mSpecialMenus.push_back(make_pair(special, menu));
//	
//	return menu;
//}
//
//void MMenubar::AddMenu(
//	MMenu*				inMenu)
//{
//	GtkWidget* menuItem = gtk_menu_item_new_with_label(_(inMenu->GetLabel().c_str()));
//	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), inMenu->GetGtkMenu());
//	gtk_menu_shell_append(GTK_MENU_SHELL(mGtkMenubar), menuItem);
//	
//	if (mTarget != nullptr)
//		inMenu->SetTarget(mTarget);
//
//	inMenu->SetAcceleratorGroup(mGtkAccel);
//	mMenus.push_back(inMenu);
//}
//
//bool MMenubar::OnButtonPress(
//	GdkEventButton*		inEvent)
//{
//	for (list<MMenu*>::iterator m = mMenus.begin(); m != mMenus.end(); ++m)
//		(*m)->UpdateCommandStatus();
//
//	for (MSpecialMenus::iterator m = mSpecialMenus.begin(); m != mSpecialMenus.end(); ++m)
//		eUpdateSpecialMenu(m->first, m->second);
//
//	gtk_widget_show_all(mGtkMenubar);
//
//	return false;
//}
//
//void MMenubar::SetTarget(
//	MHandler*			inTarget)
//{
//	mTarget = inTarget;
//	
//	for (list<MMenu*>::iterator m = mMenus.begin(); m != mMenus.end(); ++m)
//		(*m)->SetTarget(inTarget);
//}
//
