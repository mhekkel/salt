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

#pragma once

#include "MDialog.hpp"

#include <optional>

struct ConnectInfoBase
{
	std::string host;
	std::string user;
	uint16_t port = 22;

	std::string HostAndPortString() const
	{
		return port == 22 ? host : host + ':' + std::to_string(port);
	}

	auto operator<=>(const ConnectInfoBase &) const noexcept = default;

	friend std::ostream &operator<<(std::ostream &os, const ConnectInfoBase &c);

	explicit operator bool() const
	{
		return not host.empty();
	}
};

struct ProxyInfo : public ConnectInfoBase
{
	std::string command;

	auto operator<=>(const ProxyInfo &) const noexcept = default;

	friend std::ostream &operator<<(std::ostream &os, const ProxyInfo &c);
};

struct ConnectInfo : public ConnectInfoBase
{
	std::optional<ProxyInfo> proxy;

	auto operator<=>(const ConnectInfo &) const noexcept = default;

	std::string str() const
	{
		return (std::ostringstream() << *this).str();
	}

	std::string DisplayString() const;
	static ConnectInfo parse(const std::string &s);

	friend std::ostream &operator<<(std::ostream &os, const ConnectInfo &c);
};

// --------------------------------------------------------------------

class MConnectDialog : public MDialog
{
  public:
	MConnectDialog();

	bool CancelClicked() override;
	bool OKClicked() override;

	// void TextChanged(const std::string &inID, const std::string &inText) override;
	void ValueChanged(const std::string &inID, int32_t inValue) override;

	void CheckboxChanged(const std::string &inID, bool inValue) override;
	void ButtonClicked(const std::string &inID) override;

	static std::vector<ConnectInfo> GetRecentHosts();

  private:
	void SelectProxy(const ProxyInfo &inProxy);
	void SelectRecent(const ConnectInfo &inRecent);

	std::vector<ConnectInfo> mRecentSessions;
	std::vector<ProxyInfo> mRecentProxies;
};
