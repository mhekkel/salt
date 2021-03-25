//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

#include "MError.hpp"
#include "MGtkCanvasImpl.hpp"
#include "MGtkDeviceImpl.hpp"
#include "MUnicode.hpp"
#include "MView.hpp"
#include "MWindow.hpp"

using namespace std;

struct MPangoContext
{
	MPangoContext()
		: mPangoContext(pango_font_map_create_context(PANGO_FONT_MAP(pango_cairo_font_map_get_default())))
	{
	}

	~MPangoContext() { g_object_unref(mPangoContext); }

	static MPangoContext &instance()
	{
		static MPangoContext sPangoContext;
		return sPangoContext;
	}

	PangoContext *mPangoContext;
};

struct MPangoLanguage
{
	MPangoLanguage()
	{
		const char *LANG = getenv("LANG");

		if (LANG != nullptr)
			mPangoLanguage = pango_language_from_string(LANG);
		else
			mPangoLanguage = gtk_get_default_language();
	}
	~MPangoLanguage() { g_object_unref(mPangoLanguage); }

	PangoLanguage *mPangoLanguage;
};

MGtkDeviceImpl::MGtkDeviceImpl()
	: MGtkDeviceImpl(pango_layout_new(MPangoContext::instance().mPangoContext))

{
}

MGtkDeviceImpl::MGtkDeviceImpl(
	PangoLayout *inLayout)
	: mPangoLayout(inLayout)
	, mFont(nullptr)
	, mMetrics(nullptr)
{
	mPangoScale = PANGO_SCALE;

	auto display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

	double res = DisplayWidth(display, 0) * 25.4 / DisplayWidthMM(display, 0);

	mPangoScale = lrint(PANGO_SCALE * (96 / res));

	pango_cairo_font_map_set_resolution((PangoCairoFontMap *)pango_cairo_font_map_get_default(), res);
}

MGtkDeviceImpl::~MGtkDeviceImpl()
{
	if (mMetrics != nullptr)
		pango_font_metrics_unref(mMetrics);

	if (mFont != nullptr)
		pango_font_description_free(mFont);

	if (mPangoLayout != nullptr)
		g_object_unref(mPangoLayout);
}

void MGtkDeviceImpl::Save()
{
}

void MGtkDeviceImpl::Restore()
{
}

void MGtkDeviceImpl::SetOrigin(
	int32_t inX,
	int32_t inY)
{
}

void MGtkDeviceImpl::SetFont(
	const string &inFont)
{
	PangoFontDescription *newFontDesc = pango_font_description_from_string(inFont.c_str());

	if (mFont == nullptr or newFontDesc == nullptr or
	    not pango_font_description_equal(mFont, newFontDesc))
	{
		if (mMetrics != nullptr)
			pango_font_metrics_unref(mMetrics);

		if (mFont != nullptr)
			pango_font_description_free(mFont);

		mFont = newFontDesc;

		if (mFont != nullptr)
		{
			mMetrics = GetMetrics();

			pango_layout_set_font_description(mPangoLayout, mFont);
			GetWhiteSpaceGlyphs(mSpaceGlyph, mTabGlyph, mNewLineGlyph);
		}
	}
	else
		pango_font_description_free(newFontDesc);
}

void MGtkDeviceImpl::SetForeColor(
	MColor inColor)
{
}

MColor MGtkDeviceImpl::GetForeColor() const
{
	return kBlack;
}

void MGtkDeviceImpl::SetBackColor(
	MColor inColor)
{
}

MColor MGtkDeviceImpl::GetBackColor() const
{
	return kWhite;
}

void MGtkDeviceImpl::ClipRect(
	MRect inRect)
{
}

//void MGtkDeviceImpl::ClipRegion(
//	MRegion				inRegion)
//{
//}

void MGtkDeviceImpl::EraseRect(
	MRect inRect)
{
}

void MGtkDeviceImpl::FillRect(
	MRect inRect)
{
}

void MGtkDeviceImpl::StrokeRect(
	MRect inRect,
	uint32_t inLineWidth)
{
}

void MGtkDeviceImpl::FillEllipse(
	MRect inRect)
{
}

