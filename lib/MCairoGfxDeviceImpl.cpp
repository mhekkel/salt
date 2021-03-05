/* 
   Created by: Maarten L. Hekkelman
   Date: woensdag 09 januari, 2019
*/

#include "MLib.hpp"

#include <cairo/cairo.h>
#include <cmath>

#include "MColor.hpp"
#include "MCairoGfxDeviceImpl.hpp"

using namespace std;

// --------------------------------------------------------------------

MCairoGfxDeviceImpl::MCairoGfxDeviceImpl(cairo_t* inCairo)
	: mCairo(inCairo)
{
	cairo_save(mCairo);
//	cairo_reset_clip(mCairo);
}

MCairoGfxDeviceImpl::~MCairoGfxDeviceImpl()
{
	cairo_restore(mCairo);
	cairo_surface_flush(cairo_get_target(mCairo));
}

void MCairoGfxDeviceImpl::ClipRect(float x, float y, float width, float height)
{
	cairo_new_path(mCairo);
	cairo_rectangle(mCairo, x, y, width, height);
	cairo_clip(mCairo);
}

void MCairoGfxDeviceImpl::SetColorRGB(float r, float g, float b)
{
	cairo_set_source_rgb(mCairo, r, g, b);
}

void MCairoGfxDeviceImpl::SetLineWidth(float lineWidth)
{
	cairo_set_line_width(mCairo, lineWidth);
}

void MCairoGfxDeviceImpl::MoveTo(float x, float y)
{
	cairo_move_to(mCairo, x, y);
}

void MCairoGfxDeviceImpl::LineTo(float x, float y)
{
	cairo_line_to(mCairo, x, y);
}

void MCairoGfxDeviceImpl::Stroke()
{
	cairo_stroke(mCairo);
}

void MCairoGfxDeviceImpl::FillRect(float x, float y, float width, float height)
{
	cairo_rectangle(mCairo, x, y, width, height);
	cairo_fill(mCairo);
}

void MCairoGfxDeviceImpl::StrokeRect(float x, float y, float width, float height)
{
	cairo_rectangle(mCairo, x, y, width, height);
	cairo_stroke(mCairo);
}

void MCairoGfxDeviceImpl::CreateRoundedRectPath(float x, float y, float width, float height, float radius)
{
	if (width <= 0 or height <= 0)
		return;
	
	cairo_new_path(mCairo);
	
	if (radius > width / 2)
		radius = width / 2;
	
	if (radius > height / 2)
		radius = height / 2;
	
	double l = x;
	double r = x + width;
	double t = y;
	double b = y + height;

	double midX = (l + r) / 2;
	double deltaC = radius;
	
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

void MCairoGfxDeviceImpl::FillRoundedRect(float x, float y, float width, float height, float radius)
{
	CreateRoundedRectPath(x, y, width, height, radius);
	cairo_fill(mCairo);
}

void MCairoGfxDeviceImpl::StrokeRoundedRect(float x, float y, float width, float height, float radius)
{
	CreateRoundedRectPath(x, y, width, height, radius);
	cairo_stroke(mCairo);
}

void MCairoGfxDeviceImpl::FillPoly(std::initializer_list<std::tuple<float,float>> pts)
{
	cairo_new_path(mCairo);
	
	auto i = pts.begin();
	cairo_move_to(mCairo, get<0>(*i), get<1>(*i));
	
	while (++i != pts.end())
		cairo_line_to(mCairo, get<0>(*i), get<1>(*i));
	
	cairo_close_path(mCairo);
	
	cairo_fill(mCairo);
}

void MCairoGfxDeviceImpl::StrokePoly(std::initializer_list<std::tuple<float,float>> pts)
{
	cairo_new_path(mCairo);
	
	auto i = pts.begin();
	cairo_move_to(mCairo, get<0>(*i), get<1>(*i));
	
	while (++i != pts.end())
		cairo_line_to(mCairo, get<0>(*i), get<1>(*i));
	
	cairo_close_path(mCairo);
	
	cairo_fill(mCairo);
}

void MCairoGfxDeviceImpl::SetFont(const char* inFontName, float inSize, bool inBold, bool inItalic)
{
	cairo_select_font_face(mCairo, inFontName,
		inItalic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
		inBold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL );
	cairo_set_font_size(mCairo, inSize);
}

FontExtents MCairoGfxDeviceImpl::GetFontExtents()
{
	cairo_font_extents_t t;
	cairo_font_extents(mCairo, &t);
	
	return {
		static_cast<float>(t.ascent),
		static_cast<float>(t.descent),
		static_cast<float>(t.height),
		static_cast<float>(t.max_x_advance),
		static_cast<float>(t.max_y_advance)
	};
}

TextExtents MCairoGfxDeviceImpl::GetTextExtents(const char* inText)
{
	cairo_text_extents_t t;
	cairo_text_extents(mCairo, inText, &t);
	
	return {
		static_cast<float>(t.x_bearing),
		static_cast<float>(t.y_bearing),
		static_cast<float>(t.width),
		static_cast<float>(t.height),
		static_cast<float>(t.x_advance),
		static_cast<float>(t.y_advance)
	};
}

void MCairoGfxDeviceImpl::ShowText(const char* inText)
{
	cairo_show_text(mCairo, inText);
}

// --------------------------------------------------------------------

MCairoImageGfxDeviceImpl::MCairoImageGfxDeviceImpl(int width, int height)
{
	mSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	mCairo = cairo_create(mSurface);
}

MCairoImageGfxDeviceImpl::~MCairoImageGfxDeviceImpl()
{
	cairo_destroy(mCairo);
	cairo_surface_destroy(mSurface);
}

// --------------------------------------------------------------------

MGfxDevice::MGfxDevice(int width, int height)
	: MGfxDevice(new MCairoImageGfxDeviceImpl(width, height))
{
}
