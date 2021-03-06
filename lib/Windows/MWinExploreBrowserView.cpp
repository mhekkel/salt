//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "comptr.hpp"

#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <propkey.h>

#include "MWinUtils.hpp"

#include "MWinWindowImpl.hpp"
#include "MWinExploreBrowserView.hpp"

typedef ComPtr<IExplorerBrowser>	IExplorerBrowserPtr;
typedef ComPtr<IResultsFolder>		IResultsFolderPtr;
typedef ComPtr<IShellItem>			IShellItemPtr;
//typedef ComPtr<IDataAdviseHolder>	IDataAdviseHolderPtr;

using namespace std;

// --------------------------------------------------------------------

class MExploreBrowserData : public IDataObject
{
  public:
						MExploreBrowserData();
	virtual				~MExploreBrowserData();
	
	// IUnknown
	HRESULT __stdcall	QueryInterface(REFIID riid, void **ppv);
	ULONG __stdcall		AddRef();
	ULONG __stdcall		Release();
	
	// IDataObject
	HRESULT __stdcall	GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
	HRESULT __stdcall	GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
	HRESULT __stdcall	QueryGetData(FORMATETC *pformatetc);
	HRESULT __stdcall	GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut);
	HRESULT __stdcall	SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
	HRESULT __stdcall	EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
	HRESULT __stdcall	DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
	HRESULT __stdcall	DUnadvise(DWORD dwConnection);
	HRESULT __stdcall	EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

  private:
	uint32_t					mRefCount;
	IDataAdviseHolderPtr	mAdvise;
};

class MExploreBrowserImpl : public IServiceProvider,
							public ICommDlgBrowser
{
  public:
						MExploreBrowserImpl();
						~MExploreBrowserImpl();
	
	void				Initialize(HWND inWindow, MRect inBounds);
	void				Close();
	void				SetBounds(MRect inBounds);

	void				AddItem(IShellItemPtr inItem);
	
	// IUnknown
	HRESULT __stdcall	QueryInterface(REFIID riid, void **ppv);
	ULONG __stdcall		AddRef();
	ULONG __stdcall		Release();
	
	// IServiceProvider
	HRESULT __stdcall	QueryService(REFGUID guidService, REFIID riid, void **ppv);
	
	// ICommDlgBrowser
	HRESULT __stdcall	OnDefaultCommand(IShellView * /* psv */);
	HRESULT __stdcall	OnStateChange(IShellView * /* psv */, ULONG uChange);
	HRESULT __stdcall	IncludeObject(IShellView * /* psv */, PCUITEMID_CHILD /* pidl */);

  private:
	uint32_t				mRefCount;
	IExplorerBrowserPtr	mBrowser;
	IResultsFolderPtr	mFolder;
};

MExploreBrowserImpl::MExploreBrowserImpl()
	: mRefCount(1)
{
}

MExploreBrowserImpl::~MExploreBrowserImpl()
{
}

void MExploreBrowserImpl::Initialize(HWND inWindow, MRect inBounds)
{
	for (;;)
	{
		if (FAILED(::CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&mBrowser))))
			break;
		
		IUnknown_SetSite(mBrowser, static_cast<IServiceProvider*>(this));
		
        FOLDERSETTINGS fs = {0};
        fs.ViewMode = FVM_DETAILS;
        fs.fFlags = FWF_AUTOARRANGE | FWF_NOWEBVIEW;
        
        inBounds.InsetBy(-2, -2);
        RECT rc = { inBounds.x, inBounds.y, inBounds.x + inBounds.width, inBounds.y + inBounds.height };
        
        if (FAILED(mBrowser->Initialize(inWindow, &rc, &fs)))
        	break;
		
		mBrowser->SetOptions(EBO_NONE);	// aanpassen
		
        // Initialize the exporer browser so that we can use the results folder
        // as the data source. This enables us to program the contents of
        // the view via IResultsFolder

		if (FAILED(mBrowser->FillFromObject(nullptr, EBF_SELECTFROMDATAOBJECT)))
			break;
		
		ComPtr<IFolderView2> folderView;
		if (FAILED(mBrowser->GetCurrentView(IID_PPV_ARGS(&folderView))))
			break;
		
		ComPtr<IColumnManager> columnManager;
		if (FAILED(folderView->QueryInterface(&columnManager)))
			break;
		
        PROPERTYKEY rgkeys[] = { PKEY_ItemNameDisplay, PKEY_FileOwner, PKEY_FileAllocationSize, PKEY_DateModified };
        if (FAILED(columnManager->SetColumns(rgkeys, ARRAYSIZE(rgkeys))))
        	break;

