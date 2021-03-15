//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if __has_include(<experimental/type_traits>)
#include <experimental/type_traits>
#elif __has_include(<zeep/type-traits.hpp>)
#include <zeep/type-traits.hpp>
#endif

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

int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName, const std::vector<std::string> &inArguments);
