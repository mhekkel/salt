//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "MWinCanvasImpl.hpp"
#include "MControls.hpp"
#include "MWinControlsImpl.hpp"
#include "MWinWindowImpl.hpp"
#include "MWinDeviceImpl.hpp"
#include "MWinUtils.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MUnicode.hpp"

using namespace std;

// --------------------------------------------------------------------

namespace
{
	
UINT
	sCFSTR_FILEDESCRIPTOR = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR),
	sCFSTR_MWinCanvasImpl = ::RegisterClipboardFormat(L"MWinCanvasImplPtr");

class MDropTarget : public IDropTarget
{
  public:
					MDropTarget(MCanvas* inTarget, bool inFiles, bool inText);
	virtual			~MDropTarget();

	HRESULT __stdcall DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT __stdcall DragLeave();
	HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT __stdcall Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(IID const& riid, void** ppvObject);

	void			GetEffect(DWORD grfKeyState, DWORD& outEffect);

  private:
	MCanvas*		mTarget;
	bool			mAcceptDropFiles, mAcceptDropText;
	MCanvas*		mSource;
	vector<fs::path>mFiles;
	unsigned long	mRefCount;
};

MDropTarget::MDropTarget(MCanvas* inTarget, bool inFiles, bool inText)
	: mTarget(inTarget)
	, mAcceptDropFiles(inFiles)
	, mAcceptDropText(inText)
	, mSource(nullptr)
	, mRefCount(0)
{
}

MDropTarget::~MDropTarget()
{
}

void MDropTarget::GetEffect(DWORD grfKeyState, DWORD& outEffect)
{
	if (outEffect & DROPEFFECT_COPY and (grfKeyState & MK_CONTROL or mTarget != mSource))
		outEffect = DROPEFFECT_COPY;
	else if (outEffect & DROPEFFECT_MOVE and mTarget == mSource)
		outEffect = DROPEFFECT_MOVE;
	else
		outEffect = 0;	
}

HRESULT __stdcall MDropTarget::DragEnter(
	IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	FORMATETC
		fmt_text = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
		fmt_file = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
		fmt_canv = { sCFSTR_MWinCanvasImpl, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
//		fmt_file = { sCFSTR_FILEDESCRIPTOR, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	HRESULT result = S_FALSE;
	
	mFiles.clear();
	
	STGMEDIUM stgmed;
	if (pDataObj->GetData(&fmt_canv, &stgmed) == S_OK)
	{
		if (stgmed.tymed & TYMED_HGLOBAL)
		{
			void** p = (void**)::GlobalLock(stgmed.hGlobal);
			uint32_t len = ::GlobalSize(stgmed.hGlobal);
			if (p != nullptr)
			{
				mSource = (MCanvas*)*p;
				::GlobalUnlock(stgmed.hGlobal);
			}
		}
		::ReleaseStgMedium(&stgmed);
	}

	if (mAcceptDropText and pDataObj->QueryGetData(&fmt_text) == S_OK)
	{
		GetEffect(grfKeyState, *pdwEffect);

		if (*pdwEffect)
		{
			mTarget->DragEnter();
			result = S_OK;
		}
	}
	
	if (mAcceptDropFiles and pDataObj->QueryGetData(&fmt_file) == S_OK and *pdwEffect & DROPEFFECT_COPY)
	{
		STGMEDIUM stgmed;
		if (pDataObj->GetData(&fmt_file, &stgmed) == S_OK)
		{
			if (stgmed.tymed & TYMED_HGLOBAL)
			{
				*pdwEffect = DROPEFFECT_COPY;
		
				HDROP drop = (HDROP)::GlobalLock(stgmed.hGlobal);
				uint32_t len = ::GlobalSize(stgmed.hGlobal);
				if (drop != nullptr)
				{
					uint32_t n = ::DragQueryFile(drop, 0xFFFFFFFF, nullptr, 0);
					for (uint32_t i = 0; i < n; ++i)
					{
						wchar_t path[1024];
						if (::DragQueryFile(drop, i, path, sizeof(path)) != 0)
						{
							string p(w2c(path));
							mFiles.push_back(p);
						}
					}
					
					::GlobalUnlock(stgmed.hGlobal);
				}
			}
			::ReleaseStgMedium(&stgmed);
		
			if (not mFiles.empty())
			{
				mTarget->DragEnter();
				result = S_OK;
			}
		}
	}
	
	return result;
}

HRESULT __stdcall MDropTarget::DragLeave()
{
	mTarget->DragLeave();
	return S_OK;
}

HRESULT __stdcall MDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if (mFiles.empty())
	{
		GetEffect(grfKeyState, *pdwEffect);
	
		int32_t x(pt.x), y(pt.y);
		mTarget->ConvertFromScreen(x, y);
	
		if (not mTarget->DragWithin(x, y))
			*pdwEffect = 0;
	}

	return S_OK;
}

