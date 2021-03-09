// Copyright Maarten L. Hekkelman 2015
// All rights reserved

#define BOOST_ASIO_HAS_MOVE 1

#include "MSalt.hpp"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pty.h>

#include <fstream>

#include <pinch/terminal_channel.hpp>

#include "MTerminalChannel.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MSaltApp.hpp"

using namespace std;
//namespace ba = boost::algorithm;
//namespace io = boost::iostreams;

// --------------------------------------------------------------------
// MTerminalChannel

MTerminalChannel::MTerminalChannel(boost::asio::io_service& inIOService)
	: mIOService(inIOService)
	, mRefCount(1)
{
}

MTerminalChannel::~MTerminalChannel()
{
	PRINT(("Deleting MTerminalChannel"));
	assert(mRefCount == 0);
}

void MTerminalChannel::Release()
{
	if (--mRefCount == 0)
		delete this;
}

void MTerminalChannel::SetMessageCallback(const MessageCallback& inMessageCallback)
{
	mMessageCB = inMessageCallback;
}

void MTerminalChannel::Disconnect(bool disconnectProxy)
{
}

void MTerminalChannel::KeepAliveIfNeeded()
{
}

// --------------------------------------------------------------------
// MSshTerminalChannel

class MSshTerminalChannel : public MTerminalChannel
{
  public:
	MSshTerminalChannel(std::shared_ptr<pinch::basic_connection> inConnection);
	~MSshTerminalChannel();
	
	virtual void SetMessageCallback(const MessageCallback& inMessageCallback);
	
	virtual void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
		uint32_t inPixelWidth, uint32_t inPixelHeight);
	
	virtual void Open(const string& inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const string& inCommand, const vector<string>& env,
		const OpenCallback& inOpenCallback);

	virtual void Close();

	virtual bool IsOpen() const;
	virtual void KeepAliveIfNeeded();

	virtual bool CanDisconnect() const { return true; }
	virtual void Disconnect(bool disconnectProxy);

	virtual void SendData(const string& inData, const WriteCallback& inCallback);
	virtual void SendSignal(const string& inSignal);
	virtual void ReadData(const ReadCallback& inCallback);

  private:
	shared_ptr<pinch::terminal_channel> mChannel;
	boost::asio::streambuf mResponse;
};

MSshTerminalChannel::MSshTerminalChannel(std::shared_ptr<pinch::basic_connection> inConnection)
	: MTerminalChannel(static_cast<MSaltApp*>(gApp)->GetIOService())
	, mChannel(new pinch::terminal_channel(inConnection))
{
}

MSshTerminalChannel::~MSshTerminalChannel()
{
}

