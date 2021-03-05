//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MXcbLib.hpp"

#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "MAnimation.hpp"
#include "MAnimationImpl.hpp"
#include "MApplication.hpp"
#include "MUtils.hpp"
#include "MError.hpp"

using namespace std;

// --------------------------------------------------------------------

class MXcbAnimationVariableImpl : public MAnimationVariableImpl
{
  public:
	MXcbAnimationVariableImpl(double inValue, double inMin, double inMax)
		: mValue(inValue), mMin(inMin), mMax(inMax) {}

	virtual void	SetValue(double inValue)
	{
		if (inValue > mMax)
			inValue = mMax;
		if (inValue < mMin)
			inValue = mMin;
		mValue = inValue;
	}

	virtual double	GetValue() const			{ return mValue; }

  private:
	double mValue, mMin, mMax;
};

// --------------------------------------------------------------------

class MXcbStoryboardImpl : public MStoryboardImpl
{
  public:
	MXcbStoryboardImpl() {}
	virtual void AddTransition(MAnimationVariable* inVariable,
						double inNewValue, double inDuration,
						const char* inTransitionName);

	// inTime is relative to the start of the story
	bool Update(double inTime);
	bool Done(double inTime);

	struct MTransition
	{
		double	mNewValue;
		double	mDuration;
		string	mTransitionName;
	};
	
	struct MVariableStory
	{
		MAnimationVariable*	mVariable;
		double				mStartValue;
		list<MTransition>	mTransistions;
	};
	
	list<MVariableStory> mVariableStories;
};

void MXcbStoryboardImpl::AddTransition(MAnimationVariable* inVariable,
	double inNewValue, double inDuration, const char* inTransitionName)
{
	auto s = find_if(mVariableStories.begin(), mVariableStories.end(),
		[inVariable](MVariableStory& st) -> bool { return st.mVariable == inVariable; });

	if (s == mVariableStories.end())
	{
		MVariableStory st = { inVariable, inVariable->GetValue() };
		mVariableStories.push_back(st);
		s = prev(mVariableStories.end());
	}
	
	assert(s != mVariableStories.end());
	
	MTransition t = { inNewValue, inDuration, inTransitionName };
	s->mTransistions.push_back(t);
}

bool MXcbStoryboardImpl::Update(double inTime)
{
	bool result = false;
	
	for (auto vs: mVariableStories)
	{
		double v = vs.mStartValue;
		double time = inTime;
		
		for (auto t: vs.mTransistions)
		{
			if (t.mDuration < time)
			{
				v = t.mNewValue;
				time -= t.mDuration;
				continue;
			}
			
			// alleen nog maar lineair
			assert(t.mTransitionName == "acceleration-decelleration");
			double dv = t.mNewValue - v;
			if (t.mDuration > 0)
				dv *= time / t.mDuration;
			v += dv;
			break;
		}
		
		if (v != vs.mVariable->GetValue())
		{
			result = true;
			static_cast<MXcbAnimationVariableImpl*>(vs.mVariable->GetImpl())->SetValue(v);
		}
	}
	
	return result;
}

bool MXcbStoryboardImpl::Done(double inTime)
{
	bool result = true;

	for (auto vs: mVariableStories)
	{
		double totalDuration = accumulate(vs.mTransistions.begin(), vs.mTransistions.end(), 0.0,
			[](double time, const MTransition& ts) -> double { return time + ts.mDuration; });
		
		if (totalDuration > inTime)
		{
			result = false;
			break;
		}
	}
	
	return result;
}

// --------------------------------------------------------------------

class MXcbAnimationManagerImpl : public MAnimationManagerImpl
{
  public:
	MXcbAnimationManagerImpl(MAnimationManager* inManager)
		: mAnimationManager(inManager)
		, mThread(bind(&MXcbAnimationManagerImpl::Run, this))
		, mDone(false)
	{
	}
	
	~MXcbAnimationManagerImpl()
	{
		if (not mDone)
			Stop();
	}

	virtual bool Update()			{ return false; }
	virtual void Stop()
	{
//		gdk_threads_leave();
		
		try
		{
			unique_lock<mutex> lock(mMutex);
			
			mStoryboards.clear();
			mDone = true;
			
			mCondition.notify_one();
			lock.unlock();
			
			if (mThread.joinable())
				mThread.join();
		}
		catch (...)	{}
		
//		gdk_threads_enter();
	}

	virtual MAnimationVariable* CreateVariable(double inValue, double inMin, double inMax)
	{
		return new MAnimationVariable(new MXcbAnimationVariableImpl(inValue, inMin, inMax));
	}
	
	virtual MStoryboard* CreateStoryboard()
	{
		return new MStoryboard(new MXcbStoryboardImpl());
	}
	
	virtual void Schedule(MStoryboard* inStoryboard)
	{
//		gdk_threads_leave();
		
		try
		{
			unique_lock<mutex> lock(mMutex);
	
			shared_ptr<MStoryboard> sbptr(inStoryboard);
			MScheduledStoryboard sb = { GetLocalTime(), sbptr };
			mStoryboards.push_back(sb);
			
			mCondition.notify_one();
		}
		catch (...) {}
		
//		gdk_threads_enter();
	}

	void Run();

	struct MScheduledStoryboard
	{
		double mStartTime;
		shared_ptr<MStoryboard> mStoryboard;
	};

	MAnimationManager* mAnimationManager;
	list<MScheduledStoryboard> mStoryboards;
	thread mThread;
	mutex mMutex;
	condition_variable mCondition;
	bool mDone;
};

void MXcbAnimationManagerImpl::Run()
{
	for (;;)
	{
		unique_lock<mutex> lock(mMutex);
		
		if (mDone)
			break;

		if (mStoryboards.empty())
		{
			mCondition.wait(lock);
			continue;
		}

		bool update = false;
		
		double now = GetLocalTime();
		
		for (auto storyboard: mStoryboards)
		{
			MXcbStoryboardImpl* storyboardImpl = static_cast<MXcbStoryboardImpl*>(storyboard.mStoryboard->GetImpl());
			update = storyboardImpl->Update(now - storyboard.mStartTime) or update;
		}
	
		if (update)
		{
//			gdk_threads_enter();
			try
			{
				mAnimationManager->eAnimate();
			}
			catch (exception& e)
			{
				PRINT(("Exception: %s", e.what()));
			}
			catch (...)
			{
				PRINT(("Exception"));
			}
			
//			gdk_threads_leave();
		}
		
		mStoryboards.erase(remove_if(mStoryboards.begin(), mStoryboards.end(),
			[now](MScheduledStoryboard& storyboard) -> bool
			{
				MXcbStoryboardImpl* storyboardImpl = static_cast<MXcbStoryboardImpl*>(storyboard.mStoryboard->GetImpl());
				return storyboardImpl->Done(now - storyboard.mStartTime);
			}), mStoryboards.end());

		usleep(10000);
	}
}

// --------------------------------------------------------------------

MAnimationManagerImpl* MAnimationManagerImpl::Create(MAnimationManager* inManager)
{
	return new MXcbAnimationManagerImpl(inManager);
}

