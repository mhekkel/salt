//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include <UIAnimation.h>
#include <stdexcept>

#include "comptr.hpp"

#include "MAnimation.hpp"
#include "MAnimationImpl.hpp"

using namespace std;

typedef ComPtr<IUIAnimationManager>		IUIAnimationManagerPtr;
typedef ComPtr<IUIAnimationTimer>		IUIAnimationTimerPtr;
typedef ComPtr<IUIAnimationVariable>	IUIAnimationVariablePtr;
typedef ComPtr<IUIAnimationTransitionLibrary>
										IUIAnimationTransitionLibraryPtr;
typedef ComPtr<IUIAnimationStoryboard>	IUIAnimationStoryboardPtr;
typedef ComPtr<IUIAnimationManagerEventHandler>
										IUIAnimationManagerEventHandlerPtr;
typedef ComPtr<IUIAnimationTransition>	IUIAnimationTransitionPtr;
typedef ComPtr<IUIAnimationTimerUpdateHandler>
										IUIAnimationTimerUpdateHandlerPtr;

// --------------------------------------------------------------------

class MWinAnimationManagerImpl : public MAnimationManagerImpl
{
  public:
									MWinAnimationManagerImpl(MAnimationManager* inManager);
									~MWinAnimationManagerImpl();

	virtual bool					Update();
	virtual MAnimationVariable*		CreateVariable(double inValue, double inMin, double inMax);
	virtual MStoryboard*			CreateStoryboard();
	virtual void					Schedule(MStoryboard* inStoryboard);

  protected:
	IUIAnimationManagerPtr			mAnimationManager;
	IUIAnimationTimerPtr			mTimer;
	IUIAnimationTransitionLibraryPtr
									mTransitionLibrary;
};

// --------------------------------------------------------------------

class MWinAnimationVariableImpl : public MAnimationVariableImpl
{
  public:
									MWinAnimationVariableImpl(IUIAnimationVariablePtr inVariable)
										: mVariable(inVariable) {}

	virtual double					GetValue() const;
	IUIAnimationVariablePtr			GetVariable()				{ return mVariable; }

  protected:

	IUIAnimationVariablePtr			mVariable;					
};

// --------------------------------------------------------------------

class MWinStoryboardImpl : public MStoryboardImpl
{
  public:
									MWinStoryboardImpl(IUIAnimationManagerPtr inAnimationManager,
											IUIAnimationTransitionLibraryPtr inTransitionLibrary,
											IUIAnimationStoryboardPtr inStoryboard)
										: mAnimationManager(inAnimationManager)
										, mTransitionLibrary(inTransitionLibrary)
										, mStoryboard(inStoryboard) {}

	virtual void					AddTransition(MAnimationVariable* inVariable,
										double inNewValue, double inDuration,
										const char* inTransitionName);

	void							Schedule(UI_ANIMATION_SECONDS inSeconds);

  protected:
	IUIAnimationManagerPtr			mAnimationManager;
	IUIAnimationTransitionLibraryPtr
									mTransitionLibrary;
	IUIAnimationStoryboardPtr		mStoryboard;
};

// --------------------------------------------------------------------

class MWinAnimationManagerCallback : public IUIAnimationManagerEventHandler
{
  public:
						MWinAnimationManagerCallback(MAnimationManager* inManager)
							: mManager(inManager), mRefCount(0) {}
	virtual				~MWinAnimationManagerCallback() {}

	ULONG __stdcall		AddRef();
	ULONG __stdcall		Release();
	HRESULT __stdcall	QueryInterface(IID const& riid, void** ppvObject);

    // IUIAnimationManagerEventHandler
    HRESULT __stdcall	OnManagerStatusChanged(
    						UI_ANIMATION_MANAGER_STATUS newStatus,
    						UI_ANIMATION_MANAGER_STATUS previousStatus);

  private:
	MAnimationManager*	mManager;
	unsigned long		mRefCount;
};

