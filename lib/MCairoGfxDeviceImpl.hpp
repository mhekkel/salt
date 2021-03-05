/* 
   Created by: Maarten L. Hekkelman
   Date: woensdag 09 januari, 2019

	Graphics device implementation for cairo
*/

#pragma once

#include "MGfxDevice.hpp"

// --------------------------------------------------------------------
// default cairo device impl

class MCairoGfxDeviceImpl : public MGfxDeviceImpl
{
  public:
	MCairoGfxDeviceImpl(cairo_t* inCairo);
	~MCairoGfxDeviceImpl();
	
	virtual void ClipRect(float x, float y, float width, float height);
	virtual void SetColorRGB(float r, float g, float b);	
	virtual void SetLineWidth(float lineWidth);	
	virtual void MoveTo(float x, float y);
	virtual void LineTo(float x, float y);
	virtual void Stroke();
	virtual void FillRect(float x, float y, float width, float height);
	virtual void StrokeRect(float x, float y, float width, float height);	
	virtual void FillRoundedRect(float x, float y, float width, float height, float radius);
	virtual void StrokeRoundedRect(float x, float y, float width, float height, float radius);
	virtual void FillPoly(std::initializer_list<std::tuple<float,float>> pts);
	virtual void StrokePoly(std::initializer_list<std::tuple<float,float>> pts);
	virtual void SetFont(const char* inFontName, float inSize, bool inBold, bool inItalic);
	virtual FontExtents GetFontExtents();
	virtual TextExtents GetTextExtents(const char* inText);
	virtual void ShowText(const char* inText);

  protected:

	MCairoGfxDeviceImpl() {}
	
	void CreateRoundedRectPath(float x, float y, float width, float height, float radius);

	cairo_t* mCairo = nullptr;
};

// --------------------------------------------------------------------
// cairo device impl that renders to an image (offscreen buffer)

class MCairoImageGfxDeviceImpl : public MCairoGfxDeviceImpl
{
  public:
	MCairoImageGfxDeviceImpl(int width, int height);
	~MCairoImageGfxDeviceImpl();

  private:
	cairo_surface_t* mSurface;
};
