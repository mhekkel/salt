// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.h"

#include <cryptopp/base32.h>

#include "MPreferences.h"
#include "MAddTOTPHashDialog.h"
#include "MError.h"
#include "MAlerts.h"
#include "MUtils.h"

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
		
		using namespace CryptoPP;
		
		Base32Decoder decoder;
		
		decoder.Put((const unsigned char*)hash.c_str(), hash.length(), true);
		decoder.MessageEnd();
		
		word64 size = decoder.MaxRetrievable();
//		if (size != 8)
//			throw runtime_error("invalid hash");

		if (size < 8) // or (size % 8) != 0)
			throw runtime_error("invalid hash");

		vector<uint8> h(size);
	    decoder.Get(h.data(), size);
	    
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
