//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MAcceleratorTable.hpp"

template<class CONTROL>
MGtkControlImpl<CONTROL>::MGtkControlImpl(CONTROL* inControl, const std::string& inLabel)
	: CONTROL::MImpl(inControl)
	, mChanged(this, &MGtkControlImpl<CONTROL>::OnChanged)
	, mLabel(inLabel)
{
}

template<class CONTROL>
MGtkControlImpl<CONTROL>::~MGtkControlImpl()
{
	if (GetWidget() != nullptr)
		gtk_widget_destroy(GetWidget());
}

template<class CONTROL>
bool MGtkControlImpl<CONTROL>::OnDestroy()
{
	if (this->mControl != nullptr)
	{
		SetWidget(nullptr);
		this->mControl->SetImpl(nullptr);

		delete this;
	}
	
	return true;
}

template<class CONTROL>
bool MGtkControlImpl<CONTROL>::IsFocus() const
{
	return GetWidget() != nullptr and gtk_widget_has_focus(GetWidget());
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::SetFocus()
{
	if (GetWidget() != nullptr)
		gtk_widget_grab_focus(GetWidget());
}

template<class CONTROL>
std::string MGtkControlImpl<CONTROL>::GetText() const
{
	return mLabel;
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::SetText(const std::string& inText)
{
	mLabel = inText;

	GtkWidget* wdgt = GetWidget();
	if (GTK_IS_ENTRY(wdgt))
		gtk_entry_set_text(GTK_ENTRY(wdgt), inText.c_str());
	else if (GTK_IS_LABEL(wdgt))
		gtk_label_set_text(GTK_LABEL(wdgt), inText.c_str());
	else if (GTK_IS_BUTTON(wdgt))
		gtk_button_set_label(GTK_BUTTON(wdgt), inText.c_str());
	else if (GTK_IS_TEXT_VIEW(wdgt))
	{
		GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(wdgt));
		if (buffer == nullptr)
			THROW(("Invalid text buffer"));
		gtk_text_buffer_set_text(buffer, inText.c_str(), inText.length());
	}
	else if (GTK_IS_PROGRESS_BAR(wdgt))
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(wdgt), inText.c_str());
		gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(wdgt),
			PANGO_ELLIPSIZE_MIDDLE);
	}
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::ActivateSelf()
{
//	if (GetWidget() != nullptr)
//		gtk_widget_set_state(GetWidget(), 
//			mControl->IsActive() and mControl->IsEnabled() ? GTK_STATE_ACTIVE : GTK_STATE_INSENSITIVE);
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::DeactivateSelf()
{
	if (GetWidget() != nullptr)
		gtk_widget_set_state_flags(GetWidget(), GTK_STATE_FLAG_INSENSITIVE, true);
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::EnableSelf()
{
//	::EnableWindow(GetWidget(), mControl->IsEnabled());
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::DisableSelf()
{
//	if (::IsWindowEnabled(GetWidget()))
//		::EnableWindow(GetWidget(), false);
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::ShowSelf()
{
	if (GetWidget() != nullptr)
		gtk_widget_show(GetWidget());
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::HideSelf()
{
	if (GetWidget() != nullptr)
		gtk_widget_hide(GetWidget());
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::FrameMoved()
{
//	FrameResized();
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::FrameResized()
{
//	if (GetWidget() != nullptr)
//	{
//		MRect bounds;
//		MGtkWidgetMixin* parent;
//		
//		GetParentAndBounds(parent, bounds);
//		
//		::MoveWindow(GetWidget(), bounds.x, bounds.y,
//			bounds.width, bounds.height, true);
//	}
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::MarginsChanged()
{
	auto widget = GetWidget();
	
	if (widget != nullptr)
	{
		int32_t l, t, r, b;

		MView* view = this->mControl;
		view->GetMargins(l, t, r, b);
		
		
		
		
//		MRect bounds;
//		MGtkWidgetMixin* parent;
//		
//		GetParentAndBounds(parent, bounds);
//		
//		::MoveWindow(GetWidget(), bounds.x, bounds.y,
//			bounds.width, bounds.height, true);
	}
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::GetParentAndBounds(MGtkWidgetMixin*& outParent, MRect& outBounds)
{
	MView* view = this->mControl;
	MView* parent = view->GetParent();
	
	view->GetBounds(outBounds);
	
	while (parent != nullptr)
	{
		view->ConvertToParent(outBounds.x, outBounds.y);
		
		MControlBase* control = dynamic_cast<MControlBase*>(parent);
		if (control != nullptr)
		{
			MGtkWidgetMixin* impl = dynamic_cast<MGtkWidgetMixin*>(control->GetControlImplBase());

			if (impl != nullptr and impl->GetWidget() != nullptr)
			{
				outParent = impl;
				break;
			}
		}
		
		MCanvas* canvas = dynamic_cast<MCanvas*>(parent);
		if (canvas != nullptr)
		{
			outParent = static_cast<MGtkCanvasImpl*>(canvas->GetImpl());
			break;
		}
		
		MWindow* window = dynamic_cast<MWindow*>(parent);
		if (window != nullptr)
		{
			outParent = static_cast<MGtkWindowImpl*>(window->GetImpl());
			break;
		}
		
		view = parent;
		parent = parent->GetParent();
	}
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::AddedToWindow()
{
	CreateWidget();

	MGtkWidgetMixin* parent;
	MRect bounds;
	
	GetParentAndBounds(parent, bounds);
	
	MControlBase* control = this->mControl;
	parent->Append(this, control->GetPacking(), control->GetExpand(), control->GetFill(), control->GetPadding());
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::OnChanged()
{
}

template<class CONTROL>
bool MGtkControlImpl<CONTROL>::OnKeyPressEvent(GdkEventKey* inEvent)
{
	bool result = MGtkWidgetMixin::OnKeyPressEvent(inEvent);
	
	if (not result)
	{
PRINT(("OnKeyPressEvent for %s", this->mControl->GetID().c_str()));

		const uint32_t kValidModifiersMask = gtk_accelerator_get_default_mod_mask();
		
	    uint32_t modifiers = MapModifier(inEvent->state & kValidModifiersMask);
		uint32_t keyValue = MapKeyCode(inEvent->keyval);
		uint32_t cmd;
		
		if (MAcceleratorTable::Instance().IsAcceleratorKey(keyValue, modifiers, cmd))
		{
			bool enabled = true, checked = false;
			if (this->mControl->UpdateCommandStatus(cmd, nullptr, 0, enabled, checked) and enabled)
				result = this->mControl->ProcessCommand(cmd, nullptr, 0, 0);
		}	
	}
	
	return result;
}

template<class CONTROL>
void MGtkControlImpl<CONTROL>::OnPopupMenu()
{
PRINT(("OnPopupMenu for %s", this->mControl->GetID().c_str()));

	int32_t x, y;

#if GTK_CHECK_VERSION (3,20,0)
	auto seat = gdk_display_get_default_seat(gdk_display_get_default());
	auto mouse_device = gdk_seat_get_pointer(seat);
#else
	auto devman = gdk_display_get_device_manager(gdk_display_get_default());
	auto mouse_device = gdk_device_manager_get_client_pointer(devman);
#endif

	auto window = gdk_display_get_default_group(gdk_display_get_default());
	gdk_window_get_device_position(window, mouse_device, &x, &y, NULL);
	// g_message ("pointer: %i %i", x, y);

	this->mControl->ShowContextMenu(x, y);
}

