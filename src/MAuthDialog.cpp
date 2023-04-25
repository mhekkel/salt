//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id: MAuthDialog.cpp,v 1.3 2004/02/07 13:10:08 maarten Exp $
	Copyright maarten
	Created Friday November 21 2003 19:38:34
*/

#include "MLib.hpp"

#include <cmath>

#include <pinch.hpp>

#include "MAlerts.hpp"
#include "MAuthDialog.hpp"
#include "MStrings.hpp"

MAuthDialog::MAuthDialog(const std::string &inTitle, MWindow *inParent)
	: MDialog("auth-dialog")
	, mParent(inParent)
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

MAuthDialog::MAuthDialog(const std::string &inTitle,
	const std::string &name, const std::string &inInstruction,
	const std::vector<pinch::prompt> &prompts, MWindow *inParent)
	: MDialog("auth-dialog")
	, mParent(inParent)
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

		//if (not prompt.echo)
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
}

bool MAuthDialog::OKClicked()
{
	if (mPasswordHandler)
		mPasswordHandler(GetText("edit-1"));
	else if (mCredentialsHandler)
	{
		std::vector<std::string> args;
		for (int32_t id = 1; id <= mFields; ++id)
			args.push_back(GetText("edit-" + std::to_string(id)));
		mCredentialsHandler(args);
	}

	return true;
}

std::function<bool(std::string &)> MAuthDialog::RequestSimplePassword(
	const std::string &inDialogTitle, const std::string &inInstruction, MWindow *inParent)
{
	return [inDialogTitle, inInstruction, inParent](std::string &outPassword) {
		bool result = false;

		try
		{
			MAuthDialog *dlog = new MAuthDialog(inDialogTitle, inParent, [&outPassword](const std::string &pw) { outPassword = pw; });
			result = dlog->ShowModal(inParent);
		}
		catch (std::exception &e)
		{
			DisplayError(e);
		}

		return result;
	};
}
