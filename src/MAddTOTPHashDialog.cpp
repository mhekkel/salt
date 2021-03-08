// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <zeep/crypto.hpp>

#include "MPreferences.hpp"
#include "MAddTOTPHashDialog.hpp"
#include "MError.hpp"
#include "MAlerts.hpp"
#include "MUtils.hpp"

using namespace std;

// --------------------------------------------------------------------

MAddTOTPHashDialog::MAddTOTPHashDialog()
	: MDialog("add-totp-hash")
{
}

MAddTOTPHashDialog::~MAddTOTPHashDialog()
{
}

bool MAddTOTPHashDialog::OKClicked()
{
	bool result = false;
	
	try
	{
		string name = GetText("name");
		if (name.empty())
			throw runtime_error("Empty name");
		
		string hash = GetText("hash");
		if (hash.empty())
			throw runtime_error("Empty hash");

		// validate hash

		std::string b = zeep::decode_base32(hash);
		if (b.length() < 8) // or (size % 8) != 0)
			throw runtime_error("invalid hash");

	    vector<string> totp;
	    Preferences::GetArray("totp", totp);
	    totp.push_back(name + ";" + hash);
	    Preferences::SetArray("totp", totp);

		result = true;
	}
	catch (const exception& ex)
	{
		DisplayError(ex);
	}
	
	return result;
}
