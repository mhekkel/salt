//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include "MAlerts.hpp"

void DisplayError(const std::exception &inErr)
{
	DisplayAlert(nullptr, "error-alert", inErr.what());
}

void DisplayError(const boost::system::error_code &ec)
{
	DisplayAlert(nullptr, "error-alert", ec.message());
}

void DisplayError(const std::string &inErr)
{
	DisplayAlert(nullptr, "error-alert", inErr);
}
