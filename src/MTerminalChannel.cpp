// Copyright Maarten L. Hekkelman 2015
// All rights reserved

#include "MSalt.hpp"

#include <fstream>

#include <pinch.hpp>

#include "MTerminalChannel.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MSaltApp.hpp"

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
					  const string &inCommand, const vector<string> &env,
					  const OpenCallback &inOpenCallback);

	virtual void Close();

	virtual bool IsOpen() const;

	virtual bool CanDisconnect() const { return true; }
	virtual void Disconnect(bool disconnectProxy);

	virtual void SendData(string &&inData);
	virtual void SendSignal(const string &inSignal);
	virtual void ReadData(const ReadCallback &inCallback);

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
	MAppExecutor my_executor{&MSaltApp::instance().get_context()};

	mMessageCB = asio_ns::bind_executor(
		my_executor,
		[this, inMessageCallback](const std::string &s1, const std::string &s2) {
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
							   const string &inCommand, const vector<string> &env,
							   const OpenCallback &inOpenCallback)
{
	// env is ignored anyway...

	MAppExecutor my_executor{&MSaltApp::instance().get_context()};

	auto cb = asio_ns::bind_executor(
		my_executor,
		[this, inOpenCallback](const std::error_code &ec) {
			auto &connection = mChannel->get_connection();
			mConnectionInfo = vector<string>({connection.get_connection_parameters(pinch::direction::c2s),
											  connection.get_connection_parameters(pinch::direction::s2c),
											  connection.get_key_exchange_algorithm()});

			mConnectionInfo.erase(unique(mConnectionInfo.begin(), mConnectionInfo.end()), mConnectionInfo.end());

			if (this->mRefCount > 0)
				inOpenCallback(ec);
		});

	mChannel->open_with_pty(mTerminalWidth, mTerminalHeight,
							inTerminalType, inForwardAgent, inForwardX11, inCommand, std::move(cb));
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
	MAppExecutor my_executor{&MSaltApp::instance().get_context()};

	auto cb = asio_ns::bind_executor(
		my_executor,
		[this, inCallback](const std::error_code &ec, size_t inBytesReceived) {
			if (this->mRefCount > 0)
				inCallback(ec, this->mResponse);
		});

	asio_ns::async_read(*mChannel, mResponse, asio_ns::transfer_at_least(1), std::move(cb));
}

// --------------------------------------------------------------------
// MTerminalChannel factory

MTerminalChannel *MTerminalChannel::Create(std::shared_ptr<pinch::basic_connection> inConnection)
{
	return new MSshTerminalChannel(inConnection);
}
