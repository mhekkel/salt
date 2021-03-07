//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <iostream>

#include <gdk/gdkx.h>

#include "MMenu.hpp"
#include "MCommands.hpp"
#include "MWindow.hpp"
#include "MError.hpp"
#include "MApplication.hpp"
#include "MGtkWindowImpl.hpp"
#include "MResources.hpp"

using namespace std;

list<MWindow*> MGtkWindowImpl::sRecycle;

// --------------------------------------------------------------------
//
//	MGtkWindowImpl
//

MGtkWindowImpl::MGtkWindowImpl(MWindowFlags inFlags, const std::string& inMenu, MWindow* inWindow)
	: MWindowImpl(inFlags, inWindow)
//	, mModified(false)
//	, mTransitionThread(nullptr)
	, mChildFocus(this, &MGtkWindowImpl::ChildFocus)
	, mMapEvent(this, &MGtkWindowImpl::OnMapEvent)
//	, mChanged(this, &MGtkWindowImpl::Changed)
	, mMenubar(nullptr)
	, mMainVBox(nullptr)
	, mFocus(this)
	, mConfigured(false)
{
	if (not inMenu.empty())
		mMenubar = MWindowImpl::CreateMenu(inMenu);
}

MGtkWindowImpl::~MGtkWindowImpl()
{
}

void MGtkWindowImpl::Create(MRect inBounds, const std::string& inTitle)
{
	GtkWidget* widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	THROW_IF_NIL(widget);

	gtk_window_set_default_size(GTK_WINDOW(widget), inBounds.width, inBounds.height);
	gtk_window_set_title(GTK_WINDOW(widget), inTitle.c_str());
	
	SetWidget(widget);
	
	GList* iconList = nullptr;

	mrsrc::rsrc appIconResource("Icons/appicon.png");
	GInputStream* s = g_memory_input_stream_new_from_data(appIconResource.data(), appIconResource.size(), nullptr);
	THROW_IF_NIL(s);
	
	GError* error = nullptr;
	GdkPixbuf* icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
	if (icon)
		iconList = g_list_append(iconList, icon);
	
	if (error)
		g_free(error);

	mrsrc::rsrc smallAppIconResource("Icons/appicon.png");
	s = g_memory_input_stream_new_from_data(smallAppIconResource.data(), smallAppIconResource.size(), nullptr);
	THROW_IF_NIL(s);
	
	icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
	if (icon)
		iconList = g_list_append(iconList, icon);
	
	if (error)
		g_free(error);

	if (iconList)
		gtk_window_set_icon_list(GTK_WINDOW(widget), iconList);

//	GList* defaulIconList = gtk_window_get_default_icon_list();
//	if (defaulIconList != nullptr)
//	{
//		gtk_window_set_icon_list(GTK_WINDOW(widget), defaulIconList);
//		g_list_free(defaulIconList);
//	}
	
	mMapEvent.Connect(widget, "map-event");
	
	if (mMenubar != nullptr)
	{
		mMenubar->AddToWindow(this);
		mMenubar->SetTarget(mWindow);
	}
	
//	mChanged.Connect(this, "on_changed");
}

void MGtkWindowImpl::AddMenubarWidget(GtkWidget* inWidget)
{
	if (mMainVBox == nullptr)
	{
		mMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(GetWidget()), mMainVBox);
		gtk_widget_show(mMainVBox);
	}
	
	gtk_box_pack_start(GTK_BOX(mMainVBox), inWidget, FALSE, FALSE, 0);
	gtk_widget_show_all(inWidget);
}

void MGtkWindowImpl::AddStatusbarWidget(MGtkWidgetMixin* inChild)
{
	if (mMainVBox == nullptr)
	{
		mMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(GetWidget()), mMainVBox);
		gtk_widget_show(mMainVBox);
	}
	
	gtk_box_pack_end(GTK_BOX(mMainVBox), inChild->GetWidget(), FALSE, FALSE, 0);
	gtk_widget_show_all(inChild->GetWidget());
}

