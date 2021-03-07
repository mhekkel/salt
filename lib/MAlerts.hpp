//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <experimental/type_traits>
#include <sstream>
#include <vector>

#include <boost/system/error_code.hpp>

class MWindow;

void DisplayError(const std::exception &inException);
void DisplayError(const std::string &inError);
void DisplayError(const boost::system::error_code &inError);

// the actual implementation

template<typename T>
using to_string_t = decltype(std::to_string(std::declval<const T&>()));

template<typename T>
constexpr bool inline has_to_string_v = std::experimental::is_detected_v<to_string_t, T>;

int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName, std::vector<std::string> &inArguments);

inline int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName)
{
	std::vector<std::string> args;
	return DisplayAlert(inParent, inResourceName, args);
}

template <class T, typename... Args, std::enable_if_t<has_to_string_v<T>, int> = 0>
int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName, const T &inArgument, const Args &...inMoreArguments)
{
	std::vector<std::string> args(std::to_string(inArgument));
	return DisplayAlert(inParent, inResourceName, args, inMoreArguments...);
}

template <class T, typename... Args, std::enable_if_t<not has_to_string_v<T>, int> = 0>
int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName, const T &inArgument, const Args &...inMoreArguments)
{
	std::vector<std::string> args({ inArgument });
	return DisplayAlert(inParent, inResourceName, args, inMoreArguments...);
}

template <class T, typename... Args, std::enable_if_t<has_to_string_v<T>, int> = 0>
int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName, std::vector<std::string> &inArguments, const T &inArgument, const Args &...inMoreArguments)
{
	inArguments.push_back(std::to_string(inArgument));
	return DisplayAlert(inParent, inResourceName, inArguments, inMoreArguments...);
}

template <class T, typename... Args, std::enable_if_t<not has_to_string_v<T>, int> = 0>
int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName, std::vector<std::string> &inArguments, const T &inArgument, const Args &...inMoreArguments)
{
	inArguments.push_back(inArgument);
	return DisplayAlert(inParent, inResourceName, inArguments, inMoreArguments...);
}

