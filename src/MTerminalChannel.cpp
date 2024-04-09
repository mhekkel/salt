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

#include "MTerminalChannel.hpp"
#include "MAlerts.hpp"
#include "MError.hpp"
#include "MSaltApp.hpp"
#include "MUtils.hpp"

#include <pinch.hpp>

#include <fstream>

using namespace std;

// --------------------------------------------------------------------
// MTerminalChannel

MTerminalChannel::MTerminalChannel()
	: mRefCount(1)
{
}

MTerminalChannel::~MTerminalChannel()
{
	assert(mRefCount == 0);
}

void MTerminalChannel::Release()
{
	if (--mRefCount == 0)
		delete this;
}

void MTerminalChannel::SetMessageCallback(const MessageCallback &inMessageCallback)
{
	mMessageCB = inMessageCallback;
}

void MTerminalChannel::Disconnect(bool disconnectProxy)
{
}

// --------------------------------------------------------------------
// MSshTerminalChannel

class MSshTerminalChannel : public MTerminalChannel
{
  public:
	MSshTerminalChannel(std::shared_ptr<pinch::basic_connection> inConnection);
	~MSshTerminalChannel();

	virtual void SetMessageCallback(const MessageCallback &inMessageCallback);

	virtual void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
		uint32_t inPixelWidth, uint32_t inPixelHeight);

	virtual void Open(const string &inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const std::vector<std::string> &inArgv, const vector<string> &env,
		const OpenCallback &inOpenCallback);

	virtual void Close();

	virtual bool IsOpen() const;

	virtual bool CanDisconnect() const { return true; }
	virtual void Disconnect(bool disconnectProxy);

	virtual void SendData(string &&inData);
	virtual void SendSignal(const string &inSignal);
	virtual void ReadData(const ReadCallback &inCallback);

	virtual bool CanDownloadFiles() const { return true; }
	virtual void DownloadFile(const std::string &remotepath, const std::string &localpath);
	virtual void UploadFile(const std::string &remotepath, const std::string &localpath);

  private:
	shared_ptr<pinch::terminal_channel> mChannel;
	asio_ns::streambuf mResponse;
};

MSshTerminalChannel::MSshTerminalChannel(std::shared_ptr<pinch::basic_connection> inConnection)
	: mChannel(new pinch::terminal_channel(inConnection))
{
	inConnection->keep_alive();
}

MSshTerminalChannel::~MSshTerminalChannel()
{
}

void MSshTerminalChannel::SetMessageCallback(const MessageCallback &inMessageCallback)
{
	MAppExecutor my_executor{ &MSaltApp::Instance().get_context() };

	mMessageCB = asio_ns::bind_executor(
		my_executor,
		[this, inMessageCallback](const std::string &s1, const std::string &s2)
		{
			inMessageCallback(s1, s2);
		});

	MTerminalChannel::SetMessageCallback(mMessageCB);
	mChannel->set_message_callbacks(mMessageCB, mMessageCB, mMessageCB);
}

void MSshTerminalChannel::SetTerminalSize(uint32_t inColumns, uint32_t inRows,
	uint32_t inPixelWidth, uint32_t inPixelHeight)
{
	mTerminalWidth = inColumns;
	mTerminalHeight = inRows;
	mPixelWidth = inPixelWidth;
	mPixelHeight = inPixelHeight;

	if (mChannel->is_open())
		mChannel->send_window_resize(mTerminalWidth, mTerminalHeight);
}

void MSshTerminalChannel::Open(const string &inTerminalType,
	bool inForwardAgent, bool inForwardX11,
	const std::vector<std::string> &inArgv, const vector<string> &env,
	const OpenCallback &inOpenCallback)
{
	// env is ignored anyway...

	MAppExecutor my_executor{ &MSaltApp::Instance().get_context() };

	auto cb = asio_ns::bind_executor(
		my_executor,
		[this, inOpenCallback](const std::error_code &ec)
		{
			auto &connection = mChannel->get_connection();
			mConnectionInfo = vector<string>({ connection.get_connection_parameters(pinch::direction::c2s),
				connection.get_connection_parameters(pinch::direction::s2c),
				connection.get_key_exchange_algorithm() });

			mConnectionInfo.erase(unique(mConnectionInfo.begin(), mConnectionInfo.end()), mConnectionInfo.end());

			if (this->mRefCount > 0)
				inOpenCallback(ec);
		});

	mChannel->open_with_pty(mTerminalWidth, mTerminalHeight,
		inTerminalType, inForwardAgent, inForwardX11, inArgv.empty() ? "" : inArgv.front(), std::move(cb));
}

void MSshTerminalChannel::Close()
{
	mChannel->close();
}

bool MSshTerminalChannel::IsOpen() const
{
	return mChannel->is_open();
}

void MSshTerminalChannel::Disconnect(bool disconnectProxy)
{
	mChannel->get_connection().close();
}

void MSshTerminalChannel::SendData(string &&inData)
{
	mChannel->send_data(std::move(inData));
}

void MSshTerminalChannel::SendSignal(const string &inSignal)
{
	mChannel->send_signal(inSignal);
}

void MSshTerminalChannel::ReadData(const ReadCallback &inCallback)
{
	MAppExecutor my_executor{ &MSaltApp::Instance().get_context() };

	auto cb = asio_ns::bind_executor(
		my_executor,
		[this, inCallback](const std::error_code &ec, size_t inBytesReceived)
		{
			if (this->mRefCount > 0)
				inCallback(ec, this->mResponse);
		});

	asio_ns::async_read(*mChannel, mResponse, asio_ns::transfer_at_least(1), std::move(cb));
}

void MSshTerminalChannel::DownloadFile(const std::string &remotepath, const std::string &localpath)
{
	auto p = std::make_shared<pinch::sftp_channel>(mChannel->get_connection().shared_from_this());

	p->async_init(3,
		[self = p, remotepath, localpath](const asio_system_ns::error_code &ec, int version)
		{
			if (ec or version != 3)
			{
				DisplayError(ec);
				self->close();
			}
			else
			{
				self->read_file(remotepath, localpath,
					[self](asio_system_ns::error_code ec, size_t bytes_transfered)
					{
						if (ec)
							DisplayError(ec);
					});
			}
		});
}

void MSshTerminalChannel::UploadFile(const std::string &remotepath, const std::string &localpath)
{
	auto p = std::make_shared<pinch::sftp_channel>(mChannel->get_connection().shared_from_this());

	p->async_init(3,
		[self = p, remotepath, localpath](const asio_system_ns::error_code &ec, int version)
		{
			if (ec or version != 3)
			{
				DisplayError(ec);
				self->close();
			}
			else
			{
				self->write_file(remotepath, localpath,
					[self](asio_system_ns::error_code ec, size_t bytes_transfered)
					{
						if (ec)
							DisplayError(ec);
					});
			}
		});
}

// --------------------------------------------------------------------
// MTerminalChannel factory

MTerminalChannel *MTerminalChannel::Create(std::shared_ptr<pinch::basic_connection> inConnection)
{
	return new MSshTerminalChannel(inConnection);
}
