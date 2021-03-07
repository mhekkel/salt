//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <cassert>

#include "MGtkWidgetMixin.hpp"

using namespace std;

MGtkWidgetMixin::MGtkWidgetMixin()
	: mWidget(nullptr)
	, mOnDestroy(this, &MGtkWidgetMixin::OnDestroy)
	, mOnDelete(this, &MGtkWidgetMixin::OnDelete)
	, mOnShow(this, &MGtkWidgetMixin::OnShow)
	, mFocusInEvent(this, &MGtkWidgetMixin::OnFocusInEvent)
	, mFocusOutEvent(this, &MGtkWidgetMixin::OnFocusOutEvent)
	, mButtonPressEvent(this, &MGtkWidgetMixin::OnButtonPressEvent)
	, mMotionNotifyEvent(this, &MGtkWidgetMixin::OnMotionNotifyEvent)
	, mLeaveNotifyEvent(this, &MGtkWidgetMixin::OnLeaveNotifyEvent)
	, mButtonReleaseEvent(this, &MGtkWidgetMixin::OnButtonReleaseEvent)
	, mKeyPressEvent(this, &MGtkWidgetMixin::OnKeyPressEvent)
	, mKeyReleaseEvent(this, &MGtkWidgetMixin::OnKeyReleaseEvent)
	, mConfigureEvent(this, &MGtkWidgetMixin::OnConfigureEvent)
	, mScrollEvent(this, &MGtkWidgetMixin::OnScrollEvent)
	, mRealize(this, &MGtkWidgetMixin::OnRealize)
	, mDrawEvent(this, &MGtkWidgetMixin::OnDrawEvent)
	, mPopupMenu(this, &MGtkWidgetMixin::OnPopupMenu)
	, mDragDataReceived(this, &MGtkWidgetMixin::OnDragDataReceived)
	, mDragMotion(this, &MGtkWidgetMixin::OnDragMotion)
	, mDragLeave(this, &MGtkWidgetMixin::OnDragLeave)
	, mDragDataDelete(this, &MGtkWidgetMixin::OnDragDataDelete)
	, mDragDataGet(this, &MGtkWidgetMixin::OnDragDataGet)
	, mOnCommit(this, &MGtkWidgetMixin::OnCommit)
	, mOnDeleteSurrounding(this, &MGtkWidgetMixin::OnDeleteSurrounding)
	, mOnPreeditChanged(this, &MGtkWidgetMixin::OnPreeditChanged)
	, mOnPreeditStart(this, &MGtkWidgetMixin::OnPreeditStart)
	, mOnPreeditEnd(this, &MGtkWidgetMixin::OnPreeditEnd)
	, mOnRetrieveSurrounding(this, &MGtkWidgetMixin::OnRetrieveSurrounding)
	, mOnGrabBroken(this, &MGtkWidgetMixin::OnGrabBroken)
	, mRequestedWidth(-1)
	, mRequestedHeight(-1)
	, mAutoRepeat(false)
	, mDragWithin(false)
	, mIMContext(nullptr)
	, mNextKeyPressIsAutoRepeat(false)
{
}

MGtkWidgetMixin::~MGtkWidgetMixin()
{
	if (mWidget != nullptr)
		PRINT(("mWidget != null!"));
}

void MGtkWidgetMixin::CreateIMContext()
{
	if (mIMContext == nullptr)
	{
		mIMContext = gtk_im_context_simple_new();

		mOnCommit.Connect(G_OBJECT(mIMContext), "commit");
		mOnDeleteSurrounding.Connect(G_OBJECT(mIMContext), "delete-surrounding");
		mOnPreeditChanged.Connect(G_OBJECT(mIMContext), "preedit-changed");
		mOnPreeditStart.Connect(G_OBJECT(mIMContext), "preedit-start");
		mOnPreeditEnd.Connect(G_OBJECT(mIMContext), "preedit-end");
		mOnRetrieveSurrounding.Connect(G_OBJECT(mIMContext), "retrieve-surrounding");
	}
}

bool MGtkWidgetMixin::OnDestroy()
{
	mWidget = nullptr;
	return true;
}

bool MGtkWidgetMixin::OnDelete(GdkEvent* inEvent)
{
	PRINT(("MGtkWidgetMixin::OnDelete"));
	return true;
}

