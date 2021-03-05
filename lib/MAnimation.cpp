//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include "MAnimation.hpp"
#include "MAnimationImpl.hpp"

using namespace std;

// --------------------------------------------------------------------

MAnimationVariable::MAnimationVariable(MAnimationVariableImpl* inImpl)
	: mImpl(inImpl)
{
}

MAnimationVariable::~MAnimationVariable()
{
	delete mImpl;
}

double MAnimationVariable::GetValue() const
{
	return mImpl->GetValue();
}

// --------------------------------------------------------------------

MStoryboard::MStoryboard(MStoryboardImpl* inImpl)
	: mImpl(inImpl)
{
}

MStoryboard::~MStoryboard()
{
	delete mImpl;
}

void MStoryboard::AddTransition(MAnimationVariable* inVariable,
	double inNewValue, double inDuration, const char* inTransitionName)
{
	mImpl->AddTransition(inVariable, inNewValue, inDuration, inTransitionName);
}

// --------------------------------------------------------------------

MAnimationManager::MAnimationManager()
	: mImpl(MAnimationManagerImpl::Create(this))
{
}

MAnimationManager::~MAnimationManager()
{
	delete mImpl;
}

bool MAnimationManager::Update()
{
	return mImpl->Update();
}

void MAnimationManager::Stop()
{
	mImpl->Stop();
}

MAnimationVariable* MAnimationManager::CreateVariable(double inValue, double inMin, double inMax)
{
	return mImpl->CreateVariable(inValue, inMin, inMax);
}

MStoryboard* MAnimationManager::CreateStoryboard()
{
	return mImpl->CreateStoryboard();
}

void MAnimationManager::Schedule(MStoryboard* inStoryboard)
{
	mImpl->Schedule(inStoryboard);
}