void MGtkWindowImpl::Append(MGtkWidgetMixin* inChild, MControlPacking inPacking,
		bool inExpand, bool inFill, uint32_t inPadding)
{
	if (mMainVBox == nullptr)
	{
		mMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(GetWidget()), mMainVBox);
		gtk_widget_show(mMainVBox);
	}
	
	if (inPacking == ePackStart)
		gtk_box_pack_start(GTK_BOX(mMainVBox), inChild->GetWidget(), inExpand, inFill, inPadding);
	else
		gtk_box_pack_end(GTK_BOX(mMainVBox), inChild->GetWidget(), inExpand, inFill, inPadding);
}

//void MGtkWindowImpl::ConnectChildSignals()
//{
//	gtk_container_foreach(GTK_CONTAINER(GetWidget()), &MGtkWindowImpl::DoForEachCallBack, this);
//}
//
//void MGtkWindowImpl::RemoveWindowFromList(
//	MWindow*		inWindow)
//{
//	if (inWindow == sFirst)
//		sFirst = inWindow->mNext;
//	else if (sFirst != nullptr)
//	{
//		MWindow* w = sFirst;
//		while (w != nullptr)
//		{
//			MWindow* next = w->mNext;
//			if (next == inWindow)
//			{
//				w->mNext = inWindow->mNext;
//				break;
//			}
//			w = next;
//		}
//	}
//}
	
void MGtkWindowImpl::Show()
{
	gtk_window_present(GTK_WINDOW(GetWidget()));
	gtk_widget_show(GetWidget());
}

void MGtkWindowImpl::Hide()
{
	gtk_widget_hide(GetWidget());
}

bool MGtkWindowImpl::Visible() const
{
	return gtk_widget_get_window(GetWidget()) != nullptr and gdk_window_is_visible(gtk_widget_get_window(GetWidget()));
}

// Mijn eigen xdo implementatie... zucht

bool GetProperty(Display* display, Window window, const string& name,
	long maxLength, Atom& type, int& format, unsigned long& numItems, unsigned char*& prop)
{
	Atom propertyAtom = XInternAtom(display, name.c_str(), false);
	unsigned long remainingBytes;
	return XGetWindowProperty(display, window, propertyAtom, 0, maxLength, False,
		AnyPropertyType, &type, &format, &numItems, &remainingBytes, &prop) == Success;
}

bool PropertyExists(Display* display, Window window, const string& name)
{
	Atom type = None;
	int format = 0;
	unsigned long numItems = 0;
	unsigned char* property = nullptr;
	
	bool result = GetProperty(display, window, name, 1024, type, format, numItems, property);
	
	if (property != nullptr)
		XFree(property);
	
	return result and numItems > 0;
}

bool GetXIDProperty(Display* display, Window window, const string& name, XID& xid)
{
	Atom type = None;
	int format = 0;
	unsigned long numItems = 0;
	unsigned char* property = nullptr;
	
	bool result = GetProperty(display, window, name, 1024, type, format, numItems, property);
	
	if (result and numItems > 0 and format == 32 and property != nullptr)
		xid = *reinterpret_cast<XID*>(property);
	else
		result = false;
	
	if (property != nullptr)
		XFree(property);
	
	return result;
}

bool GetLongPropery(Display* display, Window window, const string& name, long& v)
{
	Atom type = None;
	int format = 0;
	unsigned long numItems = 0;
	unsigned char* property = nullptr;
	
	bool result = GetProperty(display, window, name, 1024, type, format, numItems, property);
	
	if (result and numItems > 0 and format == 32 and property != nullptr)
		v = *reinterpret_cast<long*>(property);
	else
		result = false;
	
	if (property != nullptr)
		XFree(property);
	
	return result;
}

long GetDesktopForWindow(Display* display, Window window)
{
	long desktop = -1;
	
	if (not GetLongPropery(display, window, "_NET_WM_DESKTOP", desktop) and
		not GetLongPropery(display, window, "_WIN_WORKSPACE", desktop))
	{
		PRINT(("Error getting desktop for window"));
	}

	return desktop;
}