//        CM_COLUMNINFO ci = {sizeof(ci), CM_MASK_WIDTH | CM_MASK_DEFAULTWIDTH | CM_MASK_IDEALWIDTH};
//        if (SUCCEEDED(columnManager->GetColumnInfo(PKEY_ItemFolderPathDisplay, &ci)))
//        {
//            ci.uWidth += 100;
//            ci.uDefaultWidth += 100;
//            ci.uIdealWidth += 100;
//            columnManager->SetColumnInfo(PKEY_ItemFolderPathDisplay, &ci);
//        }
        
        folderView->GetFolder(IID_PPV_ARGS(&mFolder));
		
		// done
        break;
    }
}

void MExploreBrowserImpl::Close()
{
	IUnknown_SetSite(mBrowser, NULL);
}

void MExploreBrowserImpl::SetBounds(MRect inBounds)
{
    inBounds.InsetBy(-2, -2);
    RECT rc = { inBounds.x, inBounds.y, inBounds.x + inBounds.width, inBounds.y + inBounds.height };
	mBrowser->SetRect(nullptr, rc);
}

void MExploreBrowserImpl::AddItem(IShellItemPtr item)
{
	mFolder->AddItem(item);
}

// IUnknown
HRESULT __stdcall MExploreBrowserImpl::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(MExploreBrowserImpl, IServiceProvider),
        QITABENT(MExploreBrowserImpl, ICommDlgBrowser),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG __stdcall MExploreBrowserImpl::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}

ULONG __stdcall MExploreBrowserImpl::Release()
{
    long cRef = InterlockedDecrement(&mRefCount);
    if (!cRef)
        delete this;
    return cRef;
}

// IServiceProvider
HRESULT __stdcall MExploreBrowserImpl::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;
    if (guidService == SID_SExplorerBrowserFrame)
    {
        // responding to this SID allows us to hook up our ICommDlgBrowser
        // implementation so we get selection change events from the view
        hr = QueryInterface(riid, ppv);
    }
    return hr;
}

// ICommDlgBrowser
HRESULT __stdcall MExploreBrowserImpl::OnDefaultCommand(IShellView * /* psv */)
{
//					        _OnExplore();
    return S_OK;
}

HRESULT __stdcall MExploreBrowserImpl::OnStateChange(IShellView * /* psv */, ULONG uChange)
{
//					        if (uChange == CDBOSC_SELCHANGE)
//					        {
//					            PostMessage(_hdlg, KFD_SELCHANGE, 0, 0);
//					        }
    return S_OK;
}

HRESULT __stdcall MExploreBrowserImpl::IncludeObject(IShellView * /* psv */, PCUITEMID_CHILD /* pidl */)
{
    return S_OK;
}

// --------------------------------------------------------------------

class MSFTPItem : public IShellItem
{
  public:
						MSFTPItem(const string& inName, const string& inOwner,
							int64_t inFileSize, time_t inModificationTime);
						~MSFTPItem();

	// IUnknown
	HRESULT __stdcall	QueryInterface(REFIID riid, void **ppv);
	ULONG __stdcall		AddRef();
	ULONG __stdcall		Release();
	
