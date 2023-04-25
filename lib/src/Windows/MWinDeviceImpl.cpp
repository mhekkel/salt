//			Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//	  (See accompanying file LICENSE_1_0.txt or copy at
//			http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "windowscodecs")

#include <wincodec.h>

#include <cmath>
#include <cstring>
#include <stack>

#include "MError.hpp"
#include "MPreferences.hpp"
#include "MUnicode.hpp"
#include "MView.hpp"
#include "MWinCanvasImpl.hpp"
#include "MWinControlsImpl.hpp"
#include "MWinDeviceImpl.hpp"
#include "MWinUtils.hpp"
#include "MWinWindowImpl.hpp"

using namespace std;

// Add our own conversion routine to namespace D2D1
namespace D2D1
{

D2D1FORCEINLINE D2D1_RECT_F RectF(const MRect &r)
{
	return Rect<FLOAT>(
		static_cast<float>(r.x),
		static_cast<float>(r.y),
		static_cast<float>(r.x + r.width),
		static_cast<float>(r.y + r.height));
}

} // namespace D2D1

namespace
{
// --------------------------------------------------------------------
// MInlineReplacedChar is a substitute for unknown characters

class MInlineReplacedChar : public IDWriteInlineObject
{
  public:
	MInlineReplacedChar(ID2D1RenderTarget *pRenderTarget, uint32_t inUnicode, MColor inColor,
	                    float inWidth, float inHeight, float inBaseline, IDWriteTextFormat *inFormat);
	~MInlineReplacedChar();

	STDMETHOD(Draw)
	(
		__maybenull void *clientDrawingContext,
		IDWriteTextRenderer *renderer,
		FLOAT originX,
		FLOAT originY,
		BOOL isSideways,
		BOOL isRightToLeft,
		IUnknown *clientDrawingEffect);

	STDMETHOD(GetMetrics)
	(
		__out DWRITE_INLINE_OBJECT_METRICS *metrics);

	STDMETHOD(GetOverhangMetrics)
	(
		__out DWRITE_OVERHANG_METRICS *overhangs);

	STDMETHOD(GetBreakConditions)
	(
		__out DWRITE_BREAK_CONDITION *breakConditionBefore,
		__out DWRITE_BREAK_CONDITION *breakConditionAfter);

  public:
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(
		IID const &riid,
		void **ppvObject);

  private:
	ID2D1RenderTargetPtr mRenderTarget;
	IDWriteTextFormatPtr mTextFormat;
	uint32_t mUnicode;
	MColor mColor;
	float mWidth, mHeight;
	float mBaseline;
	unsigned long mRefCount;
};

MInlineReplacedChar::MInlineReplacedChar(ID2D1RenderTarget *inRenderTarget, uint32_t inUnicode, MColor inColor,
                                         float inWidth, float inHeight, float inBaseline, IDWriteTextFormat *inTextFormat)
	: mRenderTarget(inRenderTarget)
	, mTextFormat(inTextFormat)
	, mUnicode(inUnicode)
	, mColor(inColor)
	, mWidth(inWidth)
	, mHeight(inHeight)
	, mBaseline(inBaseline)
	, mRefCount(1)
{
}

MInlineReplacedChar::~MInlineReplacedChar()
{
}

HRESULT STDMETHODCALLTYPE MInlineReplacedChar::Draw(
	__maybenull void *clientDrawingContext,
	IDWriteTextRenderer *renderer,
	FLOAT originX,
	FLOAT originY,
	BOOL isSideways,
	BOOL isRightToLeft,
	IUnknown *clientDrawingEffect)
{
	wchar_t s[2];
	uint32_t l = 1;

	if (mUnicode <= 0x0FFFF)
		s[0] = static_cast<wchar_t>(mUnicode);
	else
	{
		s[0] = (mUnicode - 0x010000) / 0x0400 + 0x0D800;
		s[1] = (mUnicode - 0x010000) % 0x0400 + 0x0DC00;
		l = 2;
	}

	MColor textColor = mColor;
	ComPtr<ID2D1SolidColorBrush> effect;
	if (clientDrawingEffect != nullptr)
		clientDrawingEffect->QueryInterface(&effect);

	if (effect)
	{
		D2D1_COLOR_F color = effect->GetColor();
		textColor = MColor(color.r, color.g, color.b);
	}

	ComPtr<ID2D1SolidColorBrush> brush;
	if (mRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(textColor.red / 255.f, textColor.green / 255.f, textColor.blue / 255.f),
			&brush) == S_OK)
	{
		mRenderTarget->DrawTextW(s, l, mTextFormat,
		                         D2D1::RectF(originX, originY, originX + mWidth, originY + mHeight), brush);
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE MInlineReplacedChar::GetMetrics(
	__out DWRITE_INLINE_OBJECT_METRICS *metrics)
{
	DWRITE_INLINE_OBJECT_METRICS inlineMetrics = {};
	inlineMetrics.width = mWidth;
	inlineMetrics.height = mHeight;
	inlineMetrics.baseline = mBaseline;
	*metrics = inlineMetrics;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE MInlineReplacedChar::GetOverhangMetrics(
	__out DWRITE_OVERHANG_METRICS *overhangs)
{
	overhangs->left = 0;
	overhangs->top = 0;
	overhangs->right = 0;
	overhangs->bottom = 0;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE MInlineReplacedChar::GetBreakConditions(
	__out DWRITE_BREAK_CONDITION *breakConditionBefore,
	__out DWRITE_BREAK_CONDITION *breakConditionAfter)
{
	*breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
	*breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
	return S_OK;
}

STDMETHODIMP MInlineReplacedChar::QueryInterface(IID const &riid, void **ppvObject)
{
	if (__uuidof(IDWriteInlineObject) == riid)
	{
		AddRef();
		*ppvObject = dynamic_cast<IDWriteInlineObject *>(this);
	}
	else if (__uuidof(IUnknown) == riid)
	{
		AddRef();
		*ppvObject = dynamic_cast<IUnknown *>(this);
	}
	else
	{
		*ppvObject = NULL;
		return E_FAIL;
	}

	return S_OK;
}

STDMETHODIMP_(unsigned long)
MInlineReplacedChar::AddRef()
{
	return InterlockedIncrement(&mRefCount);
}

STDMETHODIMP_(unsigned long)
MInlineReplacedChar::Release()
{
	unsigned long newCount = InterlockedDecrement(&mRefCount);

	if (newCount == 0)
	{
		delete this;
		return 0;
	}

	return newCount;
}

} // namespace

// --------------------------------------------------------------------
//

ID2D1Factory *MWinDeviceImpl::GetD2D1Factory()
{
	static ID2D1FactoryPtr sD2DFactory;

	if (sD2DFactory == nullptr)
	{
#if DEBUG
		D2D1_FACTORY_OPTIONS options = {};
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		THROW_IF_HRESULT_ERROR(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &sD2DFactory));
#else
		THROW_IF_HRESULT_ERROR(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &sD2DFactory));
#endif
	}

	return sD2DFactory;
}

IDWriteFactory *MWinDeviceImpl::GetDWFactory()
{
	static IDWriteFactory *sDWFactory = nullptr;

	if (sDWFactory == nullptr)
	{
		THROW_IF_HRESULT_ERROR(::DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown **>(&sDWFactory)));
	}

