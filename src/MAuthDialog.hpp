//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id: MAuthDialog.h,v 1.2 2003/12/14 21:10:07 maarten Exp $
	Copyright maarten
	Created Thursday November 20 2003 21:36:39
*/

#pragma once

#include <pinch.hpp>

#include "MDialog.hpp"

class MAuthDialog : public MDialog
{
  public:

	template<typename Handler>
	MAuthDialog(const std::string &inTitle, MWindow *inParent, Handler&& handler)
		: MAuthDialog(inTitle, inParent)
	{
		mPasswordHandler = std::move(handler);
	}

	template<typename Handler>
	MAuthDialog(const std::string &inTitle,
		const std::string &name, const std::string &inInstruction,
		const std::vector<pinch::prompt> &prompts, MWindow *inParent, Handler&& handler)
		: MAuthDialog(inTitle, name, inInstruction, prompts, inParent)
	{
		mCredentialsHandler = std::move(handler);
	}

	virtual ~MAuthDialog();

	static std::function<bool(std::string &)> RequestSimplePassword(
		const std::string &inDialogTitle,
		const std::string &inInstruction,
		MWindow *inParent);

  protected:

	MAuthDialog(const std::string &inTitle, MWindow *inParent);

	MAuthDialog(const std::string &inTitle,
		const std::string &name, const std::string &inInstruction,
		const std::vector<pinch::prompt> &prompts, MWindow *inParent);

	virtual bool OKClicked();

	bool RequestSimplePassword(const std::string &inDialogTitle,
		const std::string &inInstruction,
		MWindow *inParent,
		std::string &outPassword);

	int32_t mFields = 1;
	MWindow *mParent;

	std::function<void(std::string)> mPasswordHandler;
	std::function<void(std::vector<std::string>)> mCredentialsHandler;
};