HRESULT __stdcall MWinAnimationManagerCallback::OnManagerStatusChanged(
	UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus)
{
    HRESULT hr = S_OK;

    if (newStatus == UI_ANIMATION_MANAGER_BUSY)
    {
    	try
    	{
			mManager->eAnimate();
    	}
    	catch (...)
    	{
    		hr = E_FAIL;
    	}
    }

    return hr;
}

unsigned long __stdcall MWinAnimationManagerCallback::AddRef()
{
    return ::InterlockedIncrement(&mRefCount);
}

unsigned long __stdcall MWinAnimationManagerCallback::Release()
{
	unsigned long newCount = ::InterlockedDecrement(&mRefCount);

    if (newCount == 0)
    {
        delete this;
        return 0;
    }

    return newCount;
}

HRESULT __stdcall MWinAnimationManagerCallback::QueryInterface(IID const& riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(MWinAnimationManagerCallback, IUIAnimationManagerEventHandler),
        { 0 },
    };
    return ::QISearch(this, qit, riid, ppv);
}

// --------------------------------------------------------------------

class MWinAnimationTimerEventHandler : public IUIAnimationTimerEventHandler
{
  public:
						MWinAnimationTimerEventHandler(MAnimationManager* inManager)
							: mManager(inManager), mRefCount(0) {}
	virtual				~MWinAnimationTimerEventHandler() {}

	ULONG __stdcall		AddRef();
	ULONG __stdcall		Release();
	HRESULT __stdcall	QueryInterface(IID const& riid, void** ppvObject);

    // IUIAnimationTimerEventHandler
    HRESULT __stdcall	OnPreUpdate()				{ return S_OK; }
    HRESULT __stdcall	OnPostUpdate();
    HRESULT __stdcall	OnRenderingTooSlow(uint32_t)	{ return S_OK; }

  private:
	MAnimationManager*	mManager;
	unsigned long		mRefCount;
};

HRESULT __stdcall MWinAnimationTimerEventHandler::OnPostUpdate()
{
    HRESULT hr = S_OK;

	try
	{
		mManager->eAnimate();
	}
	catch (...)
	{
		hr = E_FAIL;
	}

    return hr;
}

unsigned long __stdcall MWinAnimationTimerEventHandler::AddRef()
{
    return ::InterlockedIncrement(&mRefCount);
}

unsigned long __stdcall MWinAnimationTimerEventHandler::Release()
{
	unsigned long newCount = ::InterlockedDecrement(&mRefCount);

    if (newCount == 0)
    {
        delete this;
        return 0;
    }

    return newCount;
}

HRESULT __stdcall MWinAnimationTimerEventHandler::QueryInterface(IID const& riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(MWinAnimationTimerEventHandler, IUIAnimationTimerEventHandler),
        { 0 },
    };
    return ::QISearch(this, qit, riid, ppv);
}

// --------------------------------------------------------------------

MWinAnimationManagerImpl::MWinAnimationManagerImpl(MAnimationManager* inManager)
{
//	AppDriven does not work in salt...
//	if (::CoCreateInstance(CLSID_UIAnimationManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mAnimationManager)) == S_OK and
//		::CoCreateInstance(CLSID_UIAnimationTimer, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mTimer)) == S_OK and
//		::CoCreateInstance(CLSID_UIAnimationTransitionLibrary, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mTransitionLibrary)) == S_OK)
//	{
//		mAnimationManager->SetManagerEventHandler(new MWinAnimationManagerCallback(inManager));
//	}

	IUIAnimationTimerUpdateHandlerPtr timerUpdateHandler; 

	if (::CoCreateInstance(CLSID_UIAnimationManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mAnimationManager)) == S_OK and
		::CoCreateInstance(CLSID_UIAnimationTimer, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mTimer)) == S_OK and
		::CoCreateInstance(CLSID_UIAnimationTransitionLibrary, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mTransitionLibrary)) == S_OK and
		mAnimationManager->QueryInterface(&timerUpdateHandler) == S_OK and
		mTimer->SetTimerUpdateHandler(timerUpdateHandler, UI_ANIMATION_IDLE_BEHAVIOR_DISABLE) == S_OK)
	{
		mTimer->SetTimerEventHandler(new MWinAnimationTimerEventHandler(inManager));
	}
	else
		throw std::runtime_error("Animation Manager not supported");
}