HRESULT __stdcall MDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	FORMATETC
		fmt_text = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
		fmt_file = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
		fmt_canv = { sCFSTR_MWinCanvasImpl, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	HRESULT result = S_FALSE;

	int32_t x(pt.x), y(pt.y);
	mTarget->ConvertFromScreen(x, y);

	if (not mFiles.empty())
	{
		mTarget->DragLeave();
		
		for (fs::path& file: mFiles)
			mTarget->Drop(x, y, file);
	}
	else
	{
		GetEffect(grfKeyState, *pdwEffect);
	
		if (*pdwEffect)
		{
			STGMEDIUM stgmed;
			if (pDataObj->GetData(&fmt_text, &stgmed) == S_OK)
			{
				if (stgmed.tymed & TYMED_HGLOBAL)
				{
					const wchar_t* wtext = (const wchar_t*)::GlobalLock(stgmed.hGlobal);
					uint32_t len = ::GlobalSize(stgmed.hGlobal);
					if (wtext != nullptr)
					{
						unique_ptr<MDecoder> decoder(MDecoder::GetDecoder(kEncodingUTF16LE, wtext, len));
						string text;
						decoder->GetText(text);
	
						mTarget->Drop(
							*pdwEffect == DROPEFFECT_MOVE, x, y, text);
	
						result = S_OK;
						::GlobalUnlock(stgmed.hGlobal);
					}
				}
				::ReleaseStgMedium(&stgmed);
			}
		}
	}
	
	return result;
}

unsigned long __stdcall MDropTarget::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}

unsigned long __stdcall MDropTarget::Release()
{
	unsigned long newCount = InterlockedDecrement(&mRefCount);

    if (newCount == 0)
    {
        delete this;
        return 0;
    }

    return newCount;
}

HRESULT __stdcall MDropTarget::QueryInterface(IID const& riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(MDropTarget, IDropTarget),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// --------------------------------------------------------------------

class MDropSource : public IDropSource, public IDataObject
{
  public:
						MDropSource(MCanvas* inCanvas);
	virtual				~MDropSource();
	
	// IDropSource interface
	HRESULT __stdcall GiveFeedback(DWORD dwEffect);
	HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);

	// IDataObject interface
	HRESULT __stdcall GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
	HRESULT __stdcall QueryGetData(FORMATETC* pformatetc);

	HRESULT __stdcall GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
	HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut);        
	HRESULT __stdcall SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
	HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
	HRESULT __stdcall DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
		{ return OLE_E_ADVISENOTSUPPORTED; }
	HRESULT __stdcall DUnadvise(DWORD dwConnection)
		{ return OLE_E_ADVISENOTSUPPORTED; }
	HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
		{ return OLE_E_ADVISENOTSUPPORTED; }

	// IUnknown interface
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT __stdcall QueryInterface(IID const& riid, void** ppvObject);

  private:
	MCanvas*			mControl;
	unsigned long		mRefCount;
};
	
MDropSource::MDropSource(MCanvas* inCanvas)
	: mControl(inCanvas)
	, mRefCount(0)
{
}

MDropSource::~MDropSource()
{
}

HRESULT __stdcall MDropSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

HRESULT __stdcall MDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	HRESULT result = S_OK;

	if (fEscapePressed)
		result = DRAGDROP_S_CANCEL;
	
	if ((grfKeyState & MK_LBUTTON) == 0)
		result = DRAGDROP_S_DROP;
	
	return result;
}

