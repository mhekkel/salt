//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <iostream>
#include <cstdarg>
#include <cstring>
#include <cstdio>

#include "MError.hpp"
#include "MTypes.hpp"
#include "MUtils.hpp"
#include "MSound.hpp"
#include "MAlerts.hpp"
#include "MStrings.hpp"

using namespace std;

#ifndef NDEBUG
int StOKToThrow::sOkToThrow = 0;
#endif

//#ifndef _MSC_VER
//MException::MException(
//	int					inErr)
//{
//	snprintf(mMessage, sizeof(mMessage), "OS error %d", inErr);
//}
//#endif

MException::MException(
	const char*			inMsg,
	...)
{
	inMsg = GetLocalisedString(inMsg);
	
	va_list vl;
	va_start(vl, inMsg);
	vsnprintf(mMessage, sizeof(mMessage), inMsg, vl);
	va_end(vl);
	
	PRINT(("Throwing with msg: %s", mMessage));
}

const char*	MException::what() const throw()
{
	return mMessage;
}