void MGtkWidgetMixin::OnShow()
{
	PRINT(("MGtkWidgetMixin::OnShow"));
}

void MGtkWidgetMixin::RequestSize(int32_t inWidth, int32_t inHeight)
{
	mRequestedWidth = inWidth;
	mRequestedHeight = inHeight;
	
	if (GTK_IS_WIDGET(mWidget) and (mRequestedWidth >= 0 or mRequestedHeight >= 0))
		gtk_widget_set_size_request(mWidget, mRequestedWidth, mRequestedHeight);
}

void MGtkWidgetMixin::SetWidget(GtkWidget* inWidget)
{
	mWidget = inWidget;
	
	if (inWidget != nullptr)
	{
		mOnDestroy.Connect(mWidget, "destroy");
		mOnDelete.Connect(mWidget, "delete_event");
		mOnShow.Connect(mWidget, "show");

		mFocusInEvent.Connect(mWidget, "focus-in-event");
		mFocusOutEvent.Connect(mWidget, "focus-out-event");
		mButtonPressEvent.Connect(mWidget, "button-press-event");
		mMotionNotifyEvent.Connect(mWidget, "motion-notify-event");
		mLeaveNotifyEvent.Connect(mWidget, "leave-notify-event");
		mButtonReleaseEvent.Connect(mWidget, "button-release-event");
		mKeyPressEvent.Connect(mWidget, "key-press-event");
		mKeyReleaseEvent.Connect(mWidget, "key-release-event");
		mConfigureEvent.Connect(mWidget, "configure-event");
		mScrollEvent.Connect(mWidget, "scroll-event");
		mRealize.Connect(mWidget, "realize");
		mPopupMenu.Connect(mWidget, "popup-menu");
		mDrawEvent.Connect(mWidget, "draw");
		// mExposeEvent.Connect(mWidget, "expose-event");
		mOnGrabBroken.Connect(mWidget, "grab-broken-event");
		
		if (mRequestedWidth >= 0 or mRequestedHeight >= 0)
			gtk_widget_set_size_request(inWidget, mRequestedWidth, mRequestedHeight);
	}
}

void MGtkWidgetMixin::Append(MGtkWidgetMixin* inChild, MControlPacking inPacking,
	bool inExpand, bool inFill, uint32_t inPadding)
{
	assert(false);
}

void MGtkWidgetMixin::SetFocus()
{
	if (gtk_widget_get_can_focus(mWidget))
		gtk_widget_grab_focus(mWidget);
}

void MGtkWidgetMixin::ReleaseFocus()
{
	PRINT(("MGtkWidgetMixin::ReleaseFocus"));
}

bool MGtkWidgetMixin::IsFocus() const
{
	return gtk_widget_has_focus(mWidget);
}

bool MGtkWidgetMixin::OnRealize()
{
	int m = gdk_window_get_events(gtk_widget_get_window(mWidget));

	m |= GDK_FOCUS_CHANGE_MASK | GDK_STRUCTURE_MASK |
		GDK_KEY_PRESS_MASK |
		GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK |
		GDK_LEAVE_NOTIFY_MASK
		| GDK_SCROLL_MASK
//		 | GDK_SMOOTH_SCROLL_MASK
		 ;
	gdk_window_set_events(gtk_widget_get_window(mWidget), (GdkEventMask)m);

	if (mIMContext)
		gtk_im_context_set_client_window(mIMContext, gtk_widget_get_window(mWidget));
		
	return false;
}

bool MGtkWidgetMixin::OnFocusInEvent(GdkEventFocus* inEvent)
{
	if (mIMContext)
		gtk_im_context_focus_in(mIMContext);

	return false;
}

bool MGtkWidgetMixin::OnFocusOutEvent(GdkEventFocus* inEvent)
{
	if (mIMContext)
		gtk_im_context_focus_out(mIMContext);

	return false;
}

bool MGtkWidgetMixin::IsActive() const
{
	return gtk_widget_has_focus(mWidget);
}