void MSshTerminalChannel::SetMessageCallback(const MessageCallback& inMessageCallback)
{
	MTerminalChannel::SetMessageCallback(inMessageCallback);
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

void MSshTerminalChannel::Open(const string& inTerminalType,
	bool inForwardAgent, bool inForwardX11,
	const string& inCommand, const vector<string>& env,
	const OpenCallback& inOpenCallback)
{
	// env is ignored anyway...
	
	mChannel->open_with_pty(mTerminalWidth, mTerminalHeight,
		inTerminalType, inForwardAgent, inForwardX11, inCommand,
		[this, inOpenCallback](const boost::system::error_code& ec)
		{
			auto& connection = mChannel->get_connection();
			mConnectionInfo = vector<string>({
				connection.get_connection_parameters(pinch::direction::c2s),
				connection.get_connection_parameters(pinch::direction::s2c),
				connection.get_key_exchange_algorithm()
			});

			mConnectionInfo.erase(unique(mConnectionInfo.begin(), mConnectionInfo.end()), mConnectionInfo.end());

			if (this->mRefCount > 0)
				inOpenCallback(ec);
		});
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
	mChannel->get_connection().disconnect();
}

void MSshTerminalChannel::SendData(const string& inData, const WriteCallback& inCallback)
{
	mChannel->send_data(inData, 
		[this, inCallback](const boost::system::error_code& ec, size_t bytes)
		{
			if (this->mRefCount > 0)
				inCallback(ec, bytes);
		});
}

void MSshTerminalChannel::SendSignal(const string& inSignal)
{
	mChannel->send_signal(inSignal);
}

void MSshTerminalChannel::ReadData(const ReadCallback& inCallback)
{
	boost::asio::async_read(*mChannel, mResponse,
		boost::asio::transfer_at_least(1),
		[this, inCallback](const boost::system::error_code& ec, size_t inBytesReceived)
		{
			if (this->mRefCount > 0)
				inCallback(ec, this->mResponse);
		});
}

void MSshTerminalChannel::KeepAliveIfNeeded()
{
	if (mChannel->is_open())
		mChannel->keep_alive();
}

// --------------------------------------------------------------------
// MPtyTerminalChannel

class MPtyTerminalChannel : public MTerminalChannel
{
  public:
	MPtyTerminalChannel(boost::asio::io_context& inIOContext);
	~MPtyTerminalChannel();
	
	virtual void SetTerminalSize(uint32_t inColumns, uint32_t inRows,
		uint32_t inPixelWidth, uint32_t inPixelHeight);
	
	virtual void Open(const string& inTerminalType,
		bool inForwardAgent, bool inForwardX11,
		const string& inCommand, const vector<string>& env,
		const OpenCallback& inOpenCallback);

	virtual void Close();

	virtual bool IsOpen() const;

	virtual void SendData(const string& inData, const WriteCallback& inCallback);
	virtual void SendSignal(const string& inSignal);
	virtual void ReadData(const ReadCallback& inCallback);

  private:

	void Exec(const string& inCommand, const string& inTerminalType, int inTtyFD);
	
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
		
		int name()		{ return TIOCSWINSZ; }
		void* data()	{ return &ws; }
		
		struct winsize ws;
	};
	
	int mPid;
	string mTtyName;
	
	boost::asio::posix::stream_descriptor mPty;
	boost::asio::streambuf mResponse;
};

MPtyTerminalChannel::MPtyTerminalChannel(boost::asio::io_context& inIOContext)
	: MTerminalChannel(inIOContext)
	, mPid(-1)
	, mPty(mIOService)
{
}

MPtyTerminalChannel::~MPtyTerminalChannel()
{
PRINT(("MPtyTerminalChannel::~MPtyTerminalChannel()"));
}

void MPtyTerminalChannel::SetTerminalSize(uint32_t inColumns, uint32_t inRows,
	uint32_t inPixelWidth, uint32_t inPixelHeight)
{
	mTerminalWidth = inColumns;
	mTerminalHeight = inRows;
	mPixelWidth = inPixelWidth;
	mPixelHeight = inPixelHeight;

	if (mPty.is_open())
	{
		ChangeWindowsSizeCommand cmd(inColumns, inRows, inPixelWidth, inPixelHeight);
		mPty.io_control(cmd);
	}
}

void MPtyTerminalChannel::Open(const string& inTerminalType,
	bool inForwardAgent, bool inForwardX11,
	const string& inCommand, const vector<string>& env,
	const OpenCallback& inOpenCallback)
{
	int ptyfd = -1, ttyfd = -1;
	
	try
	{
		mPty.close();

		struct winsize w;
	
		w.ws_row = mTerminalHeight;
		w.ws_col = mTerminalWidth;
		w.ws_xpixel = mPixelWidth;
		w.ws_ypixel = mPixelHeight;
		
		// allocate a pty
		if (openpty(&ptyfd, &ttyfd, nullptr, nullptr, &w) < 0)
			throw runtime_error(strerror(errno));
			
		mTtyName = ttyname(ttyfd);
		mConnectionInfo = vector<string>({ mTtyName });
	
		mPid = fork();
		switch (mPid)
		{
			case -1:
				throw runtime_error(strerror(errno));

			case  0:
				close(ptyfd);
				Exec(inCommand, inTerminalType, ttyfd);
				// does not return
			
			default:
				break;
		}

		close(ttyfd);
		mPty.assign(ptyfd);
	
		inOpenCallback(boost::system::error_code());
	}
	catch (exception& e)
	{
		mMessageCB(e.what(), "");

		if (ptyfd >= 0)
			close(ptyfd);
		if (ttyfd >= 0)
			close(ttyfd);
		
		mPty.close();
	
		inOpenCallback(pinch::error::make_error_code(pinch::error::channel_closed));
	}
}