	// IShellItem
	HRESULT __stdcall	BindToHandler(IBindCtx *pbc, REFGUID bhid, REFIID riid, void **ppv);
	HRESULT __stdcall	GetParent(IShellItem **ppsi);
	HRESULT __stdcall	GetDisplayName(SIGDN sigdnName, LPWSTR *ppszName);
	HRESULT __stdcall	GetAttributes(SFGAOF sfgaoMask, SFGAOF *psfgaoAttribs);
	HRESULT __stdcall	Compare(IShellItem *psi, SICHINTF hint, int *piOrder);

//	// IShellItem2
//	HRESULT __stdcall	GetPropertyStore(GETPROPERTYSTOREFLAGS flags, REFIID riid, void **ppv);
//	HRESULT __stdcall	GetPropertyStoreWithCreateObject(GETPROPERTYSTOREFLAGS flags, IUnknown *punkCreateObject,
//							REFIID riid, void **ppv);
//	HRESULT __stdcall	GetPropertyStoreForKeys(const PROPERTYKEY *rgKeys, UINT cKeys, GETPROPERTYSTOREFLAGS flags,
//							REFIID riid, void **ppv);
//	HRESULT __stdcall	GetPropertyDescriptionList(REFPROPERTYKEY keyType, REFIID riid, void **ppv);
//	HRESULT __stdcall	Update(IBindCtx *pbc);
//	HRESULT __stdcall	GetProperty(REFPROPERTYKEY key, PROPVARIANT *ppropvar);
//	HRESULT __stdcall	GetCLSID(REFPROPERTYKEY key, CLSID *pclsid);
//	HRESULT __stdcall	GetFileTime( REFPROPERTYKEY key, FILETIME *pft);
//	HRESULT __stdcall	GetInt32_t(REFPROPERTYKEY key, int *pi);
//	HRESULT __stdcall	GetString(REFPROPERTYKEY key, LPWSTR *ppsz);
//	HRESULT __stdcall	GetUInt32_t(REFPROPERTYKEY key, ULONG *pui);
//	HRESULT __stdcall	GetUInt64_t(REFPROPERTYKEY key, ULONGLONG *pull);
//	HRESULT __stdcall	GetBool(REFPROPERTYKEY key, BOOL *pf);

  private:
	uint32_t				mRefCount;
	string				mName, mOwner;
	int64_t				mSize;
	time_t				mModificationTime;
};

MSFTPItem::MSFTPItem(const string& inName, const string& inOwner,
	int64_t inFileSize, time_t inModificationTime)
	: mRefCount(0), mName(inName), mOwner(inOwner), mSize(inFileSize), mModificationTime(inModificationTime)
{
}

MSFTPItem::~MSFTPItem()
{
}

// IUnknown
HRESULT __stdcall MSFTPItem::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(MSFTPItem, IShellItem),
//        QITABENT(MSFTPItem, IShellItem2),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG __stdcall MSFTPItem::AddRef()
{
    return InterlockedIncrement(&mRefCount);
}

ULONG __stdcall MSFTPItem::Release()
{
    long cRef = InterlockedDecrement(&mRefCount);
    if (!cRef)
        delete this;
    return cRef;
}

// IShellItem

HRESULT __stdcall MSFTPItem::BindToHandler( 
        /* [unique][in] */ __RPC__in_opt IBindCtx *pbc,
        /* [in] */ __RPC__in REFGUID bhid,
        /* [in] */ __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out_opt void **ppv)
{
	return S_FALSE;
}
    
HRESULT __stdcall MSFTPItem::GetParent( 
        /* [out] */ __RPC__deref_out_opt IShellItem **ppsi)
{
	return S_FALSE;
}
    
HRESULT __stdcall MSFTPItem::GetDisplayName( 
        /* [in] */ SIGDN sigdnName,
        /* [string][out] */ __RPC__deref_out_opt_string LPWSTR *ppszName)
{
	wstring name(c2w(mName));
	*ppszName = (LPWSTR)::CoTaskMemAlloc((name.length() + 1) * sizeof(wchar_t));
	if (*ppszName != nullptr)
		copy(name.begin(), name.end(), *ppszName);
	
	return S_OK;
}
    
