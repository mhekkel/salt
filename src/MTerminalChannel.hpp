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

// Copyright Maarten L. Hekkelman 2015
// All rights reserved

#pragma once

#include <pinch.hpp>

class MTerminalChannel
{
  public:
	typedef std::function<void(std::error_code)> OpenCallback;
	typedef std::function<void(const std::string &, const std::string &)> MessageCallback;
	typedef std::function<void(std::error_code, std::size_t)> WriteCallback;
	typedef std::function<void(std::error_code, std::streambuf &inData)> ReadCallback;

	virtual void SetMessageCallback(const MessageCallback &inMessageCallback);

	virtual void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
		uint32_t inPixelWidth, uint32_t inPixelHeight) = 0;

	virtual void Open(const std::string &inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const std::vector<std::string> &inArgv, const std::vector<std::string> &env,
		const OpenCallback &inOpenCallback) = 0;

	virtual bool IsOpen() const = 0;
	virtual void Close() = 0;

	virtual bool CanDisconnect() const { return false; }
	virtual void Disconnect(bool disconnectProxy);

	void Release();

	virtual void SendData(std::string &&inData) = 0;

	void SendData(const std::string &s)
	{
		std::string copy(s);
		SendData(std::move(copy));
	}

	virtual void SendSignal(const std::string &inSignal) = 0;
	virtual void ReadData(const ReadCallback &inCallback) = 0;

	static MTerminalChannel *Create(std::shared_ptr<pinch::basic_connection> inConnection);
	static MTerminalChannel *Create();

	const std::vector<std::string> &GetConnectionInfo() const
	{
		return mConnectionInfo;
	}

	virtual bool CanDownloadFiles() const { return false; }
	virtual void DownloadFile(const std::string &remotepath, const std::string &localpath) {}
	virtual void UploadFile(const std::string &remotepath, const std::string &localpath) {}

  protected:
	MTerminalChannel();
	virtual ~MTerminalChannel();

	uint32_t mTerminalWidth, mTerminalHeight, mPixelWidth, mPixelHeight;

	OpenCallback mOpenCB;
	MessageCallback mMessageCB;

	std::vector<std::string> mConnectionInfo;
	uint32_t mRefCount;
};