HRESULT __stdcall MDropSource::GetData(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
	HRESULT result = DV_E_FORMATETC;

	if (pformatetc != nullptr and pformatetc->tymed == TYMED_HGLOBAL and
		pformatetc->lindex == -1 and pformatetc->dwAspect == DVASPECT_CONTENT)
	{
		if (pformatetc->cfFormat == CF_UNICODETEXT)
		{
			string text;
			mControl->DragSendData(text);
			
			wstring wtext(c2w(text));
			
			pmedium->tymed = TYMED_HGLOBAL;
			pmedium->pUnkForRelease = nullptr;

			uint32_t len = (wtext.length() + 1) * sizeof(wchar_t);
			pmedium->hGlobal = ::GlobalAlloc(GMEM_FIXED, len);
			if (pmedium->hGlobal != nullptr)
			{
				memcpy(pmedium->hGlobal, wtext.c_str(), len);
				result = S_OK;
			}
		}
		else if (pformatetc->cfFormat == sCFSTR_MWinCanvasImpl)
		{
			pmedium->tymed = TYMED_HGLOBAL;
			pmedium->pUnkForRelease = nullptr;

			uint32_t len = sizeof(void*);
			pmedium->hGlobal = ::GlobalAlloc(GMEM_FIXED, len);
			if (pmedium->hGlobal != nullptr)
			{
				memcpy(pmedium->hGlobal, &mControl, len);
				result = S_OK;
			}
		}
	}

	return result;	
}

HRESULT __stdcall MDropSource::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
	return DATA_E_FORMATETC;;	
}

HRESULT __stdcall MDropSource::QueryGetData(FORMATETC *pformatetc)
{
	HRESULT result = S_OK;
	if (pformatetc == nullptr or pformatetc->cfFormat != CF_UNICODETEXT)
		result = DV_E_FORMATETC;
	else if (pformatetc->tymed != TYMED_HGLOBAL)
		result = DV_E_TYMED;
	else if (pformatetc->lindex != -1)
		result = DV_E_LINDEX;
	else if (pformatetc->dwAspect != DVASPECT_CONTENT)
		result = DV_E_DVASPECT;

	return result;
}

HRESULT __stdcall MDropSource::GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut)
{
	pformatetcOut->ptd = NULL;
	return E_NOTIMPL;
}

HRESULT __stdcall MDropSource::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	return E_NOTIMPL;
}

HRESULT __stdcall MDropSource::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
	HRESULT result = E_NOTIMPL;
	if (dwDirection == DATADIR_GET)
	{
		const FORMATETC fmt[] = {
			{ CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ sCFSTR_MWinCanvasImpl, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
		};
		result = ::SHCreateStdEnumFmtEtc(2, fmt, ppenumFormatEtc);
	}

	return result;
}

unsigned long __stdcall MDropSource::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}

unsigned long __stdcall MDropSource::Release()
{
	unsigned long newCount = InterlockedDecrement(&mRefCount);

    if (newCount == 0)
    {
        delete this;
        return 0;
    }

    return newCount;
}


HRESULT __stdcall MDropSource::QueryInterface(IID const& riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(MDropSource, IDropSource),
        QITABENT(MDropSource, IDataObject),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

}

// --------------------------------------------------------------------