HRESULT __stdcall MSFTPItem::GetAttributes( 
        /* [in] */ SFGAOF sfgaoMask,
        /* [out] */ __RPC__out SFGAOF *psfgaoAttribs)
{
	if (sfgaoMask & SFGAO_CANCOPY)			*psfgaoAttribs |= SFGAO_CANCOPY;
	if (sfgaoMask & SFGAO_CANMOVE)			*psfgaoAttribs |= SFGAO_CANMOVE;
	if (sfgaoMask & SFGAO_CANLINK)			*psfgaoAttribs &= ~SFGAO_CANLINK;
	if (sfgaoMask & SFGAO_STORAGE)			*psfgaoAttribs &= ~SFGAO_STORAGE;
	if (sfgaoMask & SFGAO_CANRENAME)		*psfgaoAttribs |= SFGAO_CANRENAME;
	if (sfgaoMask & SFGAO_CANDELETE)		*psfgaoAttribs |= SFGAO_CANDELETE;
	if (sfgaoMask & SFGAO_HASPROPSHEET)		*psfgaoAttribs &= ~SFGAO_HASPROPSHEET;
	if (sfgaoMask & SFGAO_DROPTARGET)		*psfgaoAttribs &= ~SFGAO_DROPTARGET;
	if (sfgaoMask & SFGAO_SYSTEM)			*psfgaoAttribs &= ~SFGAO_SYSTEM;
	if (sfgaoMask & SFGAO_ENCRYPTED)		*psfgaoAttribs &= ~SFGAO_ENCRYPTED;
	if (sfgaoMask & SFGAO_ISSLOW)			*psfgaoAttribs |= SFGAO_ISSLOW;
	if (sfgaoMask & SFGAO_GHOSTED)			*psfgaoAttribs &= ~SFGAO_GHOSTED;
	if (sfgaoMask & SFGAO_LINK)				*psfgaoAttribs &= ~SFGAO_LINK;
	if (sfgaoMask & SFGAO_SHARE)			*psfgaoAttribs &= ~SFGAO_SHARE;
	if (sfgaoMask & SFGAO_READONLY)			*psfgaoAttribs |= SFGAO_READONLY;
	if (sfgaoMask & SFGAO_HIDDEN)			*psfgaoAttribs |= SFGAO_HIDDEN;
	if (sfgaoMask & SFGAO_DISPLAYATTRMASK)	*psfgaoAttribs |= SFGAO_DISPLAYATTRMASK;
	if (sfgaoMask & SFGAO_NONENUMERATED)	*psfgaoAttribs |= SFGAO_NONENUMERATED;
	if (sfgaoMask & SFGAO_NEWCONTENT)		*psfgaoAttribs |= SFGAO_NEWCONTENT;
	if (sfgaoMask & SFGAO_STREAM)			*psfgaoAttribs |= SFGAO_STREAM;
	if (sfgaoMask & SFGAO_STORAGEANCESTOR)	*psfgaoAttribs |= SFGAO_STORAGEANCESTOR;
//	if (sfgaoMask & SFGAO_VALIDATE)			*psfgaoAttribs |= SFGAO_VALIDATE;
	if (sfgaoMask & SFGAO_REMOVABLE)		*psfgaoAttribs |= SFGAO_REMOVABLE;
	if (sfgaoMask & SFGAO_COMPRESSED)		*psfgaoAttribs &= ~SFGAO_COMPRESSED;
	if (sfgaoMask & SFGAO_BROWSABLE)		*psfgaoAttribs &= ~SFGAO_BROWSABLE;
	if (sfgaoMask & SFGAO_FILESYSANCESTOR)	*psfgaoAttribs &= ~SFGAO_FILESYSANCESTOR;
	if (sfgaoMask & SFGAO_FOLDER)			*psfgaoAttribs |= SFGAO_FOLDER;
	if (sfgaoMask & SFGAO_FILESYSTEM)		*psfgaoAttribs &= ~SFGAO_FILESYSTEM;
//	if (sfgaoMask & SFGAO_STORAGECAPMASK)	*psfgaoAttribs |= SFGAO_STORAGECAPMASK;
	if (sfgaoMask & SFGAO_HASSUBFOLDER)		*psfgaoAttribs &= ~SFGAO_HASSUBFOLDER;
//	if (sfgaoMask & SFGAO_CONTENTSMASK)		*psfgaoAttribs |= SFGAO_CONTENTSMASK;
//	if (sfgaoMask & SFGAO_PKEYSFGAOMASK)	*psfgaoAttribs |= SFGAO_PKEYSFGAOMASK; 

	return S_OK;
}
    
