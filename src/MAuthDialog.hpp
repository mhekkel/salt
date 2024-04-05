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

/*	$Id: MAuthDialog.h,v 1.2 2003/12/14 21:10:07 maarten Exp $
    Copyright maarten
    Created Thursday November 20 2003 21:36:39
*/

#pragma once

#include "MDialog.hpp"

#include <pinch.hpp>

#include <optional>

// --------------------------------------------------------------------

class MAuthDialog : public MDialog
{
  public:
	MAuthDialog(const std::string &inTitle, MWindow *inParent, std::promise<std::string> inReply);

	MAuthDialog(const std::string &inTitle, const std::string &name, const std::string &inInstruction,
		const std::vector<pinch::prompt> &prompts, MWindow *inParent, std::promise<std::vector<std::string>> inReply);

	virtual ~MAuthDialog();

	static void RequestSimplePassword(
		const std::string &inDialogTitle,
		const std::string &inInstruction,
		MWindow *inParent,
		std::function<void(std::string)> &&inReplyCallback);

  protected:

	virtual bool OKClicked();

	int32_t mFields = -1;
	MWindow *mParent;

	std::optional<std::promise<std::string>> mPasswordReply;
	std::optional<std::promise<std::vector<std::string>>> mCredentialsReply;
};