MWinCanvasImpl::MWinCanvasImpl(
	MCanvas*		inCanvas)
	: MCanvasImpl(inCanvas)
	, MWinProcMixin(inCanvas)
	, mLastClickTime(0)
	, mMonitor(nullptr)
{
	using namespace std::placeholders;

	AddHandler(WM_PAINT,			std::bind(&MWinCanvasImpl::WMPaint, this, _1, _2, _3, _4, _5));
	AddHandler(WM_DISPLAYCHANGE,	std::bind(&MWinCanvasImpl::WMPaint, this, _1, _2, _3, _4, _5));
	AddHandler(WM_ERASEBKGND,		std::bind(&MWinCanvasImpl::WMEraseBkgnd, this, _1, _2, _3, _4, _5));
	AddHandler(WM_SIZE,				std::bind(&MWinCanvasImpl::WMSize, this, _1, _2, _3, _4, _5));
	AddHandler(WM_WINDOWPOSCHANGED,	std::bind(&MWinCanvasImpl::WMWindowPosChanged, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONDOWN,		std::bind(&MWinCanvasImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONDBLCLK,	std::bind(&MWinCanvasImpl::WMMouseDown, this, _1, _2, _3, _4, _5));
	AddHandler(WM_LBUTTONUP,		std::bind(&MWinCanvasImpl::WMMouseUp, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSEMOVE,		std::bind(&MWinCanvasImpl::WMMouseMove, this, _1, _2, _3, _4, _5));
	AddHandler(WM_MOUSELEAVE,		std::bind(&MWinCanvasImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
	AddHandler(WM_CAPTURECHANGED,	std::bind(&MWinCanvasImpl::WMMouseExit, this, _1, _2, _3, _4, _5));
	AddHandler(WM_SETCURSOR,		std::bind(&MWinCanvasImpl::WMSetCursor, this, _1, _2, _3, _4, _5));
}

MWinCanvasImpl::~MWinCanvasImpl()
{
	if (GetHandle())
		::DestroyWindow(GetHandle());
}

ID2D1RenderTargetPtr MWinCanvasImpl::GetRenderTarget()
{
	ID2D1RenderTargetPtr result;
	mRenderTarget.QueryInterface(&result);
	return result;
}
	
void MWinCanvasImpl::MoveFrame(
	int32_t			inXDelta,
	int32_t			inYDelta)
{
	mControl->MView::MoveFrame(inXDelta, inYDelta);

	if (GetHandle() != nullptr)
	{
		MRect bounds;

		MView* view = mControl;
		MView* parent = view->GetParent();
	
		view->GetBounds(bounds);
	
		while (parent != nullptr)
		{
			view->ConvertToParent(bounds.x, bounds.y);
		
			MWindow* window = dynamic_cast<MWindow*>(parent);
			if (window != nullptr)
				break;

			MControlBase* cntrl = dynamic_cast<MControlBase*>(parent);
			if (cntrl != nullptr and dynamic_cast<MWinProcMixin*>(cntrl->GetControlImplBase()) != nullptr)
				break;

			view = parent;
			parent = parent->GetParent();
		}

		::MoveWindow(GetHandle(), bounds.x, bounds.y,
			bounds.width, bounds.height, true);
	}
}

void MWinCanvasImpl::ResizeFrame(
	int32_t			inWidthDelta,
	int32_t			inHeightDelta)
{
	mControl->MView::ResizeFrame(inWidthDelta, inHeightDelta);

	if (GetHandle() != nullptr)
	{
		MRect bounds;

		MView* view = mControl;
		MView* parent = view->GetParent();
	
		view->GetBounds(bounds);
	
		while (parent != nullptr)
		{
			view->ConvertToParent(bounds.x, bounds.y);
		
			MWindow* window = dynamic_cast<MWindow*>(parent);
			if (window != nullptr)
				break;

			MControlBase* cntrl = dynamic_cast<MControlBase*>(parent);
			if (cntrl != nullptr and dynamic_cast<MWinProcMixin*>(cntrl->GetControlImplBase()) != nullptr)
				break;

			view = parent;
			parent = parent->GetParent();
		}

		::MoveWindow(GetHandle(), bounds.x, bounds.y,
			bounds.width, bounds.height, true);
	}
}

void MWinCanvasImpl::AddedToWindow()
{
	MRect bounds;

	MView* view = mControl;
	MView* parent = view->GetParent();
	MWinProcMixin* windowImpl = nullptr;
	
	view->GetBounds(bounds);
	
	while (parent != nullptr)
	{
		view->ConvertToParent(bounds.x, bounds.y);
		
		// for now we don't support embedding of a canvas in canvas...
		MWindow* window = dynamic_cast<MWindow*>(parent);
		if (window != nullptr)
		{
			windowImpl = static_cast<MWinWindowImpl*>(window->GetImpl());
			break;
		}

		MControlBase* control = dynamic_cast<MControlBase*>(parent);
		if (control != nullptr)
		{
			MControlImplBase* impl = control->GetControlImplBase();
			if (impl != nullptr and impl->GetWinProcMixin() != nullptr)
			{
				windowImpl = impl->GetWinProcMixin();
				break;
			}
		}
		
		view = parent;
		parent = parent->GetParent();
	}

	CreateHandle(windowImpl, bounds, L"");
	SubClass();
	
	RECT r;
	::GetClientRect(GetHandle(), &r);
	if (r.right - r.left != bounds.width or
		r.bottom - r.top != bounds.height)
	{
		::MapWindowPoints(GetHandle(), windowImpl->GetHandle(), (LPPOINT)&r, 2);

		mControl->MoveFrame(
			r.left - bounds.x, r.top - bounds.y);
		mControl->ResizeFrame(
			(r.right - r.left) - bounds.width,
			(r.bottom - r.top) - bounds.height);
	}
//
//	mControl->MView::AddedToWindow();
}

void MWinCanvasImpl::CreateParams(DWORD& outStyle, DWORD& outExStyle,
	wstring& outClassName, HMENU& outMenu)
{
	MWinProcMixin::CreateParams(outStyle, outExStyle, outClassName, outMenu);
	
	outStyle = WS_CHILDWINDOW | WS_VISIBLE;
	outExStyle = 0;
	outClassName = L"MWinCanvasImpl";
}

void MWinCanvasImpl::RegisterParams(UINT& outStyle, int& outWndExtra, HCURSOR& outCursor,
	HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground)
{
	MWinProcMixin::RegisterParams(outStyle, outWndExtra, outCursor, outIcon, outSmallIcon, outBackground);
	
	outStyle = CS_HREDRAW | CS_VREDRAW;
}

bool MWinCanvasImpl::WMDestroy(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
	LPARAM inLParam, LRESULT& outResult)
{
	if (GetHandle() == inHWnd and mDropTarget)
		::RevokeDragDrop(GetHandle());

	return MWinProcMixin::WMDestroy(inHWnd, inUMsg, inWParam, inLParam, outResult);
}

void MWinCanvasImpl::AcceptDragAndDrop(bool inFiles, bool inText)
{
	mDropTarget = new MDropTarget(mControl, inFiles, inText);
	::RegisterDragDrop(GetHandle(), mDropTarget);	
}

void MWinCanvasImpl::StartDrag()
{
	ComPtr<MDropSource> source = new MDropSource(mControl);
	
	DWORD effect;
	HRESULT err = ::DoDragDrop(source, source, DROPEFFECT_COPY | DROPEFFECT_MOVE, &effect);
	//if (err == DRAGDROP_S_DROP and effect == DROPEFFECT_MOVE)
	//	mControl->DragDeleteData();
}

bool MWinCanvasImpl::WMPaint(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	/* Get the 'dirty' rect */
	RECT lUpdateRect;
	if (::GetUpdateRect(inHWnd, &lUpdateRect, FALSE) == TRUE)
	{
		MRect update{
			static_cast<int32_t>(lUpdateRect.left),
			static_cast<int32_t>(lUpdateRect.top),
			static_cast<int32_t>(lUpdateRect.right - lUpdateRect.left),
			static_cast<int32_t>(lUpdateRect.bottom - lUpdateRect.top)
		};

		//mControl->ConvertFromWindow(update.x, update.y);
		int32_t sx, sy;
		mControl->GetScrollPosition(sx, sy);
		update.x += sx;
		update.y += sy;

		PAINTSTRUCT lPs;
		HDC hdc = ::BeginPaint(inHWnd, &lPs);
		if (hdc)
		{
			RECT rc;
			::GetClientRect(GetHandle(), &rc);

			if (mRenderTarget != nullptr)
			{
				auto rSize = mRenderTarget->GetPixelSize();
				if (rSize.width != rc.right - rc.left or rSize.height != rc.bottom - rc.top)
				{
					if (mRenderTarget->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)) == D2DERR_RECREATE_TARGET)
						mRenderTarget = nullptr;
				}
			}

			if (not mRenderTarget)
			{
				D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
				D2D1_HWND_RENDER_TARGET_PROPERTIES wprops = D2D1::HwndRenderTargetProperties(
					GetHandle(), D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
				);
				THROW_IF_HRESULT_ERROR(
					MWinDeviceImpl::GetD2D1Factory()->CreateHwndRenderTarget(&props, &wprops, &mRenderTarget));
	
				mRenderTarget->SetDpi(96.f, 96.f);
			}
	
			mRenderTarget->BeginDraw();
			try
			{			
				mControl->RedrawAll(update);
			}
			catch (...)
			{
			}
	
			if (mRenderTarget->EndDraw() == D2DERR_RECREATE_TARGET)
				mRenderTarget = nullptr;

			::EndPaint(inHWnd, &lPs);
		}

		// retry in case rendering went wrong
		if (mRenderTarget == nullptr)
			::InvalidateRect(inHWnd, NULL, TRUE);
	}

	outResult = 0;
	return true; //mControl->GetChildren().empty();
}

bool MWinCanvasImpl::WMEraseBkgnd(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	outResult = 1;
	return true;
}

bool MWinCanvasImpl::WMSize(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	if (inWParam != SIZE_MINIMIZED)
	{
		MRect newBounds(0, 0, LOWORD(inLParam), HIWORD(inLParam));
		MRect oldBounds;
		mControl->GetBounds(oldBounds);

		if (mRenderTarget != nullptr)
		{
			HRESULT hr = mRenderTarget->Resize(D2D1::SizeU(newBounds.width, newBounds.height));
			if (hr == D2DERR_RECREATE_TARGET)
				mRenderTarget = nullptr;
		}
		
		mControl->ResizeFrame(newBounds.width - oldBounds.width,
			newBounds.height - oldBounds.height);
	}

	return false;
}

bool MWinCanvasImpl::WMWindowPosChanged(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
    HMONITOR monitor = MonitorFromWindow(inHWnd, MONITOR_DEFAULTTONULL);
    if (monitor != mMonitor)
    {
        mMonitor = monitor;
        if (mRenderTarget != NULL)
        {
            ComPtr<IDWriteRenderingParams> pRenderingParams;
            MWinDeviceImpl::GetDWFactory()->CreateMonitorRenderingParams(monitor, &pRenderingParams);
            mRenderTarget->SetTextRenderingParams(pRenderingParams);
        }

        ::InvalidateRect(inHWnd, NULL, TRUE);
    }

	return false;
}

void MWinCanvasImpl::MapXY(int32_t& ioX, int32_t& ioY)
{
	POINT p = { ioX, ioY };
	::MapWindowPoints(GetHandle(), static_cast<MWinWindowImpl*>(mControl->GetWindow()->GetImpl())->GetHandle(), (LPPOINT)&p, 1);
	ioX = p.x;
	ioY = p.y;
}

void MWinCanvasImpl::TrackMouse(bool inTrackMove, bool inTrackExit)
{
	TRACKMOUSEEVENT e = { sizeof(TRACKMOUSEEVENT) };
	if (inTrackMove)
		e.dwFlags |= TME_HOVER;
	if (inTrackExit)
		e.dwFlags |= TME_LEAVE;
	e.hwndTrack = GetHandle();
	e.dwHoverTime = TME_HOVER;
	::TrackMouseEvent(&e);
}

bool MWinCanvasImpl::WMMouseDown(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	::SetFocus(inHWnd);
	::SetCapture(inHWnd);

	uint32_t modifiers;
	::GetModifierState(modifiers, false);
	
	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));

	if (mLastClickTime + GetDblClickTime() > GetLocalTime())
		mClickCount = mClickCount % 3 + 1;
	else
		mClickCount = 1;

	MapXY(x, y);

	mLastClickTime = GetLocalTime();
	mControl->ConvertFromWindow(x, y);
	mControl->MouseDown(x, y, mClickCount, modifiers);

	return true;
}

bool MWinCanvasImpl::WMMouseMove(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));

	MapXY(x, y);

	uint32_t modifiers;
	::GetModifierState(modifiers, false);

	mControl->ConvertFromWindow(x, y);
	mControl->MouseMove(x, y, modifiers);

	return true;
}