	return sDWFactory;
}

wstring MWinDeviceImpl::GetLocale()
{
	static wstring sLocale;

	if (sLocale.empty())
	{
		wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
		if (::GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH))
			sLocale = localeName;
		else
			sLocale = L"en-us";
	}

	return sLocale;
}

MWinDeviceImpl::MWinDeviceImpl()
	: mView(nullptr)
	, mClipLayer(nullptr)
	, mSelectionLength(0)
	, mReplaceUnknownCharacters(false)
	, mDrawWhiteSpace(false)
{
	HDC hdc = ::GetDC(NULL);
	if (hdc)
	{
		mDpiScaleX = 96.f / ::GetDeviceCaps(hdc, LOGPIXELSX);
		mDpiScaleY = 96.f / ::GetDeviceCaps(hdc, LOGPIXELSY);
		::ReleaseDC(NULL, hdc);
	}
	else
		mDpiScaleX = mDpiScaleY = 1.0f;
}

MWinDeviceImpl::MWinDeviceImpl(MView *inView, MRect inUpdate)
	: mView(inView)
	, mRenderTarget(nullptr)
	, mClipLayer(nullptr)
	, mTextFormat(nullptr)
	, mTextLayout(nullptr)
	, mForeBrush(nullptr)
	, mBackBrush(nullptr)
	, mFont(nullptr)
	, mReplaceUnknownCharacters(false)
	, mDrawWhiteSpace(false)
	, mSelectionLength(0)
	, mWhitespaceColor(kBlack)
{
	HDC hdc = ::GetDC(NULL);
	if (hdc)
	{
		mDpiScaleX = 96.f / ::GetDeviceCaps(hdc, LOGPIXELSX);
		mDpiScaleY = 96.f / ::GetDeviceCaps(hdc, LOGPIXELSY);
		::ReleaseDC(NULL, hdc);
	}
	else
		mDpiScaleX = mDpiScaleY = 1.0f;

	MRect bounds;
	inView->GetBounds(bounds);
	inView->ConvertToWindow(bounds.x, bounds.y);

	int32_t x = 0, y = 0;
	inView->ConvertToWindow(x, y);

	MCanvas *canvas = dynamic_cast<MCanvas *>(inView);
	mRenderTarget = static_cast<MWinCanvasImpl *>(canvas->GetImpl())->GetRenderTarget();

	mRenderTarget->Flush();

	D2D1::Matrix3x2F translate(
		D2D1::Matrix3x2F::Translation(
			static_cast<float>(x - bounds.x),
			static_cast<float>(y - bounds.y)));

	mRenderTarget->SetTransform(translate);

	// default to this mode:
	mRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	mClipping.push(inUpdate);

	if (inUpdate)
		ClipRect(inUpdate);

	SetForeColor(kBlack);
	SetBackColor(kWhite);
}

MWinDeviceImpl::~MWinDeviceImpl()
{
	try
	{
		while (not mState.empty())
			Restore();
	}
	catch (...)
	{
		// PRINT(("Oeps"));
	}
}

MWinDeviceImpl::State::State(
	MWinDeviceImpl &inImpl)
	: mImpl(inImpl)
	, mStateBlock(nullptr)
{
	THROW_IF_HRESULT_ERROR(GetD2D1Factory()->CreateDrawingStateBlock(&mStateBlock));
	mImpl.mRenderTarget->SaveDrawingState(mStateBlock);

	mForeBrush = mImpl.mForeBrush;
	mBackBrush = mImpl.mBackBrush;
}

MWinDeviceImpl::State::~State()
{
	mImpl.mRenderTarget->RestoreDrawingState(mStateBlock);
	mImpl.mForeBrush = mForeBrush;
	mImpl.mBackBrush = mBackBrush;
}

void MWinDeviceImpl::Save()
{
	mState.push(new State(*this));
}

void MWinDeviceImpl::Restore()
{
	assert(not mState.empty());
	if (not mState.empty())
	{
		delete mState.top();
		mState.pop();
	}
}

void MWinDeviceImpl::SetForeColor(
	MColor inColor)
{
	ComPtr<ID2D1SolidColorBrush> brush;
	THROW_IF_HRESULT_ERROR(
		mRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(inColor.red / 255.f, inColor.green / 255.f, inColor.blue / 255.f),
			&brush));
	mForeBrush = brush;
}

MColor MWinDeviceImpl::GetForeColor() const
{
	MColor result;

	ComPtr<ID2D1SolidColorBrush> brush;
	mForeBrush.QueryInterface(&brush);

	if (brush)
	{
		D2D1_COLOR_F color = brush->GetColor();
		result = MColor(color.r, color.g, color.b);
	}

	return result;
}

void MWinDeviceImpl::SetBackColor(
	MColor inColor)
{
	ComPtr<ID2D1SolidColorBrush> brush;
	THROW_IF_HRESULT_ERROR(
		mRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(inColor.red / 255.f, inColor.green / 255.f, inColor.blue / 255.f),
			&brush));
	mBackBrush = brush;
}

MColor MWinDeviceImpl::GetBackColor() const
{
	MColor result;

	ComPtr<ID2D1SolidColorBrush> brush;
	mBackBrush.QueryInterface(&brush);

	if (brush)
	{
		D2D1_COLOR_F color = brush->GetColor();
		result = MColor(color.r, color.g, color.b);
	}

	return result;
}

void MWinDeviceImpl::ClipRect(
	MRect inRect)
{
	//	if (mClipLayer != nullptr)
	//	{
	//		mRenderTarget->PopLayer();
	//		mClipLayer->Release();
	//	}
	mClipping.push(inRect);
	//	mRenderTarget->PushAxisAlignedClip(D2D1::RectF(inRect), D2D1_ANTIALIAS_MODE_ALIASED);
	//
	//	THROW_IF_HRESULT_ERROR(mRenderTarget->CreateLayer(&mClipLayer));
	//
	//	  // Push the layer with the geometric mask.
	//	mRenderTarget->PushLayer(
	//		D2D1::LayerParameters(D2D1::RectF(inRect)),
	//		mClipLayer);
}