HRESULT __stdcall MSFTPItem::Compare( 
        /* [in] */ __RPC__in_opt IShellItem *psi,
        /* [in] */ SICHINTF hint,
        /* [out] */ __RPC__out int *piOrder)
{
	return S_FALSE;
}

//// IShellItem2
//
//HRESULT __stdcall MSFTPItem::GetPropertyStore( 
//        /* [in] */ GETPROPERTYSTOREFLAGS flags,
//        /* [in] */ __RPC__in REFIID riid,
//        /* [iid_is][out] */ __RPC__deref_out_opt void **ppv)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::GetPropertyStoreWithCreateObject( 
//        /* [in] */ GETPROPERTYSTOREFLAGS flags,
//        /* [in] */ __RPC__in_opt IUnknown *punkCreateObject,
//        /* [in] */ __RPC__in REFIID riid,
//        /* [iid_is][out] */ __RPC__deref_out_opt void **ppv)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::GetPropertyStoreForKeys( 
//        /* [size_is][in] */ __RPC__in_ecount_full(cKeys) const PROPERTYKEY *rgKeys,
//        /* [in] */ UINT cKeys,
//        /* [in] */ GETPROPERTYSTOREFLAGS flags,
//        /* [in] */ __RPC__in REFIID riid,
//        /* [iid_is][out] */ __RPC__deref_out_opt void **ppv)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::GetPropertyDescriptionList( 
//        /* [in] */ __RPC__in REFPROPERTYKEY keyType,
//        /* [in] */ __RPC__in REFIID riid,
//        /* [iid_is][out] */ __RPC__deref_out_opt void **ppv)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::Update( 
//        /* [unique][in] */ __RPC__in_opt IBindCtx *pbc)
//{
//	return S_OK;
//}
//    
//HRESULT __stdcall MSFTPItem::GetProperty( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [out] */ __RPC__out PROPVARIANT *ppropvar)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::GetCLSID( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [out] */ __RPC__out CLSID *pclsid)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::GetFileTime( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [out] */ __RPC__out FILETIME *pft)
//{
//	HRESULT result = S_FALSE;
//	
//	if (key == PKEY_DateModified)
//	{
//		// Note that LONGLONG is a 64-bit value
//	    LONGLONG ll;
//
//	    ll = Int32_tx32To64(mModificationTime, 10000000) + 116444736000000000;
//	    pft->dwLowDateTime = (DWORD)ll;
//	    pft->dwHighDateTime = ll >> 32;
//		
//		result = S_OK;
//	}
//	
//	return result;
//}
//    
//HRESULT __stdcall MSFTPItem::GetInt32_t( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [out] */ __RPC__out int *pi)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::GetString( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [string][out] */ __RPC__deref_out_opt_string LPWSTR *ppsz)
//{
//	HRESULT result = S_FALSE;
//	
//	if (key == PKEY_FileOwner)
//	{
//		wstring name(c2w(mOwner));
//		*ppsz = (LPWSTR)::CoTaskMemAlloc((name.length() + 1) * sizeof(wchar_t));
//		if (*ppsz != nullptr)
//			copy(name.begin(), name.end(), *ppsz);
//		
//		result = S_OK;
//	}
//	
//	return result;
//}
//    
//HRESULT __stdcall MSFTPItem::GetUInt32_t( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [out] */ __RPC__out ULONG *pui)
//{
//	return S_FALSE;
//}
//    
//HRESULT __stdcall MSFTPItem::GetUInt64_t( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [out] */ __RPC__out ULONGLONG *pull)
//{
//	HRESULT result = S_FALSE;
//	
//	if (key == PKEY_FileAllocationSize)
//	{
//		*pull = mSize;
//		result = S_OK;
//	}
//	
//	return result;
//}
//    
//HRESULT __stdcall MSFTPItem::GetBool( 
//        /* [in] */ __RPC__in REFPROPERTYKEY key,
//        /* [out] */ __RPC__out BOOL *pf)
//{
//	return S_FALSE;
//}