bool MGtkWidgetMixin::OnButtonPressEvent(GdkEventButton* inEvent)
{
	PRINT(("MGtkWidgetMixin::OnButtonPressEvent(%ld, %ld) [%s] ",
		static_cast<int32_t>(inEvent->x),
		static_cast<int32_t>(inEvent->y), G_OBJECT_TYPE_NAME(mWidget)));

	bool result = true;
	
	if (inEvent->button == 3)
		OnPopupMenu();
	else
	{
		result = false;
		
		uint32_t clickCount;
		switch (inEvent->type)
		{
			case GDK_BUTTON_PRESS:	clickCount = 1; break;
			case GDK_2BUTTON_PRESS:	clickCount = 2; break;
			case GDK_3BUTTON_PRESS:	clickCount = 3; break;
			default: PRINT(("Unknown event type for button press: %x", inEvent->type)); return false;
		}
		
		uint32_t modifiers = MapModifier(inEvent->state);
		int32_t x = static_cast<int32_t>(inEvent->x);
		int32_t y = static_cast<int32_t>(inEvent->y);
	
		if (OnMouseDown(x, y, inEvent->button, clickCount, modifiers))
		{
			if (gtk_grab_get_current() != GetWidget())
				gtk_grab_add(GetWidget());
			
			result = true;
		}
	}
	
	return result;
}

bool MGtkWidgetMixin::OnMotionNotifyEvent(GdkEventMotion* inEvent)
{
//	PRINT(("MGtkWidgetMixin::OnMotionNotifyEvent (%s)", G_OBJECT_TYPE_NAME(mWidget)));

	int32_t x, y;
	GdkModifierType state;
	
	if (inEvent->is_hint)
	{
#if GTK_CHECK_VERSION (3,20,0)
		auto seat = gdk_display_get_default_seat(gdk_display_get_default());
		auto mouse_device = gdk_seat_get_pointer(seat);
#else
		auto devman = gdk_display_get_device_manager(gdk_display_get_default());
		auto mouse_device = gdk_device_manager_get_client_pointer(devman);
#endif
		gdk_window_get_device_position(inEvent->window, mouse_device, &x, &y, &state);		
	}
	else
	{
		x = static_cast<int32_t>(inEvent->x);
		y = static_cast<int32_t>(inEvent->y);
		state = GdkModifierType(inEvent->state);
	}

	uint32_t modifiers = MapModifier(state);
	
	return OnMouseMove(x, y, modifiers);
}

bool MGtkWidgetMixin::OnLeaveNotifyEvent(GdkEventCrossing* inEvent)
{
//	PRINT(("MGtkWidgetMixin::OnLeaveNotifyEvent (%s)", G_OBJECT_TYPE_NAME(mWidget)));

	bool result = false;

	if (gtk_grab_get_current() != GetWidget())
		result = OnMouseExit();
	
	return result;
}

bool MGtkWidgetMixin::OnButtonReleaseEvent(GdkEventButton* inEvent)
{
//	PRINT(("MGtkWidgetMixin::OnButtonReleaseEvent (%s)", G_OBJECT_TYPE_NAME(mWidget)));

	gtk_grab_remove(GetWidget());

	uint32_t modifiers = MapModifier(inEvent->state);
	int32_t x = static_cast<int32_t>(inEvent->x);
	int32_t y = static_cast<int32_t>(inEvent->y);
	
	return OnMouseUp(x, y, modifiers);
}

bool MGtkWidgetMixin::OnGrabBroken(GdkEvent* inEvent)
{
	PRINT(("!!! MGtkWidgetMixin::OnGrabBroken (%s)", G_OBJECT_TYPE_NAME(mWidget)));
	gtk_grab_remove(GetWidget());
	return false;
}

bool MGtkWidgetMixin::OnMouseDown(int32_t inX, int32_t inY, uint32_t inButtonNr, uint32_t inClickCount, uint32_t inModifiers)
{
	return false;
}

bool MGtkWidgetMixin::OnMouseMove(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	return false;
}

bool MGtkWidgetMixin::OnMouseUp(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	return false;
}

bool MGtkWidgetMixin::OnMouseExit()
{
	return false;
}

bool MGtkWidgetMixin::OnKeyPressEvent(GdkEventKey* inEvent)
{
//	PRINT(("OnKeyPressEvent(kv=0x%x,m=0x%x,t=0x%x)", inEvent->keyval, inEvent->state, inEvent->time));
//
//	PRINT(("This is in %s", G_OBJECT_TYPE_NAME(mWidget)));

	bool result = false;
	
	if (mNextKeyPressIsAutoRepeat)
		mAutoRepeat = true;
	else
		mNextKeyPressIsAutoRepeat = true;

	if (mIMContext and gtk_im_context_filter_keypress(mIMContext, inEvent))
		result = true;

//	PRINT(("result is %s", result ? "true" : "false"));

	return result;
}

