//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include "MX11ControlsImpl.hpp"
#include "MX11WindowImpl.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MXcbCairoSurface.hpp"

using namespace std;

MXcbCairoSurface::MXcbCairoSurface(MView* inView)
	: mView(inView)
	, mSurface(nullptr), mCairo(nullptr)
	, mMouseIsIn(false)
{
}

MXcbCairoSurface::~MXcbCairoSurface()
{
}

void MXcbCairoSurface::AddedToWindow()
{
	MRect bounds;
	MXcbWinMixin* parent;
	
	GetParentAndBounds(parent, bounds);

	uint32 mask = 0;
	vector<uint32> values;

//	mask |= XCB_CW_OVERRIDE_REDIRECT;
//	values.push_back(1);
	
	mask |= XCB_CW_BACK_PIXEL;
	values.push_back(mScreen->white_pixel);
	
	mask |= XCB_CW_EVENT_MASK;
	values.push_back(
		XCB_EVENT_MASK_EXPOSURE              | XCB_EVENT_MASK_BUTTON_PRESS         |
		XCB_EVENT_MASK_BUTTON_RELEASE        | XCB_EVENT_MASK_POINTER_MOTION       |
		XCB_EVENT_MASK_ENTER_WINDOW          | XCB_EVENT_MASK_LEAVE_WINDOW         |
		XCB_EVENT_MASK_KEY_PRESS             | XCB_EVENT_MASK_KEY_RELEASE          |
		XCB_EVENT_MASK_VISIBILITY_CHANGE     | XCB_EVENT_MASK_STRUCTURE_NOTIFY     |
//		XCB_EVENT_MASK_RESIZE_REDIRECT       | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY  |
//		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE         |
		XCB_EVENT_MASK_PROPERTY_CHANGE       //| XCB_EVENT_MASK_COLOR_MAP_CHANGE     |
//		XCB_EVENT_MASK_OWNER_GRAB_BUTTON
	);

	auto cookie0 = xcb_create_window_checked(mConnection, XCB_COPY_FROM_PARENT,
		mWindowID, parent->GetWindowID(),
		bounds.x, bounds.y, bounds.width, bounds.height,
		0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		XCB_COPY_FROM_PARENT, mask, &values[0]);

	xcb_generic_error_t *error;
	if ((error = xcb_request_check(mConnection, cookie0))) {
		free(error);
		throw runtime_error("could not create control");
	}

	/* Map the window on the screen */
	xcb_map_window(mConnection, mWindowID);
	
	xcb_visualtype_t* visual = nullptr;
    xcb_depth_iterator_t d = xcb_screen_allowed_depths_iterator(mScreen);
    for (; visual == nullptr and d.rem; xcb_depth_next(&d))
    {
    	xcb_visualtype_iterator_t v = xcb_depth_visuals_iterator(d.data);
    	for (; visual == nullptr and v.rem; xcb_visualtype_next(&v))
		    if (v.data->visual_id == mScreen->root_visual)
				visual = v.data;
    }
	
	mSurface = cairo_xcb_surface_create(mConnection, mWindowID, visual, bounds.width, bounds.height);
	mCairo = cairo_create(mSurface);
	
//	PRINT(("surface status = %d", cairo_surface_status(mSurface)));

	CreateWidget();
}

void MXcbCairoSurface::Invalidate()
{
	cairo_save(mCairo);

	MRect bounds;
	mView->GetBounds(bounds);

	cairo_set_source_rgb(mCairo, 1.0, 1.0, 1.0);
	cairo_rectangle(mCairo, bounds.x, bounds.y, bounds.width, bounds.height);
	cairo_fill(mCairo);

	DrawWidget();
	
	try
	{
		DrawWidget();
	}
	catch (...) {}

	cairo_surface_flush(mSurface);
	
	cairo_restore(mCairo);
	
	xcb_flush(mConnection);
}

void MXcbCairoSurface::FrameResized()
{
}

void MXcbCairoSurface::GetParentAndBounds(MXcbWinMixin*& outParent, MRect& outBounds)
{
	MView* view = mView;
	MView* parent = view->GetParent();
	
	view->GetBounds(outBounds);
	
	while (parent != nullptr)
	{
		view->ConvertToParent(outBounds.x, outBounds.y);
		
		MControlBase* control = dynamic_cast<MControlBase*>(parent);
		if (control != nullptr)
		{
			MXcbWinMixin* impl = dynamic_cast<MXcbWinMixin*>(control->GetControlImplBase());

			if (impl != nullptr/* and impl->GetWidget() != nullptr*/)
			{
				outParent = impl;
				break;
			}
		}
		
//		MCanvas* canvas = dynamic_cast<MCanvas*>(parent);
//		if (canvas != nullptr)
//		{
//			outParent = static_cast<MGtkCanvasImpl*>(canvas->GetImpl());
//			break;
//		}
//		
		MWindow* window = dynamic_cast<MWindow*>(parent);
		if (window != nullptr)
		{
			outParent = static_cast<MX11WindowImpl*>(window->GetImpl());
			break;
		}
		
		view = parent;
		parent = parent->GetParent();
	}
}



