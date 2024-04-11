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
#include "MStrings.hpp"
#include "MUtils.hpp"

#include <pinch.hpp>

#include <asio/experimental/awaitable_operators.hpp>

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

	void SetMessageCallback(const MessageCallback &inMessageCallback) override;

	void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
		uint32_t inPixelWidth, uint32_t inPixelHeight) override;

	void Open(const string &inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const std::vector<std::string> &inArgv, const vector<string> &env,
		const OpenCallback &inOpenCallback) override;

	void Close() override;

	bool IsOpen() const override;

	bool CanDisconnect() const override { return true; }
	void Disconnect(bool disconnectProxy) override;

	void SendData(string &&inData) override;
	void SendSignal(const string &inSignal) override;
	void ReadData(const ReadCallback &inCallback) override;

	bool CanDownloadFiles() const override { return true; }
	void DownloadFile(const std::filesystem::path &remotepath, const std::filesystem::path &localpath) override;
	void UploadFile(const std::filesystem::path &remotepath, const std::filesystem::path &localpath) override;
	void UploadFileTo(const std::filesystem::path &localpath, const std::filesystem::path &remote_directory) override;

	asio_ns::awaitable<void> DoDownloadFile(std::filesystem::path remotepath, std::filesystem::path localpath);
	asio_ns::awaitable<void> DoUploadFile(std::filesystem::path remotepath, std::filesystem::path localpath);
	asio_ns::awaitable<void> DoUploadFileTo(std::filesystem::path localpath, std::filesystem::path remote_directory);

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

void ReportError(asio_system_ns::error_code ec)
{
	MSaltApp::Instance().execute([ec]()
		{ DisplayError(ec); });
}

void MSshTerminalChannel::DownloadFile(const std::filesystem::path &remotepath, const std::filesystem::path &localpath)
{
	asio_ns::co_spawn(MSaltApp::Instance().get_io_context(), DoDownloadFile(remotepath, localpath), asio_ns::detached);
}

asio_ns::awaitable<void> MSshTerminalChannel::DoDownloadFile(std::filesystem::path remotepath, std::filesystem::path localpath)
{
	try
	{
		auto p = std::make_shared<pinch::sftp_channel>(mChannel->get_connection().shared_from_this());

		auto version = co_await p->async_init(3, asio_ns::use_awaitable);
		if (version != 3)
			throw std::runtime_error("Wrong SFTP version");

		eIOStatus(FormatString("Downloading ^0", remotepath.filename().string()));
		co_await p->read_file(remotepath.string(), localpath.string(), asio_ns::use_awaitable);
		eIOStatus(FormatString("Downloaded ^0", localpath.filename().string()));
	}
	catch (const std::exception &ex)
	{
		eIOStatus(ex.what());
		std::cerr << "error: " << ex.what() << "\n";
	}
}

void MSshTerminalChannel::UploadFile(const std::filesystem::path &remotepath, const std::filesystem::path &localpath)
{
	asio_ns::co_spawn(MSaltApp::Instance().get_io_context(), DoUploadFile(remotepath, localpath), asio_ns::detached);
}

asio_ns::awaitable<void> MSshTerminalChannel::DoUploadFile(std::filesystem::path remotepath, std::filesystem::path localpath)
{
	try
	{
		auto p = std::make_shared<pinch::sftp_channel>(mChannel->get_connection().shared_from_this());

		auto version = co_await p->async_init(3, asio_ns::use_awaitable);
		if (version != 3)
			throw std::runtime_error("Wrong SFTP version");

		eIOStatus(FormatString("Uploading ^0", remotepath.filename().string()));
		co_await p->write_file(remotepath.string(), localpath.string(), asio_ns::use_awaitable);
		eIOStatus(FormatString("Uploaded ^0", localpath.filename().string()));
	}
	catch (const std::exception &ex)
	{
		eIOStatus(ex.what());
		std::cerr << "error: " << ex.what() << "\n";
	}
}

void MSshTerminalChannel::UploadFileTo(const std::filesystem::path &localpath, const std::filesystem::path &remote_dir)
{
	asio_ns::co_spawn(MSaltApp::Instance().get_io_context(), DoUploadFileTo(localpath, remote_dir), asio_ns::detached);
}

asio_ns::awaitable<void> MSshTerminalChannel::DoUploadFileTo(std::filesystem::path localpath, std::filesystem::path remote_dir)
{
	try
	{
		auto p = std::make_shared<pinch::sftp_channel>(mChannel->get_connection().shared_from_this());

		auto version = co_await p->async_init(3, asio_ns::use_awaitable);
		if (version != 3)
			throw std::runtime_error("Wrong SFTP version");

		if (remote_dir.empty())
			remote_dir = ".";
		else if (not remote_dir.is_absolute())
			remote_dir = "." / remote_dir;

		co_await p->make_dir(remote_dir, asio_ns::use_awaitable);

		auto files = co_await p->read_dir(remote_dir, asio_ns::use_awaitable);

		// see if filename needs a trailing number

		auto filename = localpath.filename();

		eIOStatus(FormatString("Uploading ^0", filename.string()));

		int nr = 0;
		for (;;)
		{
			bool exists = false;
			for (const auto &[name, longname, attr] : files)
			{
				if (name != filename.string())
					continue;

				exists = true;
				break;
			}

			if (not exists)
				break;

			filename = localpath.filename().stem().string() + '-' + std::to_string(++nr) + localpath.filename().extension().string();
		}

		auto written = co_await p->write_file((remote_dir / filename).string(), localpath, asio_ns::use_awaitable);

		eIOStatus(FormatString("Uploaded ^0", filename.string()));
	}
	catch (const asio_system_ns::system_error &e)
	{
		eIOStatus(e.what());
		std::cerr << "system error: " << e.what() << "\n";
	}
	catch (const std::exception &ex)
	{
		eIOStatus(ex.what());
		std::cerr << "error: " << ex.what() << "\n";
	}
	catch (...)
	{
		std::cerr << "exception\n";
	}
}

// --------------------------------------------------------------------
// MTerminalChannel factory

MTerminalChannel *MTerminalChannel::Create(std::shared_ptr<pinch::basic_connection> inConnection)
{
	return new MSshTerminalChannel(inConnection);
}