bool MGtkWidgetMixin::OnKeyReleaseEvent(GdkEventKey* inEvent)
{
	bool result = false;

	mAutoRepeat = false;
	mNextKeyPressIsAutoRepeat = false;

//	PRINT(("OnKeyReleaseEvent(kv=0x%x,m=0x%x,t=0x%x)", inEvent->keyval, inEvent->state, inEvent->time));

	return result;
}

bool MGtkWidgetMixin::OnConfigureEvent(GdkEventConfigure* inEvent)
{
	PRINT(("MGtkWidgetMixin::OnConfigureEvent in %s", G_OBJECT_TYPE_NAME(mWidget)));
	
	return false;
}

bool MGtkWidgetMixin::OnScrollEvent(GdkEventScroll* inEvent)
{
	PRINT(("MGtkWidgetMixin::OnScrollEvent"));
	return false;
}

// bool MGtkWidgetMixin::OnExposeEvent(GdkEventExpose* inEvent)
// {
// 	return false;
// }

bool MGtkWidgetMixin::OnDrawEvent(cairo_t* inCairo)
{
	return false;
}

// Drag and Drop support

void MGtkWidgetMixin::SetupDragAndDrop(const GtkTargetEntry inTargets[], uint32_t inTargetCount)
{
	gtk_drag_dest_set(mWidget, GTK_DEST_DEFAULT_ALL,
		inTargets, inTargetCount,
		GdkDragAction(GDK_ACTION_COPY|GDK_ACTION_MOVE));
	
	mDragDataReceived.Connect(mWidget, "drag-data-received");
	mDragMotion.Connect(mWidget, "drag-motion");
	mDragLeave.Connect(mWidget, "drag-leave");
	
	mDragDataGet.Connect(mWidget, "drag-data-get");
	mDragDataDelete.Connect(mWidget, "drag-data-delete");
	
	mDragWithin = false;
}

void MGtkWidgetMixin::OnDragDataReceived(GdkDragContext* inDragContext, gint inX, gint inY, GtkSelectionData* inData, guint inInfo, guint inTime)
{
	PRINT(("MGtkWidgetMixin::OnDragDataReceived"));
	bool ok = false;
	bool del = false;
	bool move = gdk_drag_context_get_selected_action(inDragContext) == GDK_ACTION_MOVE;
	
	if (gtk_selection_data_get_length(inData) >= 0)
	{
		ok = DragAccept(
			move,
			inX, inY,
			reinterpret_cast<const char*>(gtk_selection_data_get_data(inData)), gtk_selection_data_get_length(inData),
			inInfo);
		del = ok and move and gtk_drag_get_source_widget(inDragContext) != mWidget;
	}

	gtk_drag_finish(inDragContext, ok, del, inTime);
}

bool MGtkWidgetMixin::OnDragMotion(GdkDragContext* inDragContext, gint inX, gint inY, guint inTime)
{
	PRINT(("MGtkWidgetMixin::OnDragMotion"));
	
//	if (not mDragWithin)
//	{
//		DragEnter();
//		mDragWithin = true;
//	}
//	
//	if (DragWithin(inX, inY))
//	{
//		bool copy =
//			IsModifierDown(GDK_SHIFT_MASK) or
//			mWidget != gtk_drag_get_source_widget(inDragContext);
//		gdk_drag_status(inDragContext, copy ? GDK_ACTION_COPY : GDK_ACTION_MOVE, inTime);
//	}
//	else
//		gdk_drag_status(inDragContext, GdkDragAction(0), inTime);

	return false;
}

void MGtkWidgetMixin::OnDragLeave(GdkDragContext* inDragContext, guint inTime)
{
	PRINT(("MGtkWidgetMixin::OnDragLeave"));
	
	mDragWithin = false;
	DragLeave();
}

void MGtkWidgetMixin::OnDragDataDelete(GdkDragContext* inDragContext)
{
	PRINT(("MGtkWidgetMixin::OnDragDataDelete"));
	DragDeleteData();
}

void MGtkWidgetMixin::OnDragDataGet(GdkDragContext* inDragContext, GtkSelectionData* inData, guint inInfo, guint inTime)
{
	PRINT(("MGtkWidgetMixin::OnDragDataGet"));
	
	string data;
	
	DragSendData(data);
	
	gtk_selection_data_set_text(inData, data.c_str(), data.length());
}

