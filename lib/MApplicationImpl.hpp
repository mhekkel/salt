//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MApplication.hpp"

class MApplicationImpl
{
  public:
					MApplicationImpl()		{}
	virtual			~MApplicationImpl()		{}

	virtual void	Initialise() = 0;
	virtual int		RunEventLoop() = 0;
	virtual void	Quit() = 0;
};