void MGtkDeviceImpl::DrawImage(
	cairo_surface_t *inImage,
	float inX,
	float inY,
	float inShear)
{
}

void MGtkDeviceImpl::CreateAndUsePattern(
	MColor inColor1,
	MColor inColor2)
{
}

PangoFontMetrics *MGtkDeviceImpl::GetMetrics()
{
	if (mMetrics == nullptr)
	{
		PangoContext *context = pango_layout_get_context(mPangoLayout);

		PangoFontDescription *fontDesc = mFont;
		if (fontDesc == nullptr)
		{
			fontDesc = pango_context_get_font_description(context);

			// there's a bug in pango I guess

			int32_t x;
			if (IsPrinting(x))
				fontDesc = pango_font_description_copy(fontDesc);
		}

		mMetrics = pango_context_get_metrics(context, fontDesc, nullptr);
	}

	return mMetrics;
}

float MGtkDeviceImpl::GetAscent()
{
	float result = 10;

	PangoFontMetrics *metrics = GetMetrics();
	if (metrics != nullptr)
		result = float(pango_font_metrics_get_ascent(metrics)) / PANGO_SCALE;

	return result;
}

float MGtkDeviceImpl::GetDescent()
{
	float result = 10;

	PangoFontMetrics *metrics = GetMetrics();
	if (metrics != nullptr)
		result = float(pango_font_metrics_get_descent(metrics)) / PANGO_SCALE;

	return result;
}

float MGtkDeviceImpl::GetLeading()
{
	return 0;
}

float MGtkDeviceImpl::GetXWidth()
{
	return GetStringWidth("xxxxxxxxxx") / 10.f;
}

void MGtkDeviceImpl::DrawString(
	const string &inText,
	float inX,
	float inY,
	uint32_t inTruncateWidth,
	MAlignment inAlign)
{
}

uint32_t MGtkDeviceImpl::GetStringWidth(
	const string &inText)
{
	// reset attributes first
	PangoAttrList *attrs = pango_attr_list_new();
	pango_layout_set_attributes(mPangoLayout, attrs);
	pango_attr_list_unref(attrs);

	pango_layout_set_text(mPangoLayout, inText.c_str(), inText.length());

	PangoRectangle r;
	pango_layout_get_pixel_extents(mPangoLayout, nullptr, &r);

	return r.width;
}

void MGtkDeviceImpl::SetText(
	const string &inText)
{
	// reset attributes first
	PangoAttrList *attrs = pango_attr_list_new();
	pango_layout_set_attributes(mPangoLayout, attrs);
	pango_attr_list_unref(attrs);

	pango_layout_set_text(mPangoLayout, inText.c_str(), inText.length());
	mTextEndsWithNewLine = inText.length() > 0 and inText[inText.length() - 1] == '\n';
}

void MGtkDeviceImpl::SetTabStops(
	float inTabWidth)
{
	PangoTabArray *tabs = pango_tab_array_new(2, false);

	uint32_t next = inTabWidth;

	for (uint32_t x = 0; x < 2; ++x)
	{
		pango_tab_array_set_tab(tabs, x, PANGO_TAB_LEFT, next * mPangoScale);
		next += inTabWidth;
	}

	pango_layout_set_tabs(mPangoLayout, tabs);

	pango_tab_array_free(tabs);
}

void MGtkDeviceImpl::SetTextColors(
	uint32_t inColorCount,
	uint32_t inColorIndices[],
	uint32_t inOffsets[],
	MColor inColors[])
{
	PangoAttrList *attrs = pango_layout_get_attributes(mPangoLayout);

	for (uint32_t ix = 0; ix < inColorCount; ++ix)
	{
		MColor c = inColors[inColorIndices[ix]];

		uint16_t red = c.red << 8 | c.red;
		uint16_t green = c.green << 8 | c.green;
		uint16_t blue = c.blue << 8 | c.blue;

		PangoAttribute *attr = pango_attr_foreground_new(red, green, blue);
		attr->start_index = inOffsets[ix];

		if (ix == inColorCount - 1)
			attr->end_index = -1;
		else
			attr->end_index = inOffsets[ix + 1];

		assert(attr->end_index > attr->start_index);

		pango_attr_list_change(attrs, attr);
	}
}