void MGtkWidgetMixin::DragEnter()
{
}
	
bool MGtkWidgetMixin::DragWithin(int32_t inX, int32_t inY)
{
	return false;
}

void MGtkWidgetMixin::DragLeave()
{
}

bool MGtkWidgetMixin::DragAccept(bool inMove, int32_t inX, int32_t inY, const char* inData, uint32_t inLength, uint32_t inType)
{
	return false;
}

void MGtkWidgetMixin::DragBegin(const GtkTargetEntry inTargets[], uint32_t inTargetCount, GdkEventMotion* inEvent)
{
//	int button = 1;
//	
//	GtkTargetList* lst = gtk_target_list_new(inTargets, inTargetCount);
//	
////	GdkDragAction action = GDK_ACTION_MOVE;
////	if (inEvent->state & GDK_SHIFT_MASK)
////		action = GDK_ACTION_COPY;
////	
//	GdkDragContext* context = gtk_drag_begin(
//		mWidget, lst, GdkDragAction(GDK_ACTION_MOVE|GDK_ACTION_COPY),
//		button, (GdkEvent*)inEvent);
//
////	gtk_drag_set_icon_default(context);
//	MRect bounds;
//	GetBounds(bounds);
//	MDevice dev(this, bounds, true);
//	
//	GdkPixmap* pm = nullptr;
//	int32_t x, y;
//	GdkModifierType state;
//
//	if (inEvent->is_hint)
//		gdk_window_get_pointer(inEvent->window, &x, &y, &state);
//	else
//	{
//		x = static_cast<int32_t>(inEvent->x);
//		y = static_cast<int32_t>(inEvent->y);
//		state = GdkModifierType(inEvent->state);
//	}
//	
//	// only draw a transparent bitmap to drag around
//	// if we're on a composited screen.
//
//	GdkScreen* screen = gtk_widget_get_screen(mWidget);
//	if (gdk_screen_is_composited(screen))
//		DrawDragImage(pm, x, y);
//	
//	if (pm != nullptr)
//	{
//		int32_t w, h;
//		gdk_drawable_get_size(pm, &w, &h);
//		
//		gtk_drag_set_icon_pixmap(context, gdk_drawable_get_colormap(pm),
//			pm, nullptr, x, y);
//		
//		g_object_unref(pm);
//	}
//	else
//		gtk_drag_set_icon_default(context);
//
//	gtk_target_list_unref(lst);
}

void MGtkWidgetMixin::DragSendData(string& outData)
{
}

void MGtkWidgetMixin::DragDeleteData()
{
}

void MGtkWidgetMixin::GetMouse(int32_t& outX, int32_t& outY) const
{
	GdkModifierType modifiers;
	// gdk_window_get_pointer(gtk_widget_get_window(mWidget), nullptr, nullptr, &modifiers);

#if GTK_CHECK_VERSION (3,20,0)
	auto seat = gdk_display_get_default_seat(gdk_display_get_default());
	auto mouse_device = gdk_seat_get_pointer(seat);
#else
	auto devman = gdk_display_get_device_manager(gdk_display_get_default());
	auto mouse_device = gdk_device_manager_get_client_pointer(devman);
#endif
	gdk_window_get_device_position(gtk_widget_get_window(mWidget), mouse_device, &outX, &outY, &modifiers);		
	// gtk_widget_get_pointer(mWidget, &outX, &outY);
}

uint32_t MGtkWidgetMixin::GetModifiers() const
{
	GdkModifierType modifiers;
	// gdk_window_get_pointer(gtk_widget_get_window(mWidget), nullptr, nullptr, &modifiers);

#if GTK_CHECK_VERSION (3,20,0)
	auto seat = gdk_display_get_default_seat(gdk_display_get_default());
	auto mouse_device = gdk_seat_get_pointer(seat);
#else
	auto devman = gdk_display_get_device_manager(gdk_display_get_default());
	auto mouse_device = gdk_device_manager_get_client_pointer(devman);
#endif
	gdk_window_get_device_position(gtk_widget_get_window(mWidget), mouse_device, nullptr, nullptr, &modifiers);		

	return modifiers & gtk_accelerator_get_default_mod_mask();
}