//void MWinDeviceImpl::ClipRegion(
//	MRegion				inRegion)
//{
//}

void MWinDeviceImpl::EraseRect(
	MRect inRect)
{
	if (not mClipping.empty())
		inRect &= mClipping.top();

	assert(mBackBrush);
	assert(mRenderTarget);

	mRenderTarget->FillRectangle(D2D1::RectF(inRect), mBackBrush);
}

void MWinDeviceImpl::FillRect(
	MRect inRect)
{
	if (not mClipping.empty())
		inRect &= mClipping.top();

	assert(mForeBrush);
	assert(mRenderTarget);

	mRenderTarget->FillRectangle(D2D1::RectF(inRect), mForeBrush);
}

void MWinDeviceImpl::StrokeRect(
	MRect inRect,
	uint32_t inLineWidth)
{
	assert(mForeBrush);
	assert(mRenderTarget);

	mRenderTarget->DrawRectangle(D2D1::RectF(inRect), mForeBrush, inLineWidth);
}

void MWinDeviceImpl::StrokeLine(
	float inFromX,
	float inFromY,
	float inToX,
	float inToY,
	uint32_t inLineWidth)
{
	assert(mForeBrush);
	assert(mRenderTarget);

	mRenderTarget->DrawLine(D2D1::Point2F(inFromX, inFromY), D2D1::Point2F(inToX, inToY), mForeBrush, float(inLineWidth));
}

void MWinDeviceImpl::StrokeGeometry(MGeometryImpl &inGeometryImpl, float inLineWidth)
{
	MWinGeometryImpl &impl(static_cast<MWinGeometryImpl &>(inGeometryImpl));

	if (impl.mSink)
	{
		impl.mSink->Close();
		impl.mSink = nullptr;
	}

	mRenderTarget->DrawGeometry(impl.mPath, mForeBrush, inLineWidth);
}

void MWinDeviceImpl::FillGeometry(MGeometryImpl &inGeometryImpl)
{
	MWinGeometryImpl &impl(static_cast<MWinGeometryImpl &>(inGeometryImpl));

	if (impl.mSink)
	{
		impl.mSink->Close();
		impl.mSink = nullptr;
	}

	mRenderTarget->FillGeometry(impl.mPath, mForeBrush);
}

void MWinDeviceImpl::FillEllipse(
	MRect inRect)
{
	assert(mForeBrush);
	assert(mRenderTarget);

	float radius;
	if (inRect.height < inRect.width)
		radius = inRect.height / 2.f;
	else
		radius = inRect.width / 2.f;

	D2D1_ROUNDED_RECT r = D2D1::RoundedRect(D2D1::RectF(inRect), radius, radius);
	mRenderTarget->FillRoundedRectangle(r, mForeBrush);
}

void MWinDeviceImpl::DrawBitmap(
	const MBitmap &inBitmap,
	float inX,
	float inY)
{
	if (inBitmap.Data() != nullptr)
	{
		D2D1_BITMAP_PROPERTIES props =
			{
				{DXGI_FORMAT_B8G8R8A8_UNORM,
		         D2D1_ALPHA_MODE_PREMULTIPLIED},
				mDpiScaleX,
				mDpiScaleY};

		ComPtr<ID2D1Bitmap> bitmap;

		HRESULT err = mRenderTarget->CreateBitmap(D2D1::SizeU(inBitmap.Width(), inBitmap.Height()),
		                                          inBitmap.Data(), inBitmap.Stride(), &props, &bitmap);

		if (SUCCEEDED(err))
			mRenderTarget->DrawBitmap(bitmap, D2D1::RectF(inX, inY, inX + inBitmap.Width(), inY + inBitmap.Height()), 1.0f);
	}
}

void MWinDeviceImpl::CreateAndUsePattern(
	MColor inColor1,
	MColor inColor2,
	uint32_t inWidth,
	float inRotation)
{
	uint32_t data[8][8];

	uint32_t c1 = 0, c2 = 0;

	c1 |= inColor1.red << 16;
	c1 |= inColor1.green << 8;
	c1 |= inColor1.blue << 0;

	c2 |= inColor2.red << 16;
	c2 |= inColor2.green << 8;
	c2 |= inColor2.blue << 0;

	for (uint32_t y = 0; y < 8; ++y)
	{
		for (uint32_t x = 0; x < 8; ++x)
			data[y][x] = ((x / inWidth) & 1) ? c2 : c1;
	}

	ComPtr<ID2D1Bitmap> bitmap;
	THROW_IF_HRESULT_ERROR(mRenderTarget->CreateBitmap(D2D1::SizeU(8, 8), data, 32,
	                                                   D2D1::BitmapProperties(
														   D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
	                                                   &bitmap));

	ComPtr<ID2D1BitmapBrush> brush;
	if (inRotation == 0)
	{
		THROW_IF_HRESULT_ERROR(mRenderTarget->CreateBitmapBrush(
			bitmap,
			D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP),
			D2D1::BrushProperties(),
			&brush));
	}
	else
	{
		THROW_IF_HRESULT_ERROR(mRenderTarget->CreateBitmapBrush(
			bitmap,
			D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP),
			D2D1::BrushProperties(1.0f, D2D1::Matrix3x2F::Rotation(inRotation)),
			&brush));
	}

	mForeBrush = brush;
}

void MWinDeviceImpl::SetFont(
	const string &inFont)
{
	string::const_iterator e = inFont.end() - 1;

	float size = 0, n = 1;
	while (e != inFont.begin())
	{
		if (isdigit(*e))
		{
			size += n * (*e - '0');
			n *= 10;
			--e;
		}
		else if (*e == '.')
		{
			while (n > 1)
			{
				size /= 10;
				n /= 10;
			}
			--e;
		}
		else
			break;
	}

	if (e == inFont.end() or e == inFont.begin() or *e != ' ')
		THROW(("Error in specified font"));

	mFontFamily = c2w(inFont.substr(0, e - inFont.begin()));
	mFontSize = (size * 96.f) / (72.f * mDpiScaleY);

	// OK, so that's what the user requested, now find something suitable
	LookupFont(mFontFamily);
}