MWinAnimationManagerImpl::~MWinAnimationManagerImpl()
{
	if (mAnimationManager)
		mAnimationManager->SetManagerEventHandler(nullptr);
}

bool MWinAnimationManagerImpl::Update()
{
	bool result = false;
	UI_ANIMATION_MANAGER_STATUS status;
	
	UI_ANIMATION_SECONDS secondsNow;
	if (mTimer->GetTime(&secondsNow) == S_OK and
		mAnimationManager->Update(secondsNow) == S_OK and
		mAnimationManager->GetStatus(&status) == S_OK)
	{
		result = status == UI_ANIMATION_MANAGER_BUSY;
	}
	
	return result;
}

MAnimationVariable* MWinAnimationManagerImpl::CreateVariable(double inValue, double inMin, double inMax)
{
	MAnimationVariable* result = nullptr;
	
	IUIAnimationVariablePtr variable;
	if (mAnimationManager->CreateAnimationVariable(inValue, &variable) == S_OK and
		variable->SetLowerBound(inMin) == S_OK and variable->SetUpperBound(inMax) == S_OK)
	{
		result = new MAnimationVariable(new MWinAnimationVariableImpl(variable));
	}

	return result;
}

MStoryboard* MWinAnimationManagerImpl::CreateStoryboard()
{
	MStoryboard* result = nullptr;
	
	IUIAnimationStoryboardPtr storyboard;
	if (mAnimationManager->CreateStoryboard(&storyboard) == S_OK)
		result = new MStoryboard(new MWinStoryboardImpl(mAnimationManager, mTransitionLibrary, storyboard));

	return result;
}

void MWinAnimationManagerImpl::Schedule(MStoryboard* inStoryboard)
{
	UI_ANIMATION_SECONDS secondsNow;
	
	if (mTimer->GetTime(&secondsNow) == S_OK)
		static_cast<MWinStoryboardImpl*>(inStoryboard->GetImpl())->Schedule(secondsNow);
}

// --------------------------------------------------------------------

double MWinAnimationVariableImpl::GetValue() const
{
	double value = 0;
	if (mVariable)
		(void)mVariable->GetValue(&value);
	return static_cast<double>(value);
}

// --------------------------------------------------------------------

void MWinStoryboardImpl::AddTransition(MAnimationVariable* inVariable,
	double inNewValue, double inDuration, const char* inTransitionName)
{
	IUIAnimationTransitionPtr transition;
	
	if (inTransitionName == "acceleration-decelleration")
	{
		double kAccelerationRatio = 0.5, kDecellerationRatio = 0.5;
		(void)mTransitionLibrary->CreateAccelerateDecelerateTransition(
			inDuration, inNewValue, kAccelerationRatio, kDecellerationRatio, &transition);
	}
	else if (inTransitionName == "reversal")
		(void)mTransitionLibrary->CreateReversalTransition(inDuration, &transition);
	
	if (transition)
	{
		MWinAnimationVariableImpl* varImpl = static_cast<MWinAnimationVariableImpl*>(inVariable->GetImpl());
		mStoryboard->AddTransition(varImpl->GetVariable(), transition);
	}
}

void MWinStoryboardImpl::Schedule(UI_ANIMATION_SECONDS inSeconds)
{
	UI_ANIMATION_SCHEDULING_RESULT schedulingResult;

	mStoryboard->Schedule(inSeconds, &schedulingResult);
}

// --------------------------------------------------------------------

MAnimationManagerImpl* MAnimationManagerImpl::Create(MAnimationManager* inManager)
{
	try
	{
		return new MWinAnimationManagerImpl(inManager);
	}
	catch (...)
	{
		return nullptr;
	}
}