// --------------------------------------------------------------------

MWinExploreBrowserView::MWinExploreBrowserView(const string& inID, MRect inBounds)
	: MExploreBrowserView(inID, inBounds)
	, mImpl(nullptr)
{
}

MWinExploreBrowserView::~MWinExploreBrowserView()
{
	if (mImpl != nullptr)
	{
		mImpl->Close();
		mImpl->Release();
	}
}

void MWinExploreBrowserView::AddedToWindow()
{
	MRect bounds;
	GetBounds(bounds);
	
	MView* parent = GetParent();
	HWND windowHandle;
	
	while (parent != nullptr)
	{
		ConvertToParent(bounds.x, bounds.y);
		
		MWindow* window = dynamic_cast<MWindow*>(parent);
		if (window != nullptr)
		{
			windowHandle = static_cast<MWinWindowImpl*>(window->GetImpl())->GetHandle();
			break;
		}
		
		parent = parent->GetParent();
	}

	mImpl = new MExploreBrowserImpl();
	mImpl->Initialize(windowHandle, bounds);

	//// dummy data
	//ComPtr<IKnownFolderManager> manager;
 //   HRESULT hr = ::CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&manager));
 //   if (SUCCEEDED(hr))
 //   {
 //       UINT cCount;
 //       KNOWNFOLDERID *pkfid;

 //       hr = manager->GetFolderIds(&pkfid, &cCount);
 //       if (SUCCEEDED(hr))
 //       {
 //           for (UINT i = 0; i < cCount; i++)
 //           {
 //               ComPtr<IKnownFolder> knownFolder;
 //               hr = manager->GetFolder(pkfid[i], &knownFolder);
 //               if (SUCCEEDED(hr))
 //               {
 //                   ComPtr<IShellItem2> si;
 //                   hr = knownFolder->GetShellItem(0, IID_PPV_ARGS(&si));
 //                   if (SUCCEEDED(hr))
 //                       mImpl->AddItem(si);
 //               }
 //           }
 //           CoTaskMemFree(pkfid);
 //       }
 //   }
}

void MWinExploreBrowserView::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	MView::ResizeFrame(inWidthDelta, inHeightDelta);

	MRect bounds;
	GetBounds(bounds);
	ConvertToWindow(bounds.x, bounds.y);
	
	mImpl->SetBounds(bounds);
}

void MWinExploreBrowserView::AddItem(const string& inName, const string& inOwner,
	int64_t inFileSize, time_t inModificationTime)
{
//	mImpl->AddItem(inName, inOwner, inFileSize, inModificationTime);
	IShellItemPtr item(new MSFTPItem(inName, inOwner, inFileSize, inModificationTime));
	mImpl->AddItem(item);
}

MExploreBrowserView* MExploreBrowserView::Create(const std::string& inID, MRect inBounds)
{
	return new MWinExploreBrowserView(inID, inBounds);
}