void MWinDeviceImpl::LookupFont(const wstring &inFamily)
{
	static map<wstring, IDWriteFontPtr> sFontTable;

	map<wstring, IDWriteFontPtr>::iterator f = sFontTable.find(inFamily);

	if (f != sFontTable.end())
	{
		if (mFont != f->second)
		{
			mFont = f->second;

			if (mTextFormat)
				mTextFormat = nullptr;
		}
	}
	else
	{
		ComPtr<IDWriteFontCollection> pFontCollection;
		THROW_IF_HRESULT_ERROR(GetDWFactory()->GetSystemFontCollection(&pFontCollection));
		uint32_t familyCount = pFontCollection->GetFontFamilyCount();

		for (uint32_t i = 0; i < familyCount; ++i)
		{
			ComPtr<IDWriteFontFamily> pFontFamily;
			THROW_IF_HRESULT_ERROR(pFontCollection->GetFontFamily(i, &pFontFamily));

			ComPtr<IDWriteLocalizedStrings> pFamilyNames;
			THROW_IF_HRESULT_ERROR(pFontFamily->GetFamilyNames(&pFamilyNames));

			uint32_t index = 0;
			BOOL exists = false;

			THROW_IF_HRESULT_ERROR(pFamilyNames->FindLocaleName(GetLocale().c_str(), &index, &exists));

			// If the specified locale doesn't exist, select the first on the list.
			if (not exists)
				index = 0;

			UINT32 length = 0;
			THROW_IF_HRESULT_ERROR(pFamilyNames->GetStringLength(index, &length));

			vector<wchar_t> name(length + 1);
			THROW_IF_HRESULT_ERROR(pFamilyNames->GetString(index, &name[0], length + 1));

			if (inFamily == &name[0])
			{
				pFontFamily->GetFont(index, &mFont);
				sFontTable[inFamily] = mFont;
				break;
			}
		}
	}
}

void MWinDeviceImpl::CreateTextFormat()
{
	if (not mTextFormat)
	{
		if (mFontFamily.empty())
		{
			mFontFamily = L"Consolas";
			mFontSize = 10 * 96.f / 72.f;
		}

		THROW_IF_HRESULT_ERROR(
			GetDWFactory()->CreateTextFormat(
				mFontFamily.c_str(), // Font family name.
				NULL,                // Font collection (NULL sets it to use the system font collection).
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				mFontSize,
				GetLocale().c_str(),
				&mTextFormat));
	}
}

float MWinDeviceImpl::GetAscent()
{
	if (not mFont)
		SetFont(Preferences::GetString("font", "Consolas 10"));

	DWRITE_FONT_METRICS metrics;
	mFont->GetMetrics(&metrics);
	return metrics.ascent * mFontSize / metrics.designUnitsPerEm;
}

float MWinDeviceImpl::GetDescent()
{
	if (not mFont)
		SetFont(Preferences::GetString("font", "Consolas 10"));

	DWRITE_FONT_METRICS metrics;
	mFont->GetMetrics(&metrics);
	return metrics.descent * mFontSize / metrics.designUnitsPerEm;
}

int32_t MWinDeviceImpl::GetLineHeight()
{
	if (not mFont)
		SetFont(Preferences::GetString("font", "Consolas 10"));

	DWRITE_FONT_METRICS metrics;
	mFont->GetMetrics(&metrics);
	return static_cast<int32_t>(
		ceil((metrics.ascent + metrics.descent + metrics.lineGap) * mFontSize / metrics.designUnitsPerEm));
}

float MWinDeviceImpl::GetXWidth()
{
	if (not mFont)
		SetFont(Preferences::GetString("font", "Consolas 10"));

	CreateTextFormat();

	IDWriteTextLayout *layout = nullptr;

	THROW_IF_HRESULT_ERROR(
		GetDWFactory()->CreateTextLayout(L"xxxxxxxxxx", 10, mTextFormat, 99999.0f, 99999.0f, &layout));

	DWRITE_TEXT_METRICS metrics;
	THROW_IF_HRESULT_ERROR(layout->GetMetrics(&metrics));

	layout->Release();

	return metrics.width / 10;
}

void MWinDeviceImpl::DrawString(
	const string &inText,
	float inX,
	float inY,
	uint32_t inTruncateWidth,
	MAlignment inAlign)
{
	IDWriteTextFormatPtr savedFormat(mTextFormat), nil;
	mTextFormat = nil;

	CreateTextFormat();

	wstring s(c2w(inText));
	mRenderTarget->DrawTextW(s.c_str(), s.length(),
	                         mTextFormat, D2D1::RectF(inX, inY, 200.f, inY + 14.f), mForeBrush);

	mTextFormat = savedFormat;
}

void MWinDeviceImpl::DrawString(
	const string &inText,
	MRect inBounds,
	MAlignment inAlign)
{
	IDWriteTextFormatPtr savedFormat(mTextFormat), nil;
	mTextFormat = nil;

	CreateTextFormat();

	switch (inAlign)
	{
		default: mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); break;
		case eAlignCenter: mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); break;
		case eAlignRight: mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING); break;
	}

	if (not mTrimmingSign)
		GetDWFactory()->CreateEllipsisTrimmingSign(mTextFormat, &mTrimmingSign);

	DWRITE_TRIMMING trimOptions = {DWRITE_TRIMMING_GRANULARITY_CHARACTER};
	mTextFormat->SetTrimming(&trimOptions, mTrimmingSign);

	wstring s(c2w(inText));
	mRenderTarget->DrawTextW(s.c_str(), s.length(),
	                         mTextFormat, D2D1::RectF(inBounds), mForeBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

	mTextFormat = savedFormat;
}