void MGtkWidgetMixin::OnPopupMenu()
{
	PRINT(("Show Popup Menu"));
}

bool MGtkWidgetMixin::OnCommit(gchar* inText)
{
	PRINT(("MGtkWidgetMixin::OnCommit('%s')", inText));
	
	return false;
}

bool MGtkWidgetMixin::OnDeleteSurrounding(gint inStart, gint inLength)
{
	PRINT(("MGtkWidgetMixin::OnDeleteSurrounding"));
	return false;
}

bool MGtkWidgetMixin::OnPreeditChanged()
{
	PRINT(("MGtkWidgetMixin::OnPreeditChanged"));
	return false;
}

bool MGtkWidgetMixin::OnPreeditEnd()
{
	PRINT(("MGtkWidgetMixin::OnPreeditEnd"));
	return false;
}

bool MGtkWidgetMixin::OnPreeditStart()
{
	PRINT(("MGtkWidgetMixin::OnPreeditStart"));
	return false;
}

bool MGtkWidgetMixin::OnRetrieveSurrounding()
{
	PRINT(("MGtkWidgetMixin::OnRetrieveSurrounding"));
	return false;
}

//bool MView::OnRealize()
//{
//	int m = gdk_window_get_events(gtk_widget_get_window(mWidget));
//
//	m |= GDK_FOCUS_CHANGE_MASK | GDK_STRUCTURE_MASK |
//		GDK_KEY_PRESS_MASK |
//		GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK;
//	gdk_window_set_events(gtk_widget_get_window(mWidget), (GdkEventMask)m);
//	
//	return false;
//}
//
//bool MView::OnFocusInEvent(
//	GdkEventFocus* inEvent)
//{
//	return false;
//}
//
//bool MView::OnFocusOutEvent(
//	GdkEventFocus* inEvent)
//{
//	return false;
//}
//
//bool MView::IsActive() const
//{
//	return GTK_WIDGET_HAS_FOCUS(mWidget);
//}
//
//bool MView::OnButtonPressEvent(
//	GdkEventButton* inEvent)
//{
//	bool result = false;
//	
//	if (inEvent->button == 3 and inEvent->type == GDK_BUTTON_PRESS)
//	{
//		OnPopupMenu(inEvent);
//		result = true;
//	}
//	
//	return result;
//}
//
//bool MView::OnMotionNotifyEvent(
//	GdkEventMotion* inEvent)
//{
//	return false;
//}
//
//bool MView::OnKeyPressEvent(
//	GdkEventKey* inEvent)
//{
//	return false;
//}
//
//bool MView::OnButtonReleaseEvent(
//	GdkEventButton* inEvent)
//{
//	return false;
//}
//
//bool MView::OnConfigureEvent(
//	GdkEventConfigure* inEvent)
//{
//	return false;
//}
//
//bool MView::OnScrollEvent(
//	GdkEventScroll* inEvent)
//{
//	return false;
//}
//
//bool MView::OnExposeEvent(
//	GdkEventExpose* inEvent)
//{
//	MRect bounds;
//	GetBounds(bounds);
//
//	MRect update(inEvent->area);
//	
//	MDevice dev(this, bounds);
//	Draw(dev, update);
//
//	return true;
//}

// Drag and Drop support

