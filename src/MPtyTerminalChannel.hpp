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

// Copyright Maarten L. Hekkelman 2021
// All rights reserved

#pragma once

#include "MTerminalChannel.hpp"

#include <filesystem>

// --------------------------------------------------------------------
// MPtyTerminalChannel

class MPtyTerminalChannel : public MTerminalChannel
{
  public:
	MPtyTerminalChannel(MTerminalChannel *inCloneFrom);
	~MPtyTerminalChannel();

	void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
		uint32_t inPixelWidth, uint32_t inPixelHeight) override;

	void Open(const std::string &inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const std::vector<std::string> &inArgv, const std::vector<std::string> &env,
		const OpenCallback &inOpenCallback) override;

	void Close() override;

	bool IsOpen() const override;

	void SendData(std::string &&inData) override;
	void SendSignal(const std::string &inSignal) override;
	void ReadData(const ReadCallback &inCallback) override;

	std::filesystem::path GetCWD() const;
	void SetCWD(const std::filesystem::path &inCWD)
	{
		mCWD = inCWD;
	}

  private:
	void Execute(const std::vector<std::string> &inArgv,
		const std::string &inTerminalType, int inTtyFD);

	struct ChangeWindowsSizeCommand
	{
		ChangeWindowsSizeCommand(uint32_t inColumns, uint32_t inRows,
			uint32_t inPixelWidth, uint32_t inPixelHeight)
		{
			ws.ws_row = inRows;
			ws.ws_col = inColumns;
			ws.ws_xpixel = inPixelWidth;
			ws.ws_ypixel = inPixelHeight;
		}

		int name() { return TIOCSWINSZ; }
		void *data() { return &ws; }

		struct winsize ws;
	};

	int mPid;
	std::string mTtyName;
	std::filesystem::path mCWD;

	asio_ns::posix::stream_descriptor mPty;
	asio_ns::streambuf mResponse;
};
