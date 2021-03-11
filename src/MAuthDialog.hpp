//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id: MAuthDialog.h,v 1.2 2003/12/14 21:10:07 maarten Exp $
	Copyright maarten
	Created Thursday November 20 2003 21:36:39
*/

#pragma once

#include "MDialog.hpp"

class MAuthDialog : public MDialog
{
public:
	MAuthDialog(const std::string &inTitle, pinch::auth_state_type state,
				const std::string &name, const std::string &inInstruction,
				const std::vector<pinch::prompt> &prompts, MWindow* inParent);

	virtual ~MAuthDialog();

	MEventOut<void(pinch::auth_state_type, std::vector<std::string> &)> eAuthInfo;

	static std::function<bool(std::string &)>
	RequestSimplePassword(const std::string &inDialogTitle,
						  const std::string &inInstruction,
						  MWindow *inParent);

protected:
	virtual bool OKClicked();
	virtual bool CancelClicked();

	MEventIn<void(double)> ePulse;

	void Pulse(double inSystemTime);

	bool RequestSimplePassword(const std::string &inDialogTitle,
							   const std::string &inInstruction,
							   MWindow *inParent,
							   std::string &outPassword);

	int32_t mFields;
	bool mSentCredentials;
	pinch::auth_state_type mState;
	MWindow* mParent;
};