void MWinDeviceImpl::SetText(
	const string &inText)
{
	CreateTextFormat();

	// reset
	mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	mText.clear();
	mText.reserve(inText.length());

	mTextIndex.clear();
	mTextIndex.reserve(inText.length());

	mSelectionLength = 0;

	typedef tr1::tuple<uint32_t, uint32_t, uint32_t> replaced_char;
	vector<replaced_char> replaced;

	for (string::const_iterator i = inText.begin(); i != inText.end(); ++i)
	{
		uint32_t ch = static_cast<unsigned char>(*i);
		mTextIndex.push_back(mText.length());

		if (ch & 0x0080)
		{
			if ((ch & 0x0E0) == 0x0C0)
			{
				uint32_t ch1 = static_cast<unsigned char>(*(i + 1));
				if ((ch1 & 0x0c0) == 0x080)
				{
					ch = ((ch & 0x01f) << 6) | (ch1 & 0x03f);
					i += 1;
					mTextIndex.push_back(mText.length());
				}
			}
			else if ((ch & 0x0F0) == 0x0E0)
			{
				uint32_t ch1 = static_cast<unsigned char>(*(i + 1));
				uint32_t ch2 = static_cast<unsigned char>(*(i + 2));
				if ((ch1 & 0x0c0) == 0x080 and (ch2 & 0x0c0) == 0x080)
				{
					ch = ((ch & 0x00F) << 12) | ((ch1 & 0x03F) << 6) | (ch2 & 0x03F);
					i += 2;
					mTextIndex.push_back(mText.length());
					mTextIndex.push_back(mText.length());
				}
			}
			else if ((ch & 0x0F8) == 0x0F0)
			{
				uint32_t ch1 = static_cast<unsigned char>(*(i + 1));
				uint32_t ch2 = static_cast<unsigned char>(*(i + 2));
				uint32_t ch3 = static_cast<unsigned char>(*(i + 3));
				if ((ch1 & 0x0c0) == 0x080 and (ch2 & 0x0c0) == 0x080 and (ch3 & 0x0c0) == 0x080)
				{
					ch = ((ch & 0x007) << 18) | ((ch1 & 0x03F) << 12) | ((ch2 & 0x03F) << 6) | (ch3 & 0x03F);
					i += 3;
					mTextIndex.push_back(mText.length());
					mTextIndex.push_back(mText.length());
					mTextIndex.push_back(mText.length());
				}
			}
		}

		if (mReplaceUnknownCharacters)
		{
			assert(mFont);
			BOOL exists;
			mFont->HasCharacter(ch, &exists);
			if (not exists)
				replaced.push_back(tr1::make_tuple(mText.length(), ch <= 0x0FFFF ? 1 : 2, ch));
		}

		if (ch <= 0x0FFFF)
			mText += static_cast<wchar_t>(ch);
		else
		{
			wchar_t h = (ch - 0x010000) / 0x0400 + 0x0D800;
			wchar_t l = (ch - 0x010000) % 0x0400 + 0x0DC00;

			mText += h;
			mText += l;
		}
	}

	mTextIndex.push_back(mText.length());
	mTextLayout = nullptr;

	THROW_IF_HRESULT_ERROR(
		GetDWFactory()->CreateTextLayout(
			mText.c_str(),
			mText.length(),
			mTextFormat,
			99999.0f,
			99999.0f,
			&mTextLayout));

	if (not replaced.empty())
	{
		float w = GetXWidth();
		float h = float(GetLineHeight());
		float b = GetAscent();
		MColor color = GetForeColor();

		for (auto r : replaced)
		{
			ComPtr<MInlineReplacedChar> replaceChar(
				new MInlineReplacedChar(mRenderTarget, tr1::get<2>(r), color, w, h, b, mTextFormat));
			DWRITE_TEXT_RANGE textRange = {tr1::get<0>(r), tr1::get<1>(r)};
			mTextLayout->SetInlineObject(replaceChar, textRange);
		}
	}
}

void MWinDeviceImpl::SetTabStops(
	float inTabWidth)
{
	assert(mTextLayout);
	//	if (not mTextLayout)
	//		THROW(("SetText must be called first!"));
	mTextLayout->SetIncrementalTabStop(inTabWidth /** 96.f / 72.f*/);
}

void MWinDeviceImpl::SetTextColors(
	uint32_t inColorCount,
	uint32_t inColorIndices[],
	uint32_t inOffsets[],
	MColor inColors[])
{
	if (not mRenderTarget) // short cut
		return;

	for (uint32_t ix = 0; ix < inColorCount; ++ix)
	{
		MColor c = inColors[inColorIndices[ix]];

		//		ComPtr<MTextColor> color(new MTextColor(c));
		//
		//		DWRITE_TEXT_RANGE range;
		//		range.startPosition = mTextIndex[inOffsets[ix]];
		//		if (ix == inColorCount - 1)
		//			range.length = mText.length() - range.startPosition;
		//		else
		//			range.length = mTextIndex[inOffsets[ix + 1]] - range.startPosition;
		//
		//		mTextLayout->SetDrawingEffect(color, range);

		ComPtr<ID2D1SolidColorBrush> textColorBrush;
		THROW_IF_HRESULT_ERROR(mRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(c.red / 255.f, c.green / 255.f, c.blue / 255.f),
			&textColorBrush));

		DWRITE_TEXT_RANGE range;
		range.startPosition = mTextIndex[inOffsets[ix]];
		if (ix == inColorCount - 1)
			range.length = mText.length() - range.startPosition;
		else
			range.length = mTextIndex[inOffsets[ix + 1]] - range.startPosition;

		mTextLayout->SetDrawingEffect(textColorBrush, range);
	}
}

void MWinDeviceImpl::SetTextStyles(
	uint32_t inStyleCount,
	uint32_t inStyles[],
	uint32_t inOffsets[])
{
	if (not mRenderTarget) // short cut
		return;

	for (uint32_t ix = 0; ix < inStyleCount; ++ix)
	{
		DWRITE_TEXT_RANGE range;
		range.startPosition = mTextIndex[inOffsets[ix]];
		if (ix == inStyleCount - 1)
			range.length = mText.length() - range.startPosition;
		else
			range.length = mTextIndex[inOffsets[ix + 1]] - range.startPosition;

		mTextLayout->SetFontWeight(
			inStyles[ix] & MDevice::eTextStyleBold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
			range);

		mTextLayout->SetFontStyle(
			inStyles[ix] & MDevice::eTextStyleItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			range);

		mTextLayout->SetUnderline(
			inStyles[ix] & MDevice::eTextStyleUnderline, range);
	}
}

void MWinDeviceImpl::SetTextSelection(
	uint32_t inStart,
	uint32_t inLength,
	MColor inSelectionColor)
{
	mSelectionColor = inSelectionColor;
	mSelectionStart = mTextIndex[inStart];
	mSelectionLength = mTextIndex[inStart + inLength] - mSelectionStart;
}

void MWinDeviceImpl::IndexToPosition(
	uint32_t inIndex,
	bool inTrailing,
	int32_t &outPosition)
{
	// Translate text character offset to point x,y.
	DWRITE_HIT_TEST_METRICS caretMetrics;
	float caretX = 0, caretY = 0;

	if (mTextLayout and inIndex < mTextIndex.size())
	{
		int32_t offset = mTextIndex[inIndex];

		mTextLayout->HitTestTextPosition(
			offset, inTrailing, &caretX, &caretY, &caretMetrics);

		outPosition = static_cast<uint32_t>(caretX + 0.5f);
	}
	else
		outPosition = 0;
}

