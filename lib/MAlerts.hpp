//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MALERTS_H
#define MALERTS_H

#include <sstream>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>

class MWindow;

void DisplayError(const std::exception& inException);
void DisplayError(const std::string& inError);
void DisplayError(const boost::system::error_code& inError);

// the actual implementation

int32_t DisplayAlert(MWindow* inParent, const std::string& inResourceName, std::vector<std::string>& inArguments);

#if 0

template<class T, typename... Args>
int32_t DisplayAlert(const std::string& inResourceName, std::vector<std::string>& inArguments, const T& inArgument, const Args&... inMoreArguments)
{
	inArguments.push_back(boost::lexical_cast<std::string>(inArgument));
	return DisplayAlert(inResourceName, inArguments, inMoreArguments...);
}

#else

inline int32_t DisplayAlert(MWindow* inParent, const std::string& inResourceName)
{
	std::vector<std::string> args;
	return DisplayAlert(inParent, inResourceName, args);
}

template<class T0>
int32_t DisplayAlert(MWindow* inParent, const std::string& inResourceName, const T0& inArgument0)
{
	std::vector<std::string> args;
	args.push_back(boost::lexical_cast<std::string>(inArgument0));
	return DisplayAlert(inParent, inResourceName, args);
}

template<class T0, class T1>
int32_t DisplayAlert(MWindow* inParent, const std::string& inResourceName, const T0& inArgument0, const T1& inArgument1)
{
	std::vector<std::string> args;
	args.push_back(boost::lexical_cast<std::string>(inArgument0));
	args.push_back(boost::lexical_cast<std::string>(inArgument1));
	return DisplayAlert(inParent, inResourceName, args);
}

template<class T0, class T1, class T2>
int32_t DisplayAlert(MWindow* inParent, const std::string& inResourceName, const T0& inArgument0, const T1& inArgument1, const T2& inArgument2)
{
	std::vector<std::string> args;
	args.push_back(boost::lexical_cast<std::string>(inArgument0));
	args.push_back(boost::lexical_cast<std::string>(inArgument1));
	args.push_back(boost::lexical_cast<std::string>(inArgument2));
	return DisplayAlert(inParent, inResourceName, args);
}

template<class T0, class T1, class T2, class T3>
int32_t DisplayAlert(MWindow* inParent, const std::string& inResourceName, const T0& inArgument0, const T1& inArgument1, const T2& inArgument2, const T3& inArgument3)
{
	std::vector<std::string> args;
	args.push_back(boost::lexical_cast<std::string>(inArgument0));
	args.push_back(boost::lexical_cast<std::string>(inArgument1));
	args.push_back(boost::lexical_cast<std::string>(inArgument2));
	args.push_back(boost::lexical_cast<std::string>(inArgument3));
	return DisplayAlert(inParent, inResourceName, args);
}

#endif

#endif