bool MWinCanvasImpl::WMMouseExit(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	mControl->MouseExit();

	return true;
}

bool MWinCanvasImpl::WMMouseUp(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	::ReleaseCapture();

	int32_t x = static_cast<int16_t>(LOWORD(inLParam));
	int32_t y = static_cast<int16_t>(HIWORD(inLParam));

	MapXY(x, y);

	uint32_t modifiers;
	::GetModifierState(modifiers, false);

	mControl->ConvertFromWindow(x, y);
	mControl->MouseUp(x, y, modifiers);

	return true;
}

bool MWinCanvasImpl::WMSetCursor(HWND inHWnd, UINT inUMsg, WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)
{
	bool handled = false;
	try
	{
		if (mControl->IsActive())
		{
			int32_t x, y;
			uint32_t modifiers = 0;

			POINT p;
			::GetCursorPos(&p);
			::ScreenToClient(GetHandle(), &p);
			
			x = p.x;
			y = p.y;

			MapXY(x, y);

			mControl->ConvertFromWindow(x, y);
			mControl->AdjustCursor(x, y, modifiers);
			handled = true;
		}
	}
	catch (...)
	{
	}

	return handled;
}

void MWinCanvasImpl::SetFocus()
{
	::SetFocus(GetHandle());
}

void MWinCanvasImpl::ReleaseFocus()
{
	if (::GetFocus() == GetHandle())
		::SetFocus(nullptr);
}

bool MWinCanvasImpl::IsFocus() const
{
	return GetHandle() == ::GetFocus();
}

// --------------------------------------------------------------------

MCanvasImpl* MCanvasImpl::Create(MCanvas* inCanvas, uint32_t inWidth, uint32_t inHeight)
{
	return new MWinCanvasImpl(inCanvas);
}
