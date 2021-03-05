// Copyright Maarten L. Hekkelman 2015
// All rights reserved

#pragma once

#include <pinch/connection.hpp>

class MTerminalChannel
{
  public:
	typedef std::function<void(boost::system::error_code)> OpenCallback;
	typedef std::function<void(const std::string&, const std::string&)> MessageCallback;
	typedef std::function<void(boost::system::error_code, std::size_t)> WriteCallback;
	typedef std::function<void(boost::system::error_code, std::streambuf& inData)> ReadCallback;

	virtual void SetMessageCallback(MessageCallback&& inMessageCallback);
	
	virtual void SetTerminalSize(uint32 inColumns, uint32 inRows,
		uint32 inPixelWidth, uint32 inPixelHeight) = 0;
	
	virtual void Open(const std::string& inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const std::string& inCommand, const std::vector<std::string>& env,
		OpenCallback&& inOpenCallback) = 0;
	virtual bool IsOpen() const = 0;
	virtual void Close() = 0;
	
	virtual bool CanDisconnect() const { return false; }
	virtual void Disconnect(bool disconnectProxy);

	void Release();

	virtual void SendData(const std::string& inData, WriteCallback&& inCallback) = 0;
	virtual void SendSignal(const std::string& inSignal) = 0;
	virtual void ReadData(ReadCallback&& inCallback) = 0;
	
	boost::asio::io_service& GetIOService() { return mIOService; }
	
	static MTerminalChannel* Create(pinch::basic_connection* inConnection);
	static MTerminalChannel* Create(boost::asio::io_service& inIOService);

	const std::vector<std::string>& GetConnectionInfo() const
	{
		return mConnectionInfo;
	}
	
	virtual void KeepAliveIfNeeded();
	
  protected:

	MTerminalChannel(boost::asio::io_service& inIOService);
	virtual ~MTerminalChannel();

	uint32 mTerminalWidth, mTerminalHeight, mPixelWidth, mPixelHeight;

	OpenCallback mOpenCB;
	MessageCallback mMessageCB;
	
	boost::asio::io_service& mIOService;
	std::vector<std::string> mConnectionInfo;
	uint32 mRefCount;
};
