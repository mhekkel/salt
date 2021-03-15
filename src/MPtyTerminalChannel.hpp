#pragma once

#include "MTerminalChannel.hpp"

// --------------------------------------------------------------------
// MPtyTerminalChannel

class MPtyTerminalChannel : public MTerminalChannel
{
public:
	MPtyTerminalChannel();
	~MPtyTerminalChannel();

	virtual void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
								 uint32_t inPixelWidth, uint32_t inPixelHeight);

	virtual void Open(const string &inTerminalType,
					  bool inForwardAgent, bool inForwardX11,
					  const string &inCommand, const vector<string> &env,
					  const OpenCallback &inOpenCallback);

	virtual void Close();

	virtual bool IsOpen() const;

	virtual void SendData(const string &inData);
	virtual void SendSignal(const string &inSignal);
	virtual void ReadData(const ReadCallback &inCallback);

private:
	void Exec(const string &inCommand, const string &inTerminalType, int inTtyFD);

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
	string mTtyName;

	boost::asio::posix::stream_descriptor mPty;
	boost::asio::streambuf mResponse;
};