void MGtkDeviceImpl::RenderTextBackground(float inX, float inY, uint32_t inStart, uint32_t inLength, MColor inColor)
{
	PangoAttrList *attrs = pango_layout_get_attributes(mPangoLayout);

	uint16_t red = inColor.red << 8 | inColor.red;
	uint16_t green = inColor.green << 8 | inColor.green;
	uint16_t blue = inColor.blue << 8 | inColor.blue;

	PangoAttribute *attr = pango_attr_background_new(red, green, blue);
	attr->start_index = inStart;
	attr->end_index = inStart + inLength;

	pango_attr_list_change(attrs, attr);
}

void MGtkDeviceImpl::SetTextStyles(
	uint32_t inStyleCount,
	uint32_t inStyles[],
	uint32_t inOffsets[])
{
	PangoAttrList *attrs = pango_layout_get_attributes(mPangoLayout);

	for (uint32_t ix = 0; ix < inStyleCount; ++ix)
	{
		uint32_t start_index = inOffsets[ix];
		uint32_t end_index = (ix + 1 == inStyleCount ? G_MAXUINT : inOffsets[ix + 1]);
		assert(end_index > start_index);

		PangoAttribute *attr;

		attr = pango_attr_weight_new(inStyles[ix] & MDevice::eTextStyleBold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change(attrs, attr);

		attr = pango_attr_style_new(inStyles[ix] & MDevice::eTextStyleItalic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change(attrs, attr);

		attr = pango_attr_underline_new(inStyles[ix] & MDevice::eTextStyleUnderline ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change(attrs, attr);
	}
}

void MGtkDeviceImpl::SetTextSelection(
	uint32_t inStart,
	uint32_t inLength,
	MColor inSelectionColor)
{
	uint16_t red = inSelectionColor.red << 8 | inSelectionColor.red;
	uint16_t green = inSelectionColor.green << 8 | inSelectionColor.green;
	uint16_t blue = inSelectionColor.blue << 8 | inSelectionColor.blue;

	PangoAttribute *attr = pango_attr_background_new(red, green, blue);
	attr->start_index = inStart;
	attr->end_index = inStart + inLength;

	PangoAttrList *attrs = pango_layout_get_attributes(mPangoLayout);
	pango_attr_list_change(attrs, attr);
}

void MGtkDeviceImpl::IndexToPosition(
	uint32_t inIndex,
	bool inTrailing,
	int32_t &outPosition)
{
	PangoRectangle r;
	pango_layout_index_to_pos(mPangoLayout, inIndex, &r);
	outPosition = r.x / mPangoScale;
}

bool MGtkDeviceImpl::PositionToIndex(
	int32_t inPosition,
	uint32_t &outIndex)
{
	int index, trailing;

	bool result = pango_layout_xy_to_index(mPangoLayout,
	                                       inPosition * mPangoScale, 0, &index, &trailing);

	MEncodingTraits<kEncodingUTF8> enc;
	const char *text = pango_layout_get_text(mPangoLayout);

	while (trailing-- > 0)
	{
		uint32_t n = enc.GetNextCharLength(text);
		text += n;
		index += n;
	}

	outIndex = index;

	return result;
}

float MGtkDeviceImpl::GetTextWidth()
{
	PangoRectangle r;

	pango_layout_get_pixel_extents(mPangoLayout, nullptr, &r);

	return r.width;
}

void MGtkDeviceImpl::RenderText(
	float inX,
	float inY)
{
}

void MGtkDeviceImpl::DrawCaret(
	float inX,
	float inY,
	uint32_t inOffset)
{
}

void MGtkDeviceImpl::BreakLines(
	uint32_t inWidth,
	vector<uint32_t> &outBreaks)
{
	pango_layout_set_width(mPangoLayout, inWidth * mPangoScale);
	pango_layout_set_wrap(mPangoLayout, PANGO_WRAP_WORD_CHAR);

	if (pango_layout_is_wrapped(mPangoLayout))
	{
		uint32_t line = 0;
		for (;;)
		{
			PangoLayoutLine *pangoLine = pango_layout_get_line_readonly(mPangoLayout, line);
			++line;

			if (pangoLine == nullptr)
				break;

			outBreaks.push_back(pangoLine->start_index + pangoLine->length);
		}
	}
}

PangoItem *MGtkDeviceImpl::Itemize(
	const char *inText,
	PangoAttrList *inAttrs)
{
	PangoContext *context = pango_layout_get_context(mPangoLayout);
	GList *items = pango_itemize(context, inText, 0, strlen(inText), inAttrs, nullptr);
	PangoItem *item = static_cast<PangoItem *>(items->data);
	g_list_free(items);
	return item;
}

void MGtkDeviceImpl::GetWhiteSpaceGlyphs(
	uint32_t &outSpace,
	uint32_t &outTab,
	uint32_t &outNL)
{
	//	long kMiddleDot = 0x00B7, kRightChevron = 0x00BB, kNotSign = 0x00AC;

	const char
		not_sign[] = "\xc2\xac",      // 0x00AC
		right_chevron[] = "\xc2\xbb", // 0x00BB
		middle_dot[] = "\xc2\xb7";    // 0x00B7

	PangoAttrList *attrs = pango_attr_list_new();

	PangoAttribute *attr = pango_attr_font_desc_new(mFont);
	attr->start_index = 0;
	attr->end_index = 2;

	pango_attr_list_insert(attrs, attr);

	PangoGlyphString *ts = pango_glyph_string_new();

	PangoItem *item = Itemize(middle_dot, attrs);
	assert(item->analysis.font);
	pango_shape(middle_dot, strlen(middle_dot), &item->analysis, ts);
	outSpace = ts->glyphs[0].glyph;

	item = Itemize(right_chevron, attrs);
	assert(item->analysis.font);
	pango_shape(right_chevron, strlen(right_chevron), &item->analysis, ts);
	outTab = ts->glyphs[0].glyph;

	item = Itemize(not_sign, attrs);
	assert(item->analysis.font);
	pango_shape(not_sign, strlen(not_sign), &item->analysis, ts);
	outNL = ts->glyphs[0].glyph;

	pango_glyph_string_free(ts);
	pango_attr_list_unref(attrs);
}

// --------------------------------------------------------------------
// MCairoDeviceImp is derived from MGtkDeviceImpl
// It provides the routines for drawing on a cairo surface

class MCairoDeviceImp : public MGtkDeviceImpl
{
  public:
	MCairoDeviceImp(MView *inView);
	// MCairoDeviceImp(GtkPrintContext *inContext, MRect inRect, int32_t inPage);
	~MCairoDeviceImp();

	virtual void Save();
	virtual void Restore();
	virtual bool IsPrinting(int32_t &outPage) const
	{
		outPage = mPage;
		return outPage >= 0;
	}

	virtual MRect GetBounds() const { return mRect; }
	virtual void SetOrigin(int32_t inX, int32_t inY);
	virtual void SetForeColor(MColor inColor);
	virtual MColor GetForeColor() const;
	virtual void SetBackColor(MColor inColor);
	virtual MColor GetBackColor() const;
	virtual void ClipRect(MRect inRect);
	virtual void EraseRect(MRect inRect);
	virtual void FillRect(MRect inRect);
	virtual void StrokeRect(MRect inRect, uint32_t inLineWidth = 1);
	virtual void FillEllipse(MRect inRect);
	virtual void DrawImage(cairo_surface_t *inImage, float inX, float inY, float inShear);
	virtual void DrawBitmap(const MBitmap &inBitmap, float inX, float inY);
	virtual void CreateAndUsePattern(MColor inColor1, MColor inColor2);
	virtual void DrawString(const string &inText, float inX, float inY, uint32_t inTruncateWidth, MAlignment inAlign);
	virtual void RenderText(float inX, float inY);
	virtual void DrawCaret(float inX, float inY, uint32_t inOffset);
	virtual void MakeTransparent(float inOpacity);
	virtual void SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor);

  protected:
	void DrawWhiteSpace(float inX, float inY);

	MRect mRect;
	MColor mForeColor;
	MColor mBackColor;
	MColor mEvenRowColor;
	MColor mWhiteSpaceColor;
	cairo_t *mContext;
	uint32_t mPatternData[8][8];
	int32_t mPage;
	bool mDrawWhiteSpace;
};

MCairoDeviceImp::MCairoDeviceImp(MView *inView)
	: mContext(nullptr)
	, mPage(-1)
	, mDrawWhiteSpace(false)
{
	mForeColor = kBlack;
	mBackColor = kWhite;

	MCanvas* canvas = dynamic_cast<MCanvas*>(inView);
	MGtkCanvasImpl* target = static_cast<MGtkCanvasImpl*>(canvas->GetImpl());
	mContext = target->GetCairo();

	// cairo_rectangle(mContext, mRect.x, mRect.y, mRect.width, mRect.height);
	// cairo_clip(mContext);
}

MCairoDeviceImp::~MCairoDeviceImp()
{
}

void MCairoDeviceImp::Save()
{
	cairo_save(mContext);
}

void MCairoDeviceImp::Restore()
{
	cairo_restore(mContext);
}

void MCairoDeviceImp::SetOrigin(
	int32_t inX,
	int32_t inY)
{
	cairo_translate(mContext, inX, inY);
}

void MCairoDeviceImp::SetForeColor(
	MColor inColor)
{
	mForeColor = inColor;

	cairo_set_source_rgb(mContext,
	                     mForeColor.red / 255.0,
	                     mForeColor.green / 255.0,
	                     mForeColor.blue / 255.0);
}

MColor MCairoDeviceImp::GetForeColor() const
{
	return mForeColor;
}

void MCairoDeviceImp::SetBackColor(
	MColor inColor)
{
	mBackColor = inColor;
}

MColor MCairoDeviceImp::GetBackColor() const
{
	return mBackColor;
}

void MCairoDeviceImp::ClipRect(
	MRect inRect)
{
	cairo_rectangle(mContext, inRect.x, inRect.y, inRect.width, inRect.height);
	cairo_clip(mContext);
}

//void MCairoDeviceImp::ClipRegion(
//	MRegion				inRegion)
//{
//	GdkRegion* gdkRegion = const_cast<GdkRegion*>(inRegion.operator const GdkRegion*());
//	gdk_cairo_region(mContext, gdkRegion);
//	cairo_clip(mContext);
//}

void MCairoDeviceImp::EraseRect(
	MRect inRect)
{
	cairo_save(mContext);

	cairo_rectangle(mContext, inRect.x, inRect.y, inRect.width, inRect.height);

	cairo_set_source_rgb(mContext,
	                     mBackColor.red / 255.0,
	                     mBackColor.green / 255.0,
	                     mBackColor.blue / 255.0);

	//	if (mOffscreenPixmap != nullptr)
	//		cairo_set_operator(mContext, CAIRO_OPERATOR_CLEAR);
	//
	cairo_fill(mContext);

	cairo_restore(mContext);
}

void MCairoDeviceImp::FillRect(
	MRect inRect)
{
	cairo_rectangle(mContext, inRect.x, inRect.y, inRect.width, inRect.height);
	cairo_fill(mContext);
}

void MCairoDeviceImp::StrokeRect(
	MRect inRect,
	uint32_t inLineWidth)
{
	cairo_rectangle(mContext, inRect.x, inRect.y, inRect.width, inRect.height);
	cairo_stroke(mContext);
}

void MCairoDeviceImp::FillEllipse(
	MRect inRect)
{
	cairo_save(mContext);
	cairo_translate(mContext, inRect.x + inRect.width / 2., inRect.y + inRect.height / 2.);
	cairo_scale(mContext, inRect.width / 2., inRect.height / 2.);
	cairo_arc(mContext, 0., 0., 1., 0., 2 * M_PI);
	cairo_fill(mContext);
	cairo_restore(mContext);
}

void MCairoDeviceImp::DrawImage(
	cairo_surface_t *inImage,
	float inX,
	float inY,
	float inShear)
{
	cairo_save(mContext);

	cairo_surface_set_device_offset(inImage, -inX, -inY);

	cairo_pattern_t *p = cairo_pattern_create_for_surface(inImage);

	if (p != nullptr)
	{
		cairo_matrix_t m;
		cairo_matrix_init_translate(&m, -inX, -inY);
		//		cairo_matrix_init_rotate(&m, 2.356);
		cairo_matrix_init(&m, 1, inShear, inShear, 1, 0, 0);
		cairo_pattern_set_matrix(p, &m);

		cairo_set_source(mContext, p);

		cairo_pattern_destroy(p);

		int w = cairo_image_surface_get_width(inImage);
		int h = cairo_image_surface_get_height(inImage);

		cairo_rectangle(mContext, inX, inY, w, h);
		cairo_fill(mContext);
	}

	cairo_restore(mContext);
}

void MCairoDeviceImp::DrawBitmap(const MBitmap &inBitmap, float inX, float inY)
{
	cairo_surface_t *surface = cairo_image_surface_create_for_data(
		(uint8_t *)inBitmap.Data(), CAIRO_FORMAT_ARGB32, inBitmap.Width(), inBitmap.Height(), inBitmap.Stride());

	if (surface != nullptr)
	{
		DrawImage(surface, inX, inY, 0);
		cairo_surface_destroy(surface);
	}
}

void MCairoDeviceImp::CreateAndUsePattern(
	MColor inColor1,
	MColor inColor2)
{
	uint32_t c1 = 0, c2 = 0;

	assert(cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, 8) == 32);

	c1 |= inColor1.red << 16;
	c1 |= inColor1.green << 8;
	c1 |= inColor1.blue << 0;

	c2 |= inColor2.red << 16;
	c2 |= inColor2.green << 8;
	c2 |= inColor2.blue << 0;

	for (uint32_t y = 0; y < 8; ++y)
	{
		for (uint32_t x = 0; x < 4; ++x)
			mPatternData[y][x] = c1;
		for (uint32_t x = 4; x < 8; ++x)
			mPatternData[y][x] = c2;
	}

	cairo_surface_t *s = cairo_image_surface_create_for_data(
		reinterpret_cast<uint8_t *>(mPatternData), CAIRO_FORMAT_RGB24, 8, 8, 32);

	if (s != nullptr)
	{
		cairo_pattern_t *p = cairo_pattern_create_for_surface(s);
		cairo_pattern_set_extend(p, CAIRO_EXTEND_REPEAT);

		if (p != nullptr)
		{
			cairo_matrix_t m;
			cairo_matrix_init_rotate(&m, 2.356);
			cairo_pattern_set_matrix(p, &m);

			cairo_set_source(mContext, p);

			cairo_pattern_destroy(p);
		}

		cairo_surface_destroy(s);
	}
}

void MCairoDeviceImp::DrawString(
	const string &inText,
	float inX,
	float inY,
	uint32_t inTruncateWidth,
	MAlignment inAlign)
{
	// reset attributes first
	PangoAttrList *attrs = pango_attr_list_new();
	pango_layout_set_attributes(mPangoLayout, attrs);
	pango_attr_list_unref(attrs);

	pango_layout_set_text(mPangoLayout, inText.c_str(), inText.length());

	if (inTruncateWidth != 0)
	{
		pango_layout_set_ellipsize(mPangoLayout, PANGO_ELLIPSIZE_END);
		pango_layout_set_width(mPangoLayout, inTruncateWidth * mPangoScale);

		if (inAlign != eAlignNone and inAlign != eAlignLeft)
		{
			PangoRectangle r;
			pango_layout_get_pixel_extents(mPangoLayout, nullptr, &r);

			if (static_cast<uint32_t>(r.width) < inTruncateWidth)
			{
				if (inAlign == eAlignCenter)
					inX += (inTruncateWidth - r.width) / 2;
				else
					inX += inTruncateWidth - r.width;
			}
		}
	}
	else
	{
		pango_layout_set_ellipsize(mPangoLayout, PANGO_ELLIPSIZE_NONE);
		pango_layout_set_width(mPangoLayout, mRect.width * mPangoScale);
	}

	cairo_move_to(mContext, inX, inY);

	pango_cairo_show_layout(mContext, mPangoLayout);
}

void MCairoDeviceImp::DrawWhiteSpace(
	float inX,
	float inY)
{
#if PANGO_VERSION_CHECK(1, 22, 0)
	int baseLine = pango_layout_get_baseline(mPangoLayout);
	PangoLayoutLine *line = pango_layout_get_line(mPangoLayout, 0);

	cairo_set_source_rgb(mContext,
	                     mWhiteSpaceColor.red / 255.0,
	                     mWhiteSpaceColor.green / 255.0,
	                     mWhiteSpaceColor.blue / 255.0);

	// we're using one font anyway
	PangoFontMap *fontMap = pango_cairo_font_map_get_default();
	PangoContext *context = pango_layout_get_context(mPangoLayout);
	PangoFont *font = pango_font_map_load_font(fontMap, context, mFont);
	cairo_scaled_font_t *scaledFont =
		pango_cairo_font_get_scaled_font(reinterpret_cast<PangoCairoFont *>(font));

	if (scaledFont == nullptr or cairo_scaled_font_status(scaledFont) != CAIRO_STATUS_SUCCESS)
		return;

	cairo_set_scaled_font(mContext, scaledFont);

	int x_position = 0;
	vector<cairo_glyph_t> cairo_glyphs;

	for (GSList *run = line->runs; run != nullptr; run = run->next)
	{
		PangoGlyphItem *glyphItem = reinterpret_cast<PangoGlyphItem *>(run->data);

		PangoGlyphItemIter iter;
		const char *text = pango_layout_get_text(mPangoLayout);

		for (bool more = pango_glyph_item_iter_init_start(&iter, glyphItem, text);
		     more;
		     more = pango_glyph_item_iter_next_cluster(&iter))
		{
			PangoGlyphString *gs = iter.glyph_item->glyphs;
			char ch = text[iter.start_index];

			for (int i = iter.start_glyph; i < iter.end_glyph; ++i)
			{
				PangoGlyphInfo *gi = &gs->glyphs[i];

				if (ch == ' ' or ch == '\t')
				{
					double cx = inX + double(x_position + gi->geometry.x_offset) / mPangoScale;
					double cy = inY + double(baseLine + gi->geometry.y_offset) / mPangoScale;

					cairo_glyph_t g;
					if (ch == ' ')
						g.index = mSpaceGlyph;
					else
						g.index = mTabGlyph;
					g.x = cx;
					g.y = cy;

					cairo_glyphs.push_back(g);
				}

				x_position += gi->geometry.width;
			}
		}
	}

	// and a trailing newline perhaps?

	if (mTextEndsWithNewLine)
	{
		double cx = inX + double(x_position) / mPangoScale;
		double cy = inY + double(baseLine) / mPangoScale;

		cairo_glyph_t g;
		g.index = mNewLineGlyph;
		g.x = cx;
		g.y = cy;

		cairo_glyphs.push_back(g);
	}

	cairo_show_glyphs(mContext, &cairo_glyphs[0], cairo_glyphs.size());
#endif
}

void MCairoDeviceImp::RenderText(
	float inX,
	float inY)
{
	if (mDrawWhiteSpace)
	{
		Save();
		DrawWhiteSpace(inX, inY);
		Restore();
	}

	cairo_move_to(mContext, inX, inY);
	pango_cairo_show_layout(mContext, mPangoLayout);
}

void MCairoDeviceImp::DrawCaret(
	float inX,
	float inY,
	uint32_t inOffset)
{
	PangoRectangle sp, wp;

	pango_layout_get_cursor_pos(mPangoLayout, inOffset, &sp, &wp);

	Save();

	cairo_set_line_width(mContext, 1.0);
	cairo_set_source_rgb(mContext, 0, 0, 0);
	cairo_move_to(mContext, inX + sp.x / mPangoScale, inY + sp.y / mPangoScale);
	cairo_rel_line_to(mContext, sp.width / mPangoScale, sp.height / mPangoScale);
	cairo_stroke(mContext);

	Restore();
}

void MCairoDeviceImp::MakeTransparent(
	float inOpacity)
{
	cairo_set_operator(mContext, CAIRO_OPERATOR_DEST_OUT);
	cairo_set_source_rgba(mContext, 1, 0, 0, inOpacity);
	cairo_paint(mContext);
}

//GdkPixmap* MCairoDeviceImp::GetPixmap() const
//{
//	g_object_ref(mOffscreenPixmap);
//	return mOffscreenPixmap;
//}
//
void MCairoDeviceImp::SetDrawWhiteSpace(
	bool inDrawWhiteSpace,
	MColor inWhiteSpaceColor)
{
	mDrawWhiteSpace = inDrawWhiteSpace;
	mWhiteSpaceColor = inWhiteSpaceColor;
}

MDeviceImpl *MDeviceImpl::Create()
{
	return new MGtkDeviceImpl();
}

// MDeviceImpl* MDeviceImpl::Create(MView* inView, MRect inRect, bool inCreateOffscreen)
// {
// 	return new MCairoDeviceImp(inView, inRect, inCreateOffscreen);
// }

MDeviceImpl *MDeviceImpl::Create(MView *inView)
{
	return new MCairoDeviceImp(inView);
}

struct MPNGSurface
{
	MPNGSurface(const void *inPNG, uint32_t inLength)
		: mSurface(nullptr)
		, mData(reinterpret_cast<const uint8_t *>(inPNG))
		, mLength(inLength)
	{
		mSurface = cairo_image_surface_create_from_png_stream(&MPNGSurface::read_func, this);
		if (mSurface == nullptr)
			throw runtime_error("cannot decode PNG data");
	}

	~MPNGSurface()
	{
		cairo_surface_destroy(mSurface);
	}

	static cairo_status_t read_func(void *closure, unsigned char *data, unsigned int length)
	{
		MPNGSurface *self = reinterpret_cast<MPNGSurface *>(closure);
		return self->Read(data, length);
	}

	cairo_status_t Read(unsigned char *data, unsigned int length)
	{
		cairo_status_t result = CAIRO_STATUS_READ_ERROR;
		if (length <= mLength)
		{
			memcpy(data, mData, length);
			mLength -= length;
			mData += length;
			result = CAIRO_STATUS_SUCCESS;
		}
		return result;
	}

	operator cairo_surface_t *() { return mSurface; }

	cairo_surface_t *mSurface;
	const uint8_t *mData;
	uint32_t mLength;
};

MBitmap::MBitmap(const void *inPNG, uint32_t inLength)
	: mData(nullptr)
	, mWidth(0)
	, mHeight(0)
	, mStride(0)
	, mUseAlpha(true)
{
	MPNGSurface surface(inPNG, inLength);

	mWidth = cairo_image_surface_get_width(surface);
	mHeight = cairo_image_surface_get_height(surface);
	mStride = cairo_image_surface_get_stride(surface);

	uint32_t length = mHeight * mStride;
	switch (cairo_image_surface_get_format(surface))
	{
		case CAIRO_FORMAT_RGB24: mUseAlpha = false; // fall through
		case CAIRO_FORMAT_ARGB32: length *= 4; break;
		case CAIRO_FORMAT_A8: mUseAlpha = false; break;
		default: throw runtime_error("unsupported format"); break;
	}

	mData = new uint32_t[length / 4];
	memcpy(mData, cairo_image_surface_get_data(surface), length);
}

MGeometryImpl *MGeometryImpl::Create(MDevice &inDevice, MGeometryFillMode inMode)
{
	assert(false);
	return nullptr;
}

void MDevice::ListFonts(
	bool inFixedWidthOnly,
	vector<string> &outFonts)
{
	PangoFontMap *fontMap = pango_cairo_font_map_get_default();
	// PRINT(("DPI volgens pango/cairo: %g", pango_cairo_font_map_get_resolution((PangoCairoFontMap *)fontMap)));

	PangoFontFamily **families;
	int n_families;

	pango_font_map_list_families(fontMap, &families, &n_families);

	for (int i = 0; i < n_families; i++)
	{
		PangoFontFamily *family = families[i];

		if (not pango_font_family_is_monospace(family))
			continue;

		const char *family_name;

		family_name = pango_font_family_get_name(family);

		outFonts.push_back(family_name);
	}
	g_free(families);
}

const MColor kDialogBackgroundColor;

void MDevice::GetSysSelectionColor(MColor &outColor)
{
	outColor = MColor("#FFD281");
}
