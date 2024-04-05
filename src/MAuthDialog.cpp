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

/*	$Id: MAuthDialog.cpp,v 1.3 2004/02/07 13:10:08 maarten Exp $
    Copyright maarten
    Created Friday November 21 2003 19:38:34
*/

#include "MAuthDialog.hpp"
#include "MAlerts.hpp"
#include "MStrings.hpp"

#include <pinch.hpp>

#include <cmath>

// --------------------------------------------------------------------

MAuthDialog::MAuthDialog(const std::string &inTitle, MWindow *inParent, std::promise<std::string> inReply)
	: MDialog("auth-dialog")
	, mParent(inParent)
	, mPasswordReply(std::move(inReply))
{
	SetTitle(inTitle);

	SetText("instruction", _("Please provide password"));

	SetVisible("label-1", true);
	SetVisible("edit-1", true);

	SetText("label-1", _("Password"));

	SetPasswordChar("edit-1");

	for (int id = 2; id <= 5; ++id)
	{
		SetVisible("label-" + std::to_string(id), false);
		SetVisible("edit-" + std::to_string(id), false);
	}

	SetFocus("edit-1");
}

MAuthDialog::MAuthDialog(const std::string &inTitle, const std::string &name, const std::string &inInstruction,
	const std::vector<pinch::prompt> &prompts, MWindow *inParent, std::promise<std::vector<std::string>> inReply)
	: MDialog("auth-dialog")
	, mParent(inParent)
	, mCredentialsReply(std::move(inReply))
{
	mFields = prompts.size();

	SetTitle(inTitle);

	SetText("instruction", inInstruction);

	int32_t id = 1;
	for (auto prompt : prompts)
	{
		SetVisible("label-" + std::to_string(id), true);
		SetVisible("edit-" + std::to_string(id), true);

		SetText("label-" + std::to_string(id), prompt.str);

		// if (not prompt.echo)
		SetPasswordChar("edit-" + std::to_string(id));

		++id;
	}

	for (id = mFields + 1; id <= 5; ++id)
	{
		SetVisible("label-" + std::to_string(id), false);
		SetVisible("edit-" + std::to_string(id), false);
	}

	SetFocus("edit-1");
}

MAuthDialog::~MAuthDialog()
{
	if (mPasswordReply.has_value())
		mPasswordReply.value().set_exception(std::make_exception_ptr(std::system_error(pinch::error::auth_cancelled_by_user)));
	if (mCredentialsReply.has_value())
		mCredentialsReply.value().set_exception(std::make_exception_ptr(std::system_error(pinch::error::auth_cancelled_by_user)));
}

bool MAuthDialog::OKClicked()
{
	if (mPasswordReply.has_value())
	{
		mPasswordReply.value().set_value(GetText("edit-1"));
		mPasswordReply.reset();
	}

	if (mCredentialsReply.has_value())
	{
		std::vector<std::string> args;
		for (int32_t id = 1; id <= mFields; ++id)
			args.push_back(GetText("edit-" + std::to_string(id)));

		mCredentialsReply.value().set_value(std::move(args));
		mCredentialsReply.reset();
	}

	return true;
}

void MAuthDialog::RequestSimplePassword(
	const std::string &inDialogTitle, const std::string &inInstruction, MWindow *inParent,
	std::function<void(std::string)> &&inReplyCallback)
{
	inReplyCallback("");
#warning FIXME
	// return [](std::string &) { return false; };


	// return [inDialogTitle, inInstruction, inParent](std::string &outPassword)
	// {
	// 	bool result = false;

	// 	try
	// 	{
	// 		MAuthDialog *dlog = new MAuthDialog(inDialogTitle, inParent, [&outPassword](const std::string &pw)
	// 			{ outPassword = pw; });
	// 		result = dlog->ShowModal(inParent);
	// 	}
	// 	catch (const std::exception &e)
	// 	{
	// 		DisplayError(e);
	// 	}

	// 	return result;
	// };
}