bool MWinDeviceImpl::PositionToIndex(
	int32_t inPosition,
	uint32_t &outIndex)
{
	if (not mTextLayout)
		outIndex = 0;
	else
	{
		BOOL isTrailingHit, isInside;
		DWRITE_HIT_TEST_METRICS caretMetrics;

		float x = static_cast<float>(inPosition);
		float y = GetAscent();

		mTextLayout->HitTestPoint(x, y, &isTrailingHit, &isInside, &caretMetrics);

		if (isTrailingHit)
			++caretMetrics.textPosition;

		// remap the wchar_t index into our UTF-8 string
		outIndex = MapBack(caretMetrics.textPosition);
	}

	return true;
}

float MWinDeviceImpl::GetTextWidth()
{
	DWRITE_TEXT_METRICS metrics;
	THROW_IF_HRESULT_ERROR(mTextLayout->GetMetrics(&metrics));

	return metrics.widthIncludingTrailingWhitespace;
}

void MWinDeviceImpl::RenderText(
	float inX,
	float inY)
{
	if (mTextLayout)
	{
		DWRITE_TEXT_METRICS metrics;
		mTextLayout->GetMetrics(&metrics);

		MRect r(inX + metrics.left, inY + metrics.top, metrics.width, metrics.height);
		if (not(mClipping.empty() or mClipping.top().Intersects(r)))
			return;

		if (mSelectionLength > 0)
		{
			ComPtr<ID2D1SolidColorBrush> selectionColorBrush;
			THROW_IF_HRESULT_ERROR(mRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(mSelectionColor.red / 255.f, mSelectionColor.green / 255.f, mSelectionColor.blue / 255.f),
				&selectionColorBrush));

			DWRITE_TEXT_RANGE caretRange = {mSelectionStart, mSelectionLength};
			UINT32 actualHitTestCount = 0;

			// Determine actual number of hit-test ranges
			mTextLayout->HitTestTextRange(caretRange.startPosition, caretRange.length,
			                              inX, inY, NULL, 0, &actualHitTestCount);

			// Allocate enough room to return all hit-test metrics.
			std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(actualHitTestCount);

			mTextLayout->HitTestTextRange(caretRange.startPosition, caretRange.length,
			                              inX, inY, &hitTestMetrics[0], static_cast<UINT32>(hitTestMetrics.size()),
			                              &actualHitTestCount);

			// Draw the selection ranges behind the text.
			if (actualHitTestCount > 0)
			{
				// Note that an ideal layout will return fractional values,
				// so you may see slivers between the selection ranges due
				// to the per-primitive antialiasing of the edges unless
				// it is disabled (better for performance anyway).

				for (size_t i = 0; i < actualHitTestCount; ++i)
				{
					const DWRITE_HIT_TEST_METRICS &htm = hitTestMetrics[i];
					D2D1_RECT_F highlightRect = {
						htm.left,
						htm.top,
						(htm.left + htm.width),
						(htm.top + htm.height)};

					mRenderTarget->FillRectangle(highlightRect, selectionColorBrush);
				}
			}
		}

		if (mDrawWhiteSpace)
		{
			vector<DWRITE_CLUSTER_METRICS> clusters(mText.size() + 1);
			uint32_t count;

			HRESULT err = mTextLayout->GetClusterMetrics(&clusters[0], clusters.size(), &count);
			if (err == E_NOT_SUFFICIENT_BUFFER)
			{
				DWRITE_CLUSTER_METRICS v = {};
				clusters.insert(clusters.end(), clusters.size() - count, v);
				err = mTextLayout->GetClusterMetrics(&clusters[0], clusters.size(), &count);
			}

			if (SUCCEEDED(err))
			{
				float x = inX;
				uint32_t offset = 0;

				ComPtr<ID2D1SolidColorBrush> brush;
				err = mRenderTarget->CreateSolidColorBrush(
					D2D1::ColorF(
						mWhitespaceColor.red / 255.f,
						mWhitespaceColor.green / 255.f,
						mWhitespaceColor.blue / 255.f),
					&brush);

				for (auto cluster : clusters)
				{
					if (cluster.isWhitespace)
					{
						const wchar_t *s;
						switch (mText[offset])
						{
							case ' ': s = L"."; break;
							case '\t': s = L"\xbb"; break;
							case '\n':
							case '\r': s = L"\xac"; break;
							default: s = L"?"; break;
						}

						mRenderTarget->DrawTextW(s, 1, mTextFormat,
						                         D2D1::RectF(x, inY, x + cluster.width, inY + metrics.height), brush);
					}

					x += cluster.width;
					offset += cluster.length;
				}
			}
		}

		//		MTextRenderer renderer(GetD2D1Factory(), mRenderTarget);
		//		mTextLayout->Draw(nullptr, &renderer, inX, inY);

		mRenderTarget->DrawTextLayout(D2D1::Point2F(inX, inY), mTextLayout, mForeBrush);
	}
}

void MWinDeviceImpl::DrawCaret(
	float inX,
	float inY,
	uint32_t inOffset)
{
	// Translate text character offset to point x,y.
	DWRITE_HIT_TEST_METRICS caretMetrics = {};
	float caretX = 0, caretY = 0;

	if (mTextLayout)
	{
		int32_t offset;

		if (inOffset < mTextIndex.size())
			offset = mTextIndex[inOffset];
		else
			offset = mText.length() + 1;

		mTextLayout->HitTestTextPosition(
			offset, false, &caretX, &caretY, &caretMetrics);
	}
	else
		caretMetrics.height = static_cast<float>(GetLineHeight());

	// The default thickness of 1 pixel is almost _too_ thin on modern large monitors,
	// but we'll use it.
	DWORD caretIntThickness = static_cast<int32_t>(2 / mDpiScaleX);
	::SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretIntThickness, FALSE);
	const float caretThickness = float(caretIntThickness);

	SetForeColor(kBlack);

	mRenderTarget->FillRectangle(
		D2D1::RectF(inX + caretX - caretThickness / 2, inY + caretY,
	                inX + caretX + caretThickness / 2, floor(inY + caretY + caretMetrics.height)),
		mForeBrush);
}

