//          Copyright Maarten L. Hekkelman 2006-2015
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id$
	Copyright Drs M.L. Hekkelman
	Created 28-09-07 11:18:30
*/

#include "MX11Lib.hpp"

#include <iostream>
#include <cassert>

#include "MControls.hpp"
#include "MX11CanvasImpl.hpp"
#include "MXcbWindowImpl.hpp"
#include "MX11DeviceImpl.hpp"
#include "MPrimary.hpp"
#include "MUnicode.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include "MControls.inl"
//#include "MX11ControlsImpl.inl"

using namespace std;

//namespace
//{
//
//GdkCursor* gGdkCursors[eCursorCount];
//
//}
//
//MX11CanvasImpl::MX11CanvasImpl(MCanvas* inCanvas, uint32 inWidth, uint32 inHeight)
//	: MX11ControlImpl(inCanvas, "canvas")
//{
//	RequestSize(inWidth, inHeight);
//	
//	CreateIMContext();
//}
//
//MX11CanvasImpl::~MX11CanvasImpl()
//{
//}
//
//void MX11CanvasImpl::CreateWidget()
//{
//	SetWidget(gtk_drawing_area_new());
//	
//	GTK_WIDGET_SET_FLAGS(GetWidget(), GTK_CAN_FOCUS);
//	
//	g_object_set_data(G_OBJECT(GetWidget()), "m-canvas", this);
//}
//
//bool MX11CanvasImpl::OnMouseDown(int32 inX, int32 inY, uint32 inButtonNr, uint32 inClickCount, uint32 inModifiers)
//{
//	bool result = false;
//	
//	// PRIMARY paste?
//	switch (inButtonNr)
//	{
//		case 1:
//			mControl->MouseDown(inX, inY, inClickCount, inModifiers);
//			result = true;
//			break;
//		
//		case 2:
//			if (MPrimary::Instance().HasText())
//			{
//				string text;
//				MPrimary::Instance().GetText(text);
//				result = mControl->PastePrimaryBuffer(text);
//			}
//			break;
//	}
//
//	return result;
//}
//
//bool MX11CanvasImpl::OnMouseMove(int32 inX, int32 inY, uint32 inModifiers)
//{
//	mControl->MouseMove(inX, inY, inModifiers);
//	return true;
//}
//
//bool MX11CanvasImpl::OnMouseUp(int32 inX, int32 inY, uint32 inModifiers)
//{
//	mControl->MouseUp(inX, inY, inModifiers);
//	return true;
//}
//
//bool MX11CanvasImpl::OnMouseExit()
//{
//	mControl->MouseExit();
//	return true;
//}
//
//void MX11CanvasImpl::Invalidate()
//{
//	if (GTK_IS_WIDGET(GetWidget()))
//		gtk_widget_queue_draw(GetWidget());
//}
//
//bool MX11CanvasImpl::OnConfigureEvent(GdkEventConfigure* inEvent)
//{
//	PRINT(("MX11CanvasImpl::OnConfigureEvent"));
//
//	MRect frame;
//	mControl->GetFrame(frame);
//	
//	MRect bounds;
//
//	GtkWidget* parent = gtk_widget_get_parent(GetWidget());
//
//	if (GTK_IS_VIEWPORT(parent))
//	{
//		bounds.width = parent->allocation.width;
//		bounds.height = parent->allocation.height;
//		
//		gtk_widget_translate_coordinates(parent, GetWidget(),
//			bounds.x, bounds.y,
//			&bounds.x, &bounds.y);
//	}
//	else
//	{
//		bounds.width = GetWidget()->allocation.width;
//		bounds.height = GetWidget()->allocation.height;
//	}
//
//PRINT(("bounds(%d,%d,%d,%d)", bounds.x, bounds.y, bounds.width, bounds.height));
//
//	mControl->ResizeFrame(bounds.width - frame.width, bounds.height - frame.height);
//	
//	return false;
//}
//
//bool MX11CanvasImpl::OnKeyPressEvent(GdkEventKey* inEvent)
//{
//	bool result = MX11ControlImpl<MCanvas>::OnKeyPressEvent(inEvent);
//
//	if (not result)
//	{
//		const uint32 kValidModifiersMask = gtk_accelerator_get_default_mod_mask();
//
//PRINT(("OnKeyPressEvent(keyval=0x%x)", inEvent->keyval));
//		
//	    uint32 modifiers = MapModifier(inEvent->state & kValidModifiersMask);
//		uint32 keyValue = MapKeyCode(inEvent->keyval);
//
//		if (keyValue >= 0x60 and keyValue <= 0x7f and modifiers == kControlKey)
//		{
//			char ch = static_cast<char>(keyValue) - 0x60;
//			string text(&ch, 1);
//			result = mControl->HandleCharacter(text, mAutoRepeat);
//		}
//		else
//			result = mControl->HandleKeyDown(keyValue, modifiers, mAutoRepeat);
//		
//		if (not result and modifiers == 0)
//		{
//			unicode ch = gdk_keyval_to_unicode(keyValue);
//			
//			if (ch != 0)
//			{
//				char s[8] = {};
//				char* sp = s;
//				uint32 length = MEncodingTraits<kEncodingUTF8>::WriteUnicode(sp, ch);
//				
//				string text(s, length);
//				result = mControl->HandleCharacter(text, mAutoRepeat);
//			}
//		}
//	}
//	
//	return result;
//}
//
//bool MX11CanvasImpl::OnExposeEvent(GdkEventExpose* inEvent)
//{
//	MRect bounds;
//	mControl->GetBounds(bounds);
//
//	MRect update(inEvent->area.x, inEvent->area.y, inEvent->area.width, inEvent->area.height);
//	
//	mControl->Draw(update);
//
//	return true;
//}
//
//bool MX11CanvasImpl::OnCommit(gchar* inText)
//{
//	string text(inText);
//	return mControl->HandleCharacter(text, mAutoRepeat);
//}
//
//bool MX11CanvasImpl::OnScrollEvent(GdkEventScroll* inEvent)
//{
//	int32 x = inEvent->x;
//	int32 y = inEvent->y;
//    uint32 modifiers = MapModifier(inEvent->state & kValidModifiersMask);
//	
//	switch (inEvent->direction)
//	{
//		case GDK_SCROLL_UP:
//			mControl->MouseWheel(x, y, 0, 1, modifiers);
//			break; 
//
//		case GDK_SCROLL_DOWN:
//			mControl->MouseWheel(x, y, 0, -1, modifiers);
//			break; 
//
//		case GDK_SCROLL_LEFT:
//			mControl->MouseWheel(x, y, 1, 0, modifiers);
//			break;
//
//		case GDK_SCROLL_RIGHT:
//			mControl->MouseWheel(x, y, -1, 0, modifiers);
//			break; 
//	}	
//		
//	return true;
//}
//
//void MX11CanvasImpl::AcceptDragAndDrop(bool inFiles, bool inText)
//{
//}
//
//void MX11CanvasImpl::StartDrag()
//{
//}
//

MCanvasImpl* MCanvasImpl::Create(MCanvas* inCanvas, uint32 inWidth, uint32 inHeight)
{
	return new MX11CanvasImpl(inCanvas, inWidth, inHeight);
}


