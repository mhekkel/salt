//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id$
	Copyright Drs M.L. Hekkelman
	Created 28-09-07 11:18:30
*/

#include "MGtkLib.h"

#include <iostream>
#include <cassert>

#include "MControls.h"
#include "MGtkCanvasImpl.h"
#include "MGtkWindowImpl.h"
#include "MGtkDeviceImpl.h"
#include "MPrimary.h"
#include "MUnicode.h"
#include "MUtils.h"
#include "MWindow.h"

#include "MControls.inl"
#include "MGtkControlsImpl.inl"

using namespace std;

MGtkCanvasImpl::MGtkCanvasImpl(MCanvas* inCanvas, uint32 inWidth, uint32 inHeight)
	: MGtkControlImpl(inCanvas, "canvas")
{
	RequestSize(inWidth, inHeight);
	
	CreateIMContext();
}

MGtkCanvasImpl::~MGtkCanvasImpl()
{
}

void MGtkCanvasImpl::CreateWidget()
{
	SetWidget(gtk_drawing_area_new());
	
	gtk_widget_set_can_focus(GetWidget(), true);
	
	g_object_set_data(G_OBJECT(GetWidget()), "m-canvas", this);
}

bool MGtkCanvasImpl::OnMouseDown(int32 inX, int32 inY, uint32 inButtonNr, uint32 inClickCount, uint32 inModifiers)
{
	bool result = false;
	
	// PRIMARY paste?
	switch (inButtonNr)
	{
		case 1:
			mControl->MouseDown(inX, inY, inClickCount, inModifiers);
			result = true;
			break;
		
		case 2:
			if (MPrimary::Instance().HasText())
			{
				string text;
				MPrimary::Instance().GetText(text);
				result = mControl->PastePrimaryBuffer(text);
			}
			break;
		
		case 3:
			PRINT(("Show Contextmenu!"));
			mControl->ShowContextMenu(inX, inY);
			break;
	}

	return result;
}

bool MGtkCanvasImpl::OnMouseMove(int32 inX, int32 inY, uint32 inModifiers)
{
	mControl->MouseMove(inX, inY, inModifiers);
	return true;
}

bool MGtkCanvasImpl::OnMouseUp(int32 inX, int32 inY, uint32 inModifiers)
{
	mControl->MouseUp(inX, inY, inModifiers);
	return true;
}

bool MGtkCanvasImpl::OnMouseExit()
{
	mControl->MouseExit();
	return true;
}

void MGtkCanvasImpl::Invalidate()
{
	if (GTK_IS_WIDGET(GetWidget()))
		gtk_widget_queue_draw(GetWidget());
}

bool MGtkCanvasImpl::OnConfigureEvent(GdkEventConfigure* inEvent)
{
	PRINT(("MGtkCanvasImpl::OnConfigureEvent"));

	MRect frame;
	mControl->GetFrame(frame);
	
	MRect bounds;

	GtkWidget* parent = gtk_widget_get_parent(GetWidget());

	if (GTK_IS_VIEWPORT(parent))
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(parent, &allocation);
		
		bounds.width = allocation.width;
		bounds.height = allocation.height;
		
		gtk_widget_translate_coordinates(parent, GetWidget(),
			bounds.x, bounds.y,
			&bounds.x, &bounds.y);
	}
	else
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(GetWidget(), &allocation);

		bounds.width = allocation.width;
		bounds.height = allocation.height;
	}

PRINT(("bounds(%d,%d,%d,%d)", bounds.x, bounds.y, bounds.width, bounds.height));

	mControl->ResizeFrame(bounds.width - frame.width, bounds.height - frame.height);
	
	return false;
}

bool MGtkCanvasImpl::OnKeyPressEvent(GdkEventKey* inEvent)
{
	bool result = MGtkControlImpl<MCanvas>::OnKeyPressEvent(inEvent);

	if (not result)
	{
		const uint32 kValidModifiersMask = gtk_accelerator_get_default_mod_mask();

PRINT(("OnKeyPressEvent(keyval=0x%x)", inEvent->keyval));
		
	    uint32 modifiers = MapModifier(inEvent->state & kValidModifiersMask);
		uint32 keyValue = MapKeyCode(inEvent->keyval);

		if (keyValue >= 0x60 and keyValue <= 0x7f and modifiers == kControlKey)
		{
			char ch = static_cast<char>(keyValue) - 0x60;
			string text(&ch, 1);
			result = mControl->HandleCharacter(text, mAutoRepeat);
		}
		else
			result = mControl->HandleKeyDown(keyValue, modifiers, mAutoRepeat);
		
		if (not result and modifiers == 0)
		{
			unicode ch = gdk_keyval_to_unicode(keyValue);
			
			if (ch != 0)
			{
				char s[8] = {};
				char* sp = s;
				uint32 length = MEncodingTraits<kEncodingUTF8>::WriteUnicode(sp, ch);
				
				string text(s, length);
				result = mControl->HandleCharacter(text, mAutoRepeat);
			}
		}
	}
	
	return result;
}

// bool MGtkCanvasImpl::OnExposeEvent(GdkEventExpose* inEvent)
// {
// 	MRect update(inEvent->area.x, inEvent->area.y, inEvent->area.width, inEvent->area.height);
	
// 	mControl->Draw(update);

// 	return true;
// }

bool MGtkCanvasImpl::OnDrawEvent(cairo_t* inCairo)
{
	mControl->Draw(inCairo);
	return true;
}

bool MGtkCanvasImpl::OnCommit(gchar* inText)
{
	string text(inText);
	return mControl->HandleCharacter(text, mAutoRepeat);
}

bool MGtkCanvasImpl::OnScrollEvent(GdkEventScroll* inEvent)
{
	int32 x = inEvent->x;
	int32 y = inEvent->y;
    uint32 modifiers = MapModifier(inEvent->state & kValidModifiersMask);
	
	switch (inEvent->direction)
	{
		case GDK_SCROLL_UP:
			mControl->MouseWheel(x, y, 0, 1, modifiers);
			break; 

		case GDK_SCROLL_DOWN:
			mControl->MouseWheel(x, y, 0, -1, modifiers);
			break; 

		case GDK_SCROLL_LEFT:
			mControl->MouseWheel(x, y, 1, 0, modifiers);
			break;

		case GDK_SCROLL_RIGHT:
			mControl->MouseWheel(x, y, -1, 0, modifiers);
			break; 
		
		case GDK_SCROLL_SMOOTH:
			mControl->MouseWheel(x, y, -inEvent->delta_x, -inEvent->delta_y, modifiers);
			break; 
	}	
		
	return true;
}

void MGtkCanvasImpl::AcceptDragAndDrop(bool inFiles, bool inText)
{
}

void MGtkCanvasImpl::StartDrag()
{
}

MCanvasImpl* MCanvasImpl::Create(MCanvas* inCanvas, uint32 inWidth, uint32 inHeight)
{
	return new MGtkCanvasImpl(inCanvas, inWidth, inHeight);
}