void MWinDeviceImpl::RenderTextBackground(
	float inX,
	float inY,
	uint32_t inStart,
	uint32_t inLength,
	MColor inColor)
{
	if (mTextLayout)
	{
		DWRITE_TEXT_METRICS metrics;
		mTextLayout->GetMetrics(&metrics);

		MRect r(inX + metrics.left, inY + metrics.top, metrics.width, metrics.height);
		if (not(mClipping.empty() or mClipping.top().Intersects(r)))
			return;

		ComPtr<ID2D1SolidColorBrush> colorBrush;
		THROW_IF_HRESULT_ERROR(mRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(inColor.red / 255.f, inColor.green / 255.f, inColor.blue / 255.f),
			&colorBrush));

		DWRITE_TEXT_RANGE range;
		range.startPosition = mTextIndex[inStart];
		range.length = mTextIndex[inStart + inLength] - range.startPosition;

		UINT32 actualHitTestCount = 0;

		// Determine actual number of hit-test ranges
		mTextLayout->HitTestTextRange(range.startPosition, range.length,
		                              inX, inY, NULL, 0, &actualHitTestCount);

		// Allocate enough room to return all hit-test metrics.
		std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(actualHitTestCount);

		mTextLayout->HitTestTextRange(range.startPosition, range.length,
		                              inX, inY, &hitTestMetrics[0], static_cast<UINT32>(hitTestMetrics.size()),
		                              &actualHitTestCount);

		// Draw the selection ranges behind the text.
		if (actualHitTestCount > 0)
		{
			// Note that an ideal layout will return fractional values,
			// so you may see slivers between the selection ranges due
			// to the per-primitive antialiasing of the edges unless
			// it is disabled (better for performance anyway).

			for (size_t i = 0; i < actualHitTestCount; ++i)
			{
				const DWRITE_HIT_TEST_METRICS &htm = hitTestMetrics[i];
				D2D1_RECT_F highlightRect = {
					htm.left,
					htm.top,
					(htm.left + htm.width),
					(htm.top + htm.height)};

				mRenderTarget->FillRectangle(highlightRect, colorBrush);
			}
		}
	}
}

void MWinDeviceImpl::SetScale(
	float inScaleX,
	float inScaleY,
	float inCenterX,
	float inCenterY)
{
	mRenderTarget->SetTransform(
		D2D1::Matrix3x2F::Scale(
			D2D1::Size(inScaleX, inScaleY),
			D2D1::Point2F(inCenterX, inCenterY)));
}

void MWinDeviceImpl::BreakLines(
	uint32_t inWidth,
	vector<uint32_t> &outBreaks)
{
	mTextLayout->SetMaxWidth(static_cast<float>(inWidth));

	uint32_t lineCount = 0;
	mTextLayout->GetLineMetrics(nullptr, 0, &lineCount);
	if (lineCount > 0)
	{
		vector<DWRITE_LINE_METRICS> lineMetrics(lineCount);
		THROW_IF_HRESULT_ERROR(
			mTextLayout->GetLineMetrics(&lineMetrics[0], lineCount, &lineCount));

		uint32_t offset = 0;
		for (DWRITE_LINE_METRICS &m : lineMetrics)
		{
			offset += m.length;
			outBreaks.push_back(MapBack(offset));
		}
	}
}

uint32_t MWinDeviceImpl::MapBack(
	uint32_t inOffset)
{
	vector<uint16_t>::iterator ix =
		find(mTextIndex.begin(), mTextIndex.end(), inOffset);

	uint32_t result;

	if (ix == mTextIndex.end())
		result = mText.length();
	else
		result = ix - mTextIndex.begin();

	return result;
}

// --------------------------------------------------------------------

MDeviceImpl *MDeviceImpl::Create()
{
	return new MWinDeviceImpl();
}

MDeviceImpl *MDeviceImpl::Create(MView *inView, MRect inRect)
{
	return new MWinDeviceImpl(inView, inRect);
}

// --------------------------------------------------------------------

//#include <AeroStyle.xml>

void MDevice::GetSysSelectionColor(MColor &outColor)
{
	// just like Visual Studio does, we take a 2/3 mix of
	// the system highlight color and white. This way we
	// can still use the syntax highlighted colors and don't
	// have to fall back to some recalculated colors.

	COLORREF clr = ::GetSysColor(COLOR_HIGHLIGHT);
	if (clr != 0)
	{
		uint32_t red = (clr & 0x000000FF);
		red = (2 * red + 3 * 255) / 5;
		if (red > 255)
			red = 255;
		outColor.red = red;

		clr >>= 8;

		uint32_t green = (clr & 0x000000FF);
		green = (2 * green + 3 * 255) / 5;
		if (green > 255)
			green = 255;
		outColor.green = green;

		clr >>= 8;

		uint32_t blue = (clr & 0x000000FF);
		blue = (2 * blue + 3 * 255) / 5;
		if (blue > 255)
			blue = 255;
		outColor.blue = blue;
	}
}

// --------------------------------------------------------------------

template <class T>
inline void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

void MDevice::ListFonts(bool inFixedWidthOnly, vector<string> &outFonts)
{
	IDWriteFactory *pDWriteFactory = MWinDeviceImpl::GetDWFactory();

	ComPtr<IDWriteFontCollection> pFontCollection;

	// Get the system font collection.
	HRESULT hr = pDWriteFactory->GetSystemFontCollection(&pFontCollection);

	UINT32 familyCount = 0;

	// Get the number of font families in the collection.
	if (SUCCEEDED(hr))
	{
		familyCount = pFontCollection->GetFontFamilyCount();
	}

	for (UINT32 i = 0; i < familyCount; ++i)
	{
		ComPtr<IDWriteFontFamily> pFontFamily;

		// Get the font family.
		if (SUCCEEDED(hr))
		{
			hr = pFontCollection->GetFontFamily(i, &pFontFamily);
		}

		ComPtr<IDWriteLocalizedStrings> pFamilyNames;

		// Get a list of localized strings for the family name.
		if (SUCCEEDED(hr))
		{
			hr = pFontFamily->GetFamilyNames(&pFamilyNames);
		}

		UINT32 index = 0;
		BOOL exists = false;

		wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

		if (SUCCEEDED(hr))
		{
			// Get the default locale for this user.
			int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

			// If the default locale is returned, find that locale name, otherwise use "en-us".
			if (defaultLocaleSuccess)
			{
				hr = pFamilyNames->FindLocaleName(localeName, &index, &exists);
			}
			if (SUCCEEDED(hr) && !exists) // if the above find did not find a match, retry with US English
			{
				hr = pFamilyNames->FindLocaleName(L"en-us", &index, &exists);
			}
		}

		// If the specified locale doesn't exist, select the first on the list.
		if (!exists)
			index = 0;

		UINT32 length = 0;

		// Get the string length.
		if (SUCCEEDED(hr))
		{
			hr = pFamilyNames->GetStringLength(index, &length);
		}

		// Allocate a string big enough to hold the name.
		vector<wchar_t> name(length + 1);

		// Get the family name.
		if (SUCCEEDED(hr))
		{
			hr = pFamilyNames->GetString(index, &name[0], length + 1);
		}
		if (SUCCEEDED(hr))
		{
			string fontName = w2c(&name[0]);

			if (inFixedWidthOnly)
			{
				try
				{
					// now see if this font is 'fixed width'
					MDevice dev;
					dev.SetFont(fontName + " 10");
					dev.SetText("iii");
					float w1 = dev.GetTextWidth();
					dev.SetText("www");
					float w2 = dev.GetTextWidth();
					if (w1 != w2)
						continue;
				}
				catch (...)
				{
				}
			}

			outFonts.push_back(fontName);
		}
	}
}