long GetCurrentDesktop(Display* display)
{
	long desktop = -1;
	
	Window root = DefaultRootWindow(display);
	
	if (not GetLongPropery(display, root, "_NET_CURRENT_DESKTOP", desktop) and
		not GetLongPropery(display, root, "_WIN_WORKSPACE", desktop))
	{
		PRINT(("Failed to get current desktop"));
	}

	return desktop;
}

void SetCurrentDesktop(Display* display, long desktop)
{
	Window root = DefaultRootWindow(display);
	
	XEvent xev = {};
	xev.type = ClientMessage;
	xev.xclient.display = display;
	xev.xclient.window = root;
	xev.xclient.message_type = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = desktop;
	xev.xclient.data.l[1] = CurrentTime;
	
	int ret = XSendEvent(display, root, False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
	
	if (ret == 0)
		PRINT(("_NET_CURRENT_DESKTOP failed"));
}

bool ActivateWindow(Display* display, Window window)
{
	long desktop = GetDesktopForWindow(display, window);
	long current = GetCurrentDesktop(display);
	
	PRINT(("ActivateWindow, desktop = %ld, current = %ld", desktop, current));
	
	if (desktop != current and desktop != -1)
		SetCurrentDesktop(display, desktop);

	XEvent xev = {};
	xev.type = ClientMessage;
	xev.xclient.display = display;
	xev.xclient.window = window;
	xev.xclient.message_type = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 2;	// window pager... is dat echt wat we willen?
	xev.xclient.data.l[1] = CurrentTime;
	
	XWindowAttributes attr;
	XGetWindowAttributes(display, window, &attr);
	int ret = XSendEvent(display, attr.screen->root, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	
	if (ret == 0)
		PRINT(("_NET_ACTIVE_WINDOW failed"));

	return ret != 0;
}

void MGtkWindowImpl::Select()
{
	PRINT(("Select Window"));

#warning("fixme")
	ActivateWindow(gdk_x11_display_get_xdisplay(gdk_display_get_default()),
		gdk_x11_window_get_xid(gtk_widget_get_window(GetWidget())));

	if (Visible())
	{
		gdk_window_raise(gtk_widget_get_window(GetWidget()));
		gtk_window_present(GTK_WINDOW(GetWidget()));
	}
	else
		Show();

	mWindow->BeFocus();
}

void MGtkWindowImpl::Close()
{
	if (mWindow->AllowClose(false))
		gtk_widget_destroy(GetWidget());
}

void MGtkWindowImpl::SetTitle(string inTitle)
{
	gtk_window_set_title(GTK_WINDOW(GetWidget()), inTitle.c_str());
}

//string MGtkWindowImpl::GetTitle() const
//{
//	const char* title = gtk_window_get_title(GTK_WINDOW(GetWidget()));
//	return title ? title : "";
//}

//void MGtkWindowImpl::SetModifiedMarkInTitle(
//	bool		inModified)
//{
//	if (mModified != inModified)
//	{
//		mModified = inModified;
//		SetTitle(mTitle);
//	}
//}

void MGtkWindowImpl::RecycleWindows()
{
	for (MWindow* w: sRecycle)
		delete w;
	sRecycle.clear();
}

bool MGtkWindowImpl::OnDestroy()
{
	PRINT(("MGtkWindowImpl::OnDestroy"));
	
	SetWidget(nullptr);

//	mWindow->eWindowClosed(mWindow);
//	delete mWindow;
	sRecycle.push_back(mWindow);
	
	return true;
}

bool MGtkWindowImpl::OnDelete(GdkEvent* inEvent)
{
	PRINT(("MGtkWindowImpl::OnDelete"));
	
	bool result = true;

	if (mWindow->AllowClose(false))
		result = false;
	
	return result;
}

bool MGtkWindowImpl::OnMapEvent(GdkEvent* inEvent)
{
	PRINT(("MGtkWindowImpl::OnMapEvent"));

	mWindow->BeFocus();

	return false;
}

bool MGtkWindowImpl::OnConfigureEvent(GdkEventConfigure* inEvent)
{
	PRINT(("MGtkWindowImpl::OnConfigureEvent"));
	if (not mConfigured)
		mWindow->Mapped();
	mConfigured = true;
	
	return MGtkWidgetMixin::OnConfigureEvent(inEvent);
}

void MGtkWindowImpl::ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta)
{
	PRINT(("MGtkWindowImpl::ResizeWindow(%d,%d)", inWidthDelta, inHeightDelta));
	int w, h;
	gtk_window_get_size(GTK_WINDOW(GetWidget()), &w, &h);
	gtk_window_resize(GTK_WINDOW(GetWidget()), w + inWidthDelta, h + inHeightDelta);
}

void MGtkWindowImpl::GetWindowPosition(MRect& outPosition) const
{
	int x, y;
	gtk_window_get_position(GTK_WINDOW(GetWidget()), &x, &y);
	
	int w, h;
	gtk_window_get_size(GTK_WINDOW(GetWidget()), &w, &h);
	
	outPosition = MRect(x, y, w, h);
}

void MGtkWindowImpl::SetWindowPosition(MRect inPosition, bool inTransition)
{
	PRINT(("MGtkWindowImpl::SetWindowPosition"));
	if (inTransition)
	{
//		if (mTransitionThread != nullptr)
//			THROW(("SetWindowPosition called to fast"));
//		
//		mTransitionThread =
//			new boost::thread(std::bind(&MGtkWindowImpl::TransitionTo, this, inPosition));
	}
	else
	{
		gtk_window_move(GTK_WINDOW(GetWidget()),
			inPosition.x, inPosition.y);
	
		gtk_window_resize(GTK_WINDOW(GetWidget()),
			inPosition.width, inPosition.height);
	}
}

// try to be nice to those with multiple monitors:

//void MGtkWindowImpl::GetMaxPosition(
//	MRect&			outRect) const
//{
//	GdkScreen* screen = gtk_widget_get_screen(GetWidget());
//	
//	uint32_t monitor = gdk_screen_get_monitor_at_window(screen, gtk_widget_get_window(GetWidget()));
//	
//	GdkRectangle r;
//	gdk_screen_get_monitor_geometry(screen, monitor, &r);
//	outRect = r;
//}
//
//void MGtkWindowImpl::TransitionTo(
//	MRect			inPosition)
//{
//	MRect start;
//
//	gdk_threads_enter();
//	GetWindowPosition(start);
//	gdk_threads_leave();
//	
//	uint32_t
//		kSleep = 10000,
//		kSteps = 6;
//	
//	for (uint32_t step = 0; step < kSteps; ++step)
//	{
//		MRect r;
//		
//		r.x = ((kSteps - step) * start.x + step * inPosition.x) / kSteps;
//		r.y = ((kSteps - step) * start.y + step * inPosition.y) / kSteps;
//		r.width = ((kSteps - step) * start.width + step * inPosition.width) / kSteps;
//		r.height = ((kSteps - step) * start.height + step * inPosition.height) / kSteps;
//
//		gdk_threads_enter();
//		SetWindowPosition(r, false);
//		gdk_window_process_all_updates();
//		gdk_threads_leave();
//		
//		usleep(kSleep);
//	}
//
//	gdk_threads_enter();
//	SetWindowPosition(inPosition, false);
//	gdk_threads_leave();
//	
//	mTransitionThread = nullptr;
//}
//
//const char* MGtkWindowImpl::IDToName(
//	uint32_t			inID,
//	char			inName[5])
//{
//	inName[4] = 0;
//	inName[3] = inID & 0x000000ff; inID >>= 8;
//	inName[2] = inID & 0x000000ff; inID >>= 8;
//	inName[1] = inID & 0x000000ff; inID >>= 8;
//	inName[0] = inID & 0x000000ff;
//	
//	return inName;
//}
//
//GtkWidget* MGtkWindowImpl::GetWidget(
//	uint32_t			inID) const
//{
//	char name[5];
//	GtkWidget* wdgt = mGtkBuilder->GetWidget(IDToName(inID, name));
//	if (wdgt == nullptr)
//		THROW(("Widget '%s' does not exist", name));
//	return wdgt;
//}
//
//void MGtkWindowImpl::Beep()
//{
////	gdk_window_beep(gtk_widget_get_window(GetWidget()));
//	cout << "beep!" << endl;
//	gdk_beep();
//}

void MGtkWindowImpl::DoForEachCallBack(GtkWidget* inWidget, gpointer inUserData)
{
	MGtkWindowImpl* w = reinterpret_cast<MGtkWindowImpl*>(inUserData);
	w->DoForEach(inWidget);
}

void MGtkWindowImpl::DoForEach(GtkWidget* inWidget)
{
	gboolean canFocus = false;

	g_object_get(G_OBJECT(inWidget), "can-focus", &canFocus, NULL);

	if (canFocus)
		mChildFocus.Connect(inWidget, "focus-in-event");
	
	if (GTK_IS_CONTAINER(inWidget))
		gtk_container_foreach(GTK_CONTAINER(inWidget), &MGtkWindowImpl::DoForEachCallBack, this);
}

bool MGtkWindowImpl::ChildFocus(GdkEventFocus* inEvent)
{
	PRINT(("focus-in-event"));

	try
	{
		mWindow->BeFocus();
	}
	catch (...) {}
	return false;
}

//void MGtkWindowImpl::PutOnDuty(
//	MHandler*		inHandler)
//{
//	MWindow* w = sFirst;
//	while (w != nullptr)
//	{
//		if (w == this)
//		{
//			RemoveWindowFromList(this);
//
//			mNext = sFirst;
//			sFirst = this;
//
//			break;
//		}
//		w = w->mNext;
//	}
//}
//
//void MGtkWindowImpl::SetFocus(
//	uint32_t				inID)
//{
//	gtk_widget_grab_focus(GetWidget(inID));
//}
//
//string MGtkWindowImpl::GetText(
//	uint32_t				inID) const
//{
//	string result;
//	
//	GtkWidget* wdgt = GetWidget(inID);
//	if (GTK_IS_COMBO_BOX(wdgt))
//	{
//		char* text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(wdgt));
//		if (text != nullptr)
//		{
//			result = text;
//			g_free(text);
//		}
//	}
//	else if (GTK_IS_FONT_BUTTON(wdgt))
//		result = gtk_font_button_get_font_name(GTK_FONT_BUTTON(wdgt));
//	else if (GTK_IS_ENTRY(wdgt))
//		result = gtk_entry_get_text(GTK_ENTRY(wdgt));
//	else if (GTK_IS_TEXT_VIEW(wdgt))
//	{
//		GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(wdgt));
//		if (buffer == nullptr)
//			THROW(("Invalid text buffer"));
//		
//		GtkTextIter start, end;
//		gtk_text_buffer_get_bounds(buffer, &start, &end);
//		gchar* text = gtk_text_buffer_get_text(buffer, &start, &end, false);
//		
//		if (text != nullptr)
//		{
//			result = text;
//			g_free(text);
//		}
//	}
//	else
//		THROW(("item is not an entry"));
//	
//	return result;
//}

MHandler* MGtkWindowImpl::GetFocus()
{
	return nullptr;
}

//void MGtkWindowImpl::SetText(
//	uint32_t				inID,
//	const std::string&	inText)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	if (GTK_IS_COMBO_BOX(wdgt))
//assert(false);//		gtk_combo_box_set_active_text(GTK_COMBO_BOX(wdgt), inText.c_str());
//	else if (GTK_IS_FONT_BUTTON(wdgt))
//		gtk_font_button_set_font_name(GTK_FONT_BUTTON(wdgt), inText.c_str());
//	else if (GTK_IS_ENTRY(wdgt))
//		gtk_entry_set_text(GTK_ENTRY(wdgt), inText.c_str());
//	else if (GTK_IS_LABEL(wdgt))
//		gtk_label_set_text(GTK_LABEL(wdgt), inText.c_str());
//	else if (GTK_IS_BUTTON(wdgt))
//		gtk_button_set_label(GTK_BUTTON(wdgt), inText.c_str());
//	else if (GTK_IS_TEXT_VIEW(wdgt))
//	{
//		GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(wdgt));
//		if (buffer == nullptr)
//			THROW(("Invalid text buffer"));
//		gtk_text_buffer_set_text(buffer, inText.c_str(), inText.length());
//	}
//	else if (GTK_IS_PROGRESS_BAR(wdgt))
//	{
//		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(wdgt), inText.c_str());
//		gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(wdgt),
//			PANGO_ELLIPSIZE_MIDDLE);
//	}
//	else
//		THROW(("item is not an entry"));
//}
//
//void MGtkWindowImpl::SetPasswordField(
//	uint32_t				inID,
//	bool				isVisible)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	if (GTK_IS_ENTRY(wdgt))
//		g_object_set(G_OBJECT(wdgt), "visibility", isVisible, nullptr);
//	else
//		THROW(("item is not an entry"));
//}
//
//int32_t MGtkWindowImpl::GetValue(
//	uint32_t				inID) const
//{
//	int32_t result = 0;
//	GtkWidget* wdgt = GetWidget(inID);
//	
//	if (GTK_IS_COMBO_BOX(wdgt))
//		result = gtk_combo_box_get_active(GTK_COMBO_BOX(wdgt)) + 1;
//	else
//		THROW(("Cannot get value"));
//		
//	return result;
//}
//
//void MGtkWindowImpl::SetValue(
//	uint32_t				inID,
//	int32_t				inValue)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//
//	if (GTK_IS_COMBO_BOX(wdgt))
//		gtk_combo_box_set_active(GTK_COMBO_BOX(wdgt), inValue - 1);
//	else
//		THROW(("Cannot get value"));
//}
//
//// for comboboxes
//void MGtkWindowImpl::GetValues(
//	uint32_t				inID,
//	vector<string>& 	outValues) const
//{
//	assert(false);
//}
//
//void MGtkWindowImpl::SetValues(
//	uint32_t				inID,
//	const vector<string>&
//						inValues)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//
//	char name[5];
//	if (not GTK_IS_COMBO_BOX(wdgt))
//		THROW(("Item %s is not a combo box", IDToName(inID, name)));
//
//	GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(wdgt));
//	int32_t count = gtk_tree_model_iter_n_children(model, nullptr);
//
//	while (count-- > 0)
//		gtk_combo_box_remove_text(GTK_COMBO_BOX(wdgt), count);
//
//	for (vector<string>::const_iterator s = inValues.begin(); s != inValues.end(); ++s)
//		gtk_combo_box_append_text(GTK_COMBO_BOX(wdgt), s->c_str());
//
//	gtk_combo_box_set_active(GTK_COMBO_BOX(wdgt), 0);
//}
//
//void MGtkWindowImpl::SetColor(
//	uint32_t				inID,
//	MColor				inColor)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	if (not GTK_IS_COLOR_BUTTON(wdgt))
//		THROW(("Widget '%d' is not of the correct type", inID));
//	
//	GdkColor c = inColor;
//	gtk_color_button_set_color(GTK_COLOR_BUTTON(wdgt), &c);
//}
//
//MColor MGtkWindowImpl::GetColor(
//	uint32_t				inID) const
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	if (not GTK_IS_COLOR_BUTTON(wdgt))
//		THROW(("Widget '%d' is not of the correct type", inID));
//		
//	GdkColor c;
//	gtk_color_button_get_color(GTK_COLOR_BUTTON(wdgt), &c);
//	return MColor(c);
//}
//
//bool MGtkWindowImpl::IsChecked(
//	uint32_t				inID) const
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	if (not GTK_IS_TOGGLE_BUTTON(wdgt))
//		THROW(("Widget '%d' is not of the correct type", inID));
//	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdgt));
//}
//
//void MGtkWindowImpl::SetChecked(
//	uint32_t				inID,
//	bool				inOn)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	if (not GTK_IS_TOGGLE_BUTTON(wdgt))
//		THROW(("Widget '%d' is not of the correct type", inID));
//	return gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wdgt), inOn);
//}	
//
//bool MGtkWindowImpl::IsVisible(
//	uint32_t				inID) const
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	return GTK_WIDGET_VISIBLE(wdgt);
//}
//
//void MGtkWindowImpl::SetVisible(
//	uint32_t				inID,
//	bool				inVisible)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	if (inVisible)
//		gtk_widget_show(wdgt);
//	else
//		gtk_widget_hide(wdgt);
//}
//
//bool MGtkWindowImpl::IsEnabled(
//	uint32_t				inID) const
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	return GTK_WIDGET_IS_SENSITIVE(wdgt);
//}
//
//void MGtkWindowImpl::SetEnabled(
//	uint32_t				inID,
//	bool				inEnabled)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	gtk_widget_set_sensitive(wdgt, inEnabled);
//}
//
//bool MGtkWindowImpl::IsExpanded(
//	uint32_t				inID) const
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	assert(GTK_IS_EXPANDER(wdgt));
//	return gtk_expander_get_expanded(GTK_EXPANDER(wdgt));
//}
//
//void MGtkWindowImpl::SetExpanded(
//	uint32_t				inID,
//	bool				inExpanded)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	assert(GTK_IS_EXPANDER(wdgt));
//	gtk_expander_set_expanded(GTK_EXPANDER(wdgt), inExpanded);
//}
//
//void MGtkWindowImpl::SetProgressFraction(
//	uint32_t				inID,
//	float				inFraction)
//{
//	GtkWidget* wdgt = GetWidget(inID);
//	assert(GTK_IS_PROGRESS_BAR(wdgt));
//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(wdgt), inFraction);
//}
//
//void MGtkWindowImpl::ValueChanged(
//	uint32_t				inID)
//{
////	char name[5];
////	cout << "Value Changed for " << IDToName(inID, name) << endl;
//}

//void MGtkWindowImpl::Changed()
//{
//	const char* name = gtk_buildable_get_name(GTK_BUILDABLE(mChanged.GetSourceGObject()));
//	if (name != nullptr)
//	{
//		uint32_t id = 0;
//		for (uint32_t i = 0; i < 4 and name[i]; ++i)
//			id = (id << 8) | name[i];
//		
////		mWindow->ValueChanged(id);
//	}
//}

bool MGtkWindowImpl::DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, const string& inText)
{
	return false;
}

