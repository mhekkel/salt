// Copyright Maarten L. Hekkelman 2015
// All rights reserved

#pragma once

#include <pinch/channel.hpp>

class MTerminalChannel
{
  public:
	typedef std::function<void(boost::system::error_code)> OpenCallback;
	typedef std::function<void(const std::string &, const std::string &)> MessageCallback;
	typedef std::function<void(boost::system::error_code, std::size_t)> WriteCallback;
	typedef std::function<void(boost::system::error_code, std::streambuf &inData)> ReadCallback;

	virtual void SetMessageCallback(const MessageCallback &inMessageCallback);

	virtual void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
		uint32_t inPixelWidth, uint32_t inPixelHeight) = 0;

	virtual void Open(const std::string &inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const std::string &inCommand, const std::vector<std::string> &env,
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

  protected:
	MTerminalChannel();
	virtual ~MTerminalChannel();

	uint32_t mTerminalWidth, mTerminalHeight, mPixelWidth, mPixelHeight;

	OpenCallback mOpenCB;
	MessageCallback mMessageCB;

	std::vector<std::string> mConnectionInfo;
	uint32_t mRefCount;
};