//void MView::SetupDragAndDrop(
//	const GtkTargetEntry inTargets[],
//	uint32_t inTargetCount)
//{
//	gtk_drag_dest_set(mWidget, GTK_DEST_DEFAULT_ALL,
//		inTargets, inTargetCount,
//		GdkDragAction(GDK_ACTION_COPY|GDK_ACTION_MOVE));
//	
//	mDragDataReceived.Connect(mWidget, "drag-data-received");
//	mDragMotion.Connect(mWidget, "drag-motion");
//	mDragLeave.Connect(mWidget, "drag-leave");
//	
//	mDragDataGet.Connect(mWidget, "drag-data-get");
//	mDragDataDelete.Connect(mWidget, "drag-data-delete");
//	
//	mDragWithin = false;
//}
//
//void MView::OnDragDataReceived(
//	GdkDragContext* inDragContext,
//	gint inX,
//	gint inY,
//	GtkSelectionData* inData,
//	guint inInfo,
//	guint inTime)
//{
//	bool ok = false;
//	bool del = false;
//	bool move = inDragContext->action == GDK_ACTION_MOVE;
//	
//	if (inData->length >= 0)
//	{
//		ok = DragAccept(
//			move,
//			inX, inY,
//			reinterpret_cast<const char*>(inData->data), inData->length,
//			inInfo);
//		del = ok and move and gtk_drag_get_source_widget(inDragContext) != mWidget;
//	}
//
//	gtk_drag_finish(inDragContext, ok, del, inTime);
//}
//
//bool MView::OnDragMotion(
//	GdkDragContext* inDragContext,
//	gint inX,
//	gint inY,
//	guint inTime)
//{
//	if (not mDragWithin)
//	{
//		DragEnter();
//		mDragWithin = true;
//	}
//	
//	if (DragWithin(inX, inY))
//	{
//		bool copy =
//			IsModifierDown(GDK_SHIFT_MASK) or
//			mWidget != gtk_drag_get_source_widget(inDragContext);
//		gdk_drag_status(inDragContext, copy ? GDK_ACTION_COPY : GDK_ACTION_MOVE, inTime);
//	}
//	else
//		gdk_drag_status(inDragContext, GdkDragAction(0), inTime);
//
//	return false;
//}
//
//void MView::OnDragLeave(
//	GdkDragContext* inDragContext,
//	guint inTime)
//{
//	mDragWithin = false;
//	DragLeave();
//}
//
//void MView::OnDragDataDelete(
//	GdkDragContext* inDragContext)
//{
//	DragDeleteData();
//}
//
//void MView::OnDragDataGet(
//	GdkDragContext* inDragContext,
//	GtkSelectionData* inData,
//	guint inInfo,
//	guint inTime)
//{
//	string data;
//	
//	DragSendData(data);
//	
//	gtk_selection_data_set_text(inData, data.c_str(), data.length());
//}
//
//void MView::DragEnter()
//{
//}
//	
//bool MView::DragWithin(
//	int32_t inX,
//	int32_t inY)
//{
//	return false;
//}
//
//void MView::DragLeave()
//{
//}
//
//bool MView::DragAccept(
//	bool inMove,
//	int32_t inX,
//	int32_t inY,
//	const char* inData,
//	uint32_t inLength,
//	uint32_t inType)
//{
//	return false;
//}
//
//void MView::DragBegin(
//	const GtkTargetEntry inTargets[],
//	uint32_t inTargetCount,
//	GdkEventMotion* inEvent)
//{
//	int button = 1;
//	
//	GtkTargetList* lst = gtk_target_list_new(inTargets, inTargetCount);
//	
////	GdkDragAction action = GDK_ACTION_MOVE;
////	if (inEvent->state & GDK_SHIFT_MASK)
////		action = GDK_ACTION_COPY;
////	
//	GdkDragContext* context = gtk_drag_begin(
//		mWidget, lst, GdkDragAction(GDK_ACTION_MOVE|GDK_ACTION_COPY),
//		button, (GdkEvent*)inEvent);
//
////	gtk_drag_set_icon_default(context);
//	MRect bounds;
//	GetBounds(bounds);
//	MDevice dev(this, bounds, true);
//	
//	GdkPixmap* pm = nullptr;
//	int32_t x, y;
//	GdkModifierType state;
//
//	if (inEvent->is_hint)
//		gdk_window_get_pointer(inEvent->window, &x, &y, &state);
//	else
//	{
//		x = static_cast<int32_t>(inEvent->x);
//		y = static_cast<int32_t>(inEvent->y);
//		state = GdkModifierType(inEvent->state);
//	}
//	
//	// only draw a transparent bitmap to drag around
//	// if we're on a composited screen.
//
//	GdkScreen* screen = gtk_widget_get_screen(mWidget);
//	if (gdk_screen_is_composited(screen))
//		DrawDragImage(pm, x, y);
//	
//	if (pm != nullptr)
//	{
//		int32_t w, h;
//		gdk_drawable_get_size(pm, &w, &h);
//		
//		gtk_drag_set_icon_pixmap(context, gdk_drawable_get_colormap(pm),
//			pm, nullptr, x, y);
//		
//		g_object_unref(pm);
//	}
//	else
//		gtk_drag_set_icon_default(context);
//
//	gtk_target_list_unref(lst);
//}
//
//void MView::DragSendData(
//	string& outData)
//{
//}
//
//void MView::DragDeleteData()
//{
//}