void MXcbCairoSurface::CreateWidget()
{
}

void MXcbCairoSurface::DrawWidget()
{
}

void MXcbCairoSurface::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	Invalidate();
}

void MXcbCairoSurface::ButtonReleaseEvent(xcb_key_release_event_t* inEvent)
{
	Invalidate();
}

void MXcbCairoSurface::MotionNotifyEvent(xcb_motion_notify_event_t* inEvent)
{
}

void MXcbCairoSurface::EnterNotifyEvent(xcb_enter_notify_event_t* inEvent)
{
	if (not mMouseIsIn) PRINT(("Mouse is now In"));
	
	mMouseIsIn = true;
	
	Invalidate();
}

void MXcbCairoSurface::LeaveNotifyEvent(xcb_leave_notify_event_t* inEvent)
{
	if (mMouseIsIn) PRINT(("Mouse is now Out"));

	mMouseIsIn = false;

	Invalidate();
}

void MXcbCairoSurface::ExposeEvent(xcb_expose_event_t* inEvent)
{
	cairo_save(mCairo);
	
	try
	{
		cairo_rectangle(mCairo, inEvent->x, inEvent->y, inEvent->width, inEvent->height);
		cairo_clip(mCairo);
		
		DrawWidget();
	}
	catch (...) {}

	cairo_surface_flush(mSurface);
	
	cairo_restore(mCairo);
	
	xcb_flush(mConnection);
}

void MXcbCairoSurface::ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent)
{
	if (mSurface != nullptr)
		cairo_xcb_surface_set_size(mSurface, inEvent->width, inEvent->height);

	PRINT(("New size is (%d,%d)", inEvent->width, inEvent->height));
}

void MXcbCairoSurface::SetColor(MColor inColor)
{
	cairo_set_source_rgb(mCairo, inColor.red / 255.0, inColor.green / 255.0, inColor.blue / 255.0);
}

void MXcbCairoSurface::RoundedRectanglePath(MRect inBounds, uint32 inRadius)
{
	if (inBounds.empty())
		return;
	
	cairo_new_path(mCairo);
	
	if (inRadius > inBounds.width / 2)
		inRadius = inBounds.width / 2;
	
	if (inRadius > inBounds.height / 2)
		inRadius = inBounds.height / 2;
	
	double l = inBounds.x;
	double r = inBounds.x + inBounds.width;
	double t = inBounds.y;
	double b = inBounds.y + inBounds.height;

	double midX = (l + r) / 2;
	double deltaC = inRadius;
	
	cairo_move_to(mCairo, midX, t);
	cairo_line_to(mCairo, r - deltaC, t);
	cairo_arc(mCairo, r - deltaC, t + deltaC, deltaC, M_PI + M_PI / 2, 0);
	cairo_line_to(mCairo, r, b - deltaC);
	cairo_arc(mCairo, r - deltaC, b - deltaC, deltaC, 0, M_PI / 2);
	cairo_line_to(mCairo, l + deltaC, b);
	cairo_arc(mCairo, l + deltaC, b - deltaC, deltaC, M_PI / 2, M_PI);
	cairo_line_to(mCairo, l, t + deltaC);
	cairo_arc(mCairo, l + deltaC, t + deltaC, deltaC, M_PI, M_PI + M_PI / 2);
	cairo_close_path(mCairo);
}

void MXcbCairoSurface::FillRoundedRectangle(MRect inBounds, uint32 inRadius)
{
	RoundedRectanglePath(inBounds, inRadius);
	cairo_fill(mCairo);
}

void MXcbCairoSurface::StrokeRoundedRectangle(MRect inBounds, uint32 inRadius)
{
	RoundedRectanglePath(inBounds, inRadius);
	cairo_stroke(mCairo);
}

void MXcbCairoSurface::SetFont(const string& inFont, double inSize,
	MFontSlant inSlant, MFontWeight inWeight)
{
	cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
	switch (inSlant)
	{
		case fsNormal:	slant = CAIRO_FONT_SLANT_NORMAL; break;
		case fsItalic:	slant = CAIRO_FONT_SLANT_ITALIC; break;
		case fsOblique:	slant = CAIRO_FONT_SLANT_OBLIQUE; break;
	}

	cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
	if (inWeight == fwBold)
		weight = CAIRO_FONT_WEIGHT_BOLD;
	
	cairo_select_font_face(mCairo, inFont.c_str(), slant, weight);
	cairo_set_font_size(mCairo, inSize);
}

void MXcbCairoSurface::DrawString(MRect inBounds, const string& inText)
{
	cairo_font_extents_t fontExtends;
	cairo_font_extents(mCairo, &fontExtends);
	
	double y = inBounds.y + (inBounds.height - fontExtends.height) / 2 +
				fontExtends.ascent;
	
	cairo_text_extents_t textExtends;
	cairo_text_extents(mCairo, inText.c_str(), &textExtends);
	
	double x = inBounds.x + (inBounds.width - textExtends.width) / 2;
	
	cairo_move_to(mCairo, x, y);
	cairo_show_text(mCairo, inText.c_str());
}