void MPtyTerminalChannel::Exec(
	const string& inCommand, const string& inTerminalType, int inTtyFD)
{
	// make the pseudo tty our controlling tty
	int fd = open("/dev/tty", O_RDWR | O_NOCTTY);
	if (fd >= 0)
	{
		(void)ioctl(fd, TIOCNOTTY, nullptr);
		close(fd);
	}
	
	(void)setsid(); // ignore error?
	
	// Verify that we are successfully disconnected from the controlling tty.
	fd = open("/dev/tty", O_RDWR | O_NOCTTY);
	if (fd >= 0)
	{
		cerr << "Failed to disconnect from controlling tty." << endl;
		close(fd);
	}

	/* Make it our controlling tty. */
	if (ioctl(inTtyFD, TIOCSCTTY, NULL) < 0)
		cerr << "ioctl(TIOCSCTTY): " << strerror(errno) << endl;

	fd = open(mTtyName.c_str(), O_RDWR);
	if (fd < 0)
		cerr << mTtyName << ": " << strerror(errno) << endl;
	else
		close(fd);

	/* Verify that we now have a controlling tty. */
	fd = open("/dev/tty", O_WRONLY);
	if (fd < 0)
		cerr << "open /dev/tty failed - could not set controlling tty: " << strerror(errno) << endl;
	else
		close(fd);
	
	// redirect stdin/stdout/stderr from the pseudo tty
	
	if (dup2(inTtyFD, STDIN_FILENO) < 0)
		cerr << "dup2 stdin: " << strerror(errno) << endl;
	if (dup2(inTtyFD, STDOUT_FILENO) < 0)
		cerr << "dup2 stdout: " << strerror(errno) << endl;
	if (dup2(inTtyFD, STDERR_FILENO) < 0)
		cerr << "dup2 stderr: " << strerror(errno) << endl;
	
	close(inTtyFD);
	
	// 
	struct passwd* pw = getpwuid(getuid());
	if (pw == nullptr)
	{
		cerr << "user not found" << strerror(errno) << endl;
		exit(1);
	}
	
	ifstream motd("/etc/motd");
	while (motd.is_open() and not motd.eof())
	{
		string line;
		getline(motd, line);
		cout << line << endl;
	}
	motd.close();
	
	// force a flush of all buffers
	fflush(nullptr);
	
	string shell = pw->pw_shell;
	if (shell.empty())
		shell = "/bin/sh";
	
	(void)chdir(pw->pw_dir);
	
	// close all other file descriptors
	endpwent();
	closefrom(STDERR_FILENO + 1);
	
	// export TERM 
	setenv("TERM", inTerminalType.c_str(), true);
	
	char* argv[] = { strdup(shell.c_str()), nullptr };
	execvp(shell.c_str(), argv);
	perror("exec failed");
	exit(1);
}

void MPtyTerminalChannel::Close()
{
	mPty.close();
	
	if (mPid > 0)
	{
		int status;
		waitpid(mPid, &status, WNOHANG);
	}
}

bool MPtyTerminalChannel::IsOpen() const
{
	return mPty.is_open();
}

void MPtyTerminalChannel::SendData(const string& inData, const WriteCallback& inCallback)
{
	shared_ptr<boost::asio::streambuf> buffer(new boost::asio::streambuf);
	ostream out(buffer.get());
	out << inData;
	
	boost::asio::async_write(mPty, *buffer,
		[buffer, inCallback, this] (const boost::system::error_code& ec, size_t bytes)
		{
			if (this->mRefCount > 0)
				inCallback(ec, bytes);
		});
}

void MPtyTerminalChannel::SendSignal(const string& inSignal)
{
}

void MPtyTerminalChannel::ReadData(const ReadCallback& inCallback)
{
	boost::asio::async_read(mPty, mResponse,
		boost::asio::transfer_at_least(1),
		[this, inCallback](const boost::system::error_code& ec, size_t inBytesReceived)
		{
			if (this->mRefCount > 0)
				inCallback(ec, this->mResponse);
		});
}

// --------------------------------------------------------------------
// MTerminalChannel factory

MTerminalChannel* MTerminalChannel::Create(std::shared_ptr<pinch::basic_connection> inConnection)
{
	return new MSshTerminalChannel(inConnection);
}

MTerminalChannel* MTerminalChannel::Create(boost::asio::io_service& inIOService)
{
	return new MPtyTerminalChannel(inIOService);
}

