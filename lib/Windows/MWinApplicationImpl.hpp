//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MApplicationImpl.hpp"
#include "MWinProcMixin.hpp"

class MWinApplicationImpl : public MApplicationImpl
{
  public:
					MWinApplicationImpl(HINSTANCE inInstance);
	virtual			~MWinApplicationImpl();

	static MWinApplicationImpl*
					GetInstance()				{ return sInstance; }
	HINSTANCE		GetHInstance()				{ return mInstance; }

	virtual int		RunEventLoop();
	virtual void	Quit();

	virtual void	Initialise();

  private:

	virtual bool	IsAcceleratorKeyDown(MSG& inMessage);
	virtual void	TranslateAndDispatch(MSG& inMessage);

	virtual void	Pulse();

	HINSTANCE		mInstance;
	static MWinApplicationImpl*
					sInstance;
};
