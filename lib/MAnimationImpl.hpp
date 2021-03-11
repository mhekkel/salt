//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// --------------------------------------------------------------------

class MAnimationManagerImpl
{
  public:
	virtual			~MAnimationManagerImpl() {}

	static MAnimationManagerImpl*	Create(MAnimationManager* inManager);

	virtual bool					Update() = 0;
	virtual void					Stop() {}

	virtual MAnimationVariable*		CreateVariable(double inValue, double inMin, double inMax) = 0;
	virtual MStoryboard*			CreateStoryboard() = 0;
	virtual void					Schedule(MStoryboard* inStoryboard) = 0;

  protected:
					MAnimationManagerImpl() {}	
};

// --------------------------------------------------------------------

class MAnimationVariableImpl
{
  public:
	virtual			~MAnimationVariableImpl() {}

	virtual double	GetValue() const = 0;

  protected:
					MAnimationVariableImpl() {}
};

// --------------------------------------------------------------------

class MStoryboardImpl
{
  public:
	virtual			~MStoryboardImpl() {}

	virtual void	AddTransition(MAnimationVariable* inVariable,
						double inNewValue, double inDuration,
						const char* inTransitionName) = 0;

  protected:
					MStoryboardImpl() {}
};

#if 0
// --------------------------------------------------------------------
//	A fall back implementation using threads and such

class MFallBackAnimationManagerImpl : public MAnimationManagerImpl
{

};


#endif // 0
