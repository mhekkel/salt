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
#include <boost/lexical_cast.hpp>

#include <pinch/channel.hpp>

#include "MAuthDialog.hpp"
#include "MAlerts.hpp"
#include "MStrings.hpp"

using namespace std;

MAuthDialog::MAuthDialog(std::string inTitle, std::string inInstruction, const vector<pinch::prompt>& prompts)
	: MDialog("auth-dialog")
	, ePulse(this, &MAuthDialog::Pulse)
	, mSentCredentials(false)
{
	mFields = prompts.size();

	SetTitle(inTitle);
	
	SetText("instruction", inInstruction);
	
	int32_t id = 1;
	for (auto prompt: prompts)
	{
		SetVisible("label-" + boost::lexical_cast<string>(id), true);
		SetVisible("edit-" + boost::lexical_cast<string>(id), true);

		SetText("label-" + boost::lexical_cast<string>(id), prompt.str);

		//if (not prompt.echo)
			SetPasswordChar("edit-" + boost::lexical_cast<string>(id));
		
		++id;
	}

	for (id = mFields + 1; id <= 5; ++id)
	{
		SetVisible("label-" + boost::lexical_cast<string>(id), false);
		SetVisible("edit-" + boost::lexical_cast<string>(id), false);
	}
	
	SetFocus("edit-1");
}

MAuthDialog::~MAuthDialog()
{
	if (not mSentCredentials)
		CancelClicked();
}

bool MAuthDialog::OKClicked()
{
	vector<string> args;
	
	for (int32_t id = 1; id <= mFields; ++id)
		args.push_back(GetText("edit-" + boost::lexical_cast<string>(id)));
	
	eAuthInfo(args);
	mSentCredentials = true;

	return true;
}

bool MAuthDialog::CancelClicked()
{
	vector<string> args;
	
	eAuthInfo(args);
	mSentCredentials = true;
	
	return true;
}

void MAuthDialog::Pulse(
	double		inTime)
{
//	SetNodeVisible('caps',
//		ModifierKeyDown(kAlphaLock) && (std::fmod(inTime, 1.0) <= 0.5));
}

function<bool(string&)> MAuthDialog::RequestSimplePassword(
	const string& inDialogTitle, const string& inInstruction)
{
	return [inDialogTitle, inInstruction](string& outPassword) -> bool
	{
		bool result = false;
		
		try
		{
			vector<pinch::prompt> prompts(1);
	
			prompts[0].str = _("Password");
			prompts[0].echo = false;

			MAuthDialog* dlog = new MAuthDialog(inDialogTitle, inInstruction, prompts);

			MEventIn<void(vector<string>&)> pwevent(
				[&outPassword](vector<string>& pws) { outPassword = pws[0]; });
			AddRoute(dlog->eAuthInfo, pwevent);

			result = dlog->ShowModal(nullptr);
		}
		catch (exception& e)
		{
			DisplayError(e);
		}
		
		return result;
	};
}

