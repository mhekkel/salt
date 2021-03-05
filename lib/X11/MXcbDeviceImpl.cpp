//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MXcbLib.hpp"

#include <cmath>
#include <cstring>
#include <cassert>

#include "MXcbDeviceImpl.hpp"
#include "MXcbCanvasImpl.hpp"
#include "MView.hpp"
#include "MWindow.hpp"
#include "MError.hpp"
#include "MUnicode.hpp"

using namespace std;



MDeviceImpl* MDeviceImpl::Create()
{
//	return new MXcbDeviceImpl();
	return nullptr;
}

MDeviceImpl* MDeviceImpl::Create(MView* inView, MRect inRect, bool inCreateOffscreen)
{
//	return new MCairoDeviceImp(inView, inRect, inCreateOffscreen);
	return nullptr;
}

//struct MPNGSurface
//{
//	MPNGSurface(const void* inPNG, uint32 inLength)
//		: mSurface(nullptr), mData(reinterpret_cast<const uint8*>(inPNG)), mLength(inLength)
//	{
//		mSurface = cairo_image_surface_create_from_png_stream(&MPNGSurface::read_func, this);
//		if (mSurface == nullptr)
//			throw runtime_error("cannot decode PNG data");
//	}
//	
//	~MPNGSurface()
//	{
//		cairo_surface_destroy(mSurface);
//	}
//	
//	static cairo_status_t read_func(void* closure, unsigned char* data, unsigned int length)
//	{
//		MPNGSurface* self = reinterpret_cast<MPNGSurface*>(closure);
//		return self->Read(data, length);
//	}
//
//	cairo_status_t Read(unsigned char* data, unsigned int length)
//	{
//		cairo_status_t result = CAIRO_STATUS_READ_ERROR;
//		if (length <= mLength)
//		{
//			memcpy(data, mData, length);
//			mLength -= length;
//			mData += length;
//			result = CAIRO_STATUS_SUCCESS;
//		}
//		return result;
//	}
//	
//	operator cairo_surface_t*() { return mSurface; }
//
//	cairo_surface_t* mSurface;
//	const uint8* mData;
//	uint32 mLength;
//};
//
//MBitmap::MBitmap(const void* inPNG, uint32 inLength)
//	: mData(nullptr), mWidth(0), mHeight(0), mStride(0), mUseAlpha(true)
//{
//	MPNGSurface surface(inPNG, inLength);
//
//	mWidth = cairo_image_surface_get_width(surface);
//	mHeight = cairo_image_surface_get_height(surface);
//	mStride = cairo_image_surface_get_stride(surface);
//
//	uint32 length = mHeight * mStride;
//	switch (cairo_image_surface_get_format(surface))
//	{
//		case CAIRO_FORMAT_RGB24:	mUseAlpha = false;	// fall through
//		case CAIRO_FORMAT_ARGB32:	length *= 4; break;
//		case CAIRO_FORMAT_A8:		mUseAlpha = false; break;
//		default:					throw runtime_error("unsupported format"); break;
//	}
//	
//	mData = new uint32[length / 4];
//	memcpy(mData, cairo_image_surface_get_data(surface), length);
//}

MGeometryImpl* MGeometryImpl::Create(MDevice& inDevice, MGeometryFillMode inMode)
{
	assert(false);
	return nullptr;
}

void MDevice::ListFonts(
	bool			inFixedWidthOnly,
	vector<string>&	outFonts)
{
	PangoFontMap* fontMap = pango_cairo_font_map_get_default();

    PangoFontFamily** families;
    int n_families;
    
    pango_font_map_list_families(fontMap, &families, &n_families);
    
    for (int i = 0; i < n_families; i++)
    {
        PangoFontFamily* family = families[i];

    	if (not pango_font_family_is_monospace(family))	
    		continue;
    	
        const char* family_name;

        family_name = pango_font_family_get_name(family);
        
        outFonts.push_back(family_name);
    }
    g_free(families);
}

const MColor kDialogBackgroundColor;

void MDevice::GetSysSelectionColor(MColor& outColor)
{
	outColor = MColor("#FFD281");
}
