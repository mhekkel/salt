//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MP2PEvents.hpp"

class MAnimationManagerImpl;
class MAnimationVariableImpl;
class MStoryboardImpl;

class MAnimationVariable
{
  public:
							MAnimationVariable(MAnimationVariableImpl* inImpl);
							~MAnimationVariable();

	double					GetValue() const;

	MAnimationVariableImpl*	GetImpl()				{ return mImpl; }

  private:
	MAnimationVariableImpl*	mImpl;
};

class MStoryboard
{
  public:
							MStoryboard(MStoryboardImpl* inImpl);
							~MStoryboard();

	void					AddTransition(MAnimationVariable* inVariable,
								double inNewValue, double inDuration,
								const char* inTransitionName);

	MStoryboardImpl*		GetImpl()				{ return mImpl; }

  private:
	MStoryboardImpl*		mImpl;
};

class MAnimationManager
{
  public:
							MAnimationManager();
							~MAnimationManager();
	
	MEventOut<void()>		eAnimate;
	
	bool					Update();
	void					Stop();
	
	MAnimationVariable*		CreateVariable(double inValue, double inMin, double inMax);
	MStoryboard*			CreateStoryboard();
	void					Schedule(MStoryboard* inStoryboard);
	
  private:
	MAnimationManagerImpl*	mImpl;
};
