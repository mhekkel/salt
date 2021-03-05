//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include <vector>

#include <zeep/xml/document.hpp>

#include "MResources.hpp"
#include "MAlerts.hpp"
#include "MStrings.hpp"
#include "MWindow.hpp"
#include "MError.hpp"
#include "MSound.hpp"

using namespace std;
namespace xml = zeep::xml;
namespace io = boost::iostreams;

void DisplayError(
	const exception&	inErr)
{
	DisplayAlert(nullptr, "error-alert", inErr.what());
}

void DisplayError(const boost::system::error_code& ec)
{
	DisplayAlert(nullptr, "error-alert", ec.message());
}

void DisplayError(
	const string&		inErr)
{
	DisplayAlert(nullptr, "error-alert", inErr);
}

