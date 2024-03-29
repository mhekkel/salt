/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MAddTOTPHashDialog.hpp"
#include "MAlerts.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MSaltApp.hpp"
#include "MUtils.hpp"

#include <zeep/crypto.hpp>

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
		std::string name = GetText("name");
		if (name.empty())
			throw std::runtime_error("Empty name");

		std::string hash = GetText("hash");
		if (hash.empty())
			throw std::runtime_error("Empty hash");

		// validate hash

		std::string b = zeep::decode_base32(hash);
		if (b.length() < 8) // or (size % 8) != 0)
			throw std::runtime_error("invalid hash");

		auto totp = MPrefs::GetArray("totp");
		totp.push_back(name + ";" + hash);
		MPrefs::SetArray("totp", totp);

		MSaltApp::Instance().UpdateTOTPMenu();

		result = true;
	}
	catch (const std::exception &ex)
	{
		DisplayError(ex);
	}

	return result;
}