void MGtkWindowImpl::Invalidate(MRect inRect)
{
}

void MGtkWindowImpl::Validate(MRect inRect)
{
}

void MGtkWindowImpl::UpdateNow()
{
	// if (GTK_IS_WINDOW(GetWidget()))
	// 	gdk_window_process_updates(gtk_widget_get_window(GetWidget()), true);
}

void MGtkWindowImpl::ScrollRect(MRect inRect, int32_t inDeltaH, int32_t inDeltaV)
{
}

bool MGtkWindowImpl::GetMouse(int32_t& outX, int32_t& outY, uint32_t& outModifiers)
{
	return false;
}

bool MGtkWindowImpl::WaitMouseMoved(int32_t inX, int32_t inY)
{
	return false;
}

void MGtkWindowImpl::SetCursor(MCursor inCursor)
{
}

void MGtkWindowImpl::ObscureCursor()
{
}

void MGtkWindowImpl::ConvertToScreen(int32_t& ioX, int32_t& ioY) const
{
}

void MGtkWindowImpl::ConvertFromScreen(int32_t& ioX, int32_t& ioY) const
{
}

uint32_t MGtkWindowImpl::GetModifiers() const
{
	return 0;
}

// --------------------------------------------------------------------

MWindowImpl* MWindowImpl::Create(const string& inTitle, MRect inBounds,
	MWindowFlags inFlags, const string& inMenu, MWindow* inWindow)
{
	MGtkWindowImpl* result = new MGtkWindowImpl(inFlags, inMenu, inWindow);
	result->Create(inBounds, inTitle);
	return result;
}

void MWindow::GetMainScreenBounds(MRect& outRect)
{
	outRect = MRect(0, 0, 1024, 768);
}