void MWinDeviceImpl::SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor)
{
	mDrawWhiteSpace = inDrawWhiteSpace;
	mWhitespaceColor = inWhiteSpaceColor;
}

void MWinDeviceImpl::SetReplaceUnknownCharacters(bool inReplaceUnknownCharacters)
{
	mReplaceUnknownCharacters = inReplaceUnknownCharacters;
}

// Theme support

void MWinDeviceImpl::DrawListItemBackground(MRect inBounds, MListItemState inState)
{
	Save();

	MColor backColor = GetBackColor();
	MColor selectColor = kWhite;

	if (inState == eLIS_HotSelected or inState == eLIS_Selected or inState == eLIS_SelectedNotFocus)
	{
		MDevice::GetSysSelectionColor(selectColor);
		SetBackColor(selectColor);
	}

	EraseRect(inBounds);

	CreateAndUsePattern(MColor(0.5f, 0.5f, 0.5f), selectColor, 1, 0);

	float x = float(inBounds.x);
	float y = float(inBounds.y + inBounds.height);

	StrokeLine(x, y, x + inBounds.width, y, 1);

	Restore();

	//if (inState == eLIS_None)
	//	EraseRect(inBounds);
	//else
	//{
	//	ComPtr<ID2D1GdiInteropRenderTarget> gdiRT;
	//
	//	HRESULT hr = mRenderTarget->QueryInterface(
	//		__uuidof(ID2D1GdiInteropRenderTarget), (void**)&gdiRT);
	//
	//	HDC dc;
	//	if (SUCCEEDED(hr))
	//		hr = gdiRT->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &dc);
	//
	//	if (SUCCEEDED(hr))
	//	{
	//		RECT r = { inBounds.x, inBounds.y, inBounds.x + inBounds.width,
	//			inBounds.y + inBounds.height
	//		};
	//
	//		HWND w = static_cast<MWinWindowImpl*>(mView->GetWindow()->GetImpl())->GetHandle();
	//
	//		HTHEME theme = ::OpenThemeData(w, VSCLASS_LISTVIEWSTYLE);
	//		if (theme != nullptr)
	//		{
	//			::DrawThemeBackground(theme, dc, LVP_LISTITEM,
	//				(int)inState, &r, nullptr);
	//			::CloseThemeData(theme);
	//		}
	//
	//		gdiRT->ReleaseDC(&r);
	//	}
	//}
}

// --------------------------------------------------------------------

MWinGeometryImpl::MWinGeometryImpl(MWinDeviceImpl &inDevice, MGeometryFillMode inMode)
{
	MWinDeviceImpl::GetD2D1Factory()->CreatePathGeometry(&mPath);
	if (mPath)
	{
		if (SUCCEEDED(mPath->Open(&mSink)))
		{
			if (inMode == eGeometryFillModeWinding)
				mSink->SetFillMode(D2D1_FILL_MODE_WINDING);
			else
				mSink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);
		}
	}
}

MWinGeometryImpl::~MWinGeometryImpl()
{
}

void MWinGeometryImpl::Begin(float inX, float inY, MGeometryBegin inBegin)
{
	if (not mSink)
		THROW(("no sink"));

	if (inBegin == eGeometryBeginFilled)
		mSink->BeginFigure(D2D1::Point2F(inX, inY), D2D1_FIGURE_BEGIN_FILLED);
	else
		mSink->BeginFigure(D2D1::Point2F(inX, inY), D2D1_FIGURE_BEGIN_HOLLOW);
}

void MWinGeometryImpl::LineTo(float inX, float inY)
{
	if (not mSink)
		THROW(("no sink"));
	mSink->AddLine(D2D1::Point2F(inX, inY));
}

void MWinGeometryImpl::CurveTo(float inX1, float inY1, float inX2, float inY2, float inX3, float inY3)
{
	if (not mSink)
		THROW(("No sink"));

	mSink->AddBezier(
		D2D1::BezierSegment(
			D2D1::Point2F(inX1, inY1),
			D2D1::Point2F(inX2, inY2),
			D2D1::Point2F(inX3, inY3)));
}

void MWinGeometryImpl::End(bool inClose)
{
	if (inClose)
		mSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	else
		mSink->EndFigure(D2D1_FIGURE_END_OPEN);
}

MGeometryImpl *MGeometryImpl::Create(MDevice &inDevice, MGeometryFillMode inMode)
{
	return new MWinGeometryImpl(*static_cast<MWinDeviceImpl *>(inDevice.GetImpl()), inMode);
}

// PNG support

MBitmap::MBitmap(const void *inPNG, uint32_t inLength)
	: mData(nullptr)
	, mWidth(0)
	, mHeight(0)
	, mStride(0)
	, mUseAlpha(true)
{
	ComPtr<IWICImagingFactory> factory;
	if (::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)) == S_OK)
	{
		ComPtr<IWICStream> stream;
		ComPtr<IWICBitmapDecoder> decoder;
		ComPtr<IWICBitmapFrameDecode> frame;
		ComPtr<IWICBitmapSource> bitmap;

		if (factory->CreateStream(&stream) == S_OK and
		    stream->InitializeFromMemory(reinterpret_cast<BYTE *>(const_cast<void *>(inPNG)), inLength) == S_OK and
		    factory->CreateDecoderFromStream(stream, NULL, WICDecodeMetadataCacheOnLoad, &decoder) == S_OK and
		    decoder->GetFrame(0, &frame) == S_OK and
		    ::WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, frame, &bitmap) == S_OK)
		{
			bitmap->GetSize(&mWidth, &mHeight);

			mStride = mWidth * sizeof(uint32_t);

			mData = new uint32_t[mWidth * mHeight];
			bitmap->CopyPixels(nullptr, mStride, mWidth * mHeight * sizeof(uint32_t), (BYTE *)mData);
		}
	}
}
