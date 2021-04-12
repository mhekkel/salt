// Copyright Maarten L. Hekkelman 2015
// All rights reserved

#include "MSalt.hpp"

#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pty.h>
#include <pwd.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <fstream>

#include <pinch/terminal_channel.hpp>

#include "MError.hpp"
#include "MPtyTerminalChannel.hpp"
#include "MSaltApp.hpp"
#include "MTerminalChannel.hpp"
#include "MUtils.hpp"

using namespace std;

// --------------------------------------------------------------------

MPtyTerminalChannel::MPtyTerminalChannel()
	: mPid(-1)
	, mPty(gApp->get_io_context())
{
}

MPtyTerminalChannel::~MPtyTerminalChannel()
{
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

void MPtyTerminalChannel::Open(const string &inTerminalType,
	bool inForwardAgent, bool inForwardX11,
	const string &inCommand, const vector<string> &env,
	const OpenCallback &inOpenCallback)
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
		mConnectionInfo = vector<string>({mTtyName});

		mPid = fork();
		switch (mPid)
		{
			case -1:
				throw runtime_error(strerror(errno));

			case 0:
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
	catch (exception &e)
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

void MPtyTerminalChannel::Exec(const string &inCommand, const string &inTerminalType, int inTtyFD)
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
	struct passwd *pw = getpwuid(getuid());
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

	char *argv[] = {strdup(shell.c_str()), nullptr};
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

void MPtyTerminalChannel::SendData(string &&inData)
{
	shared_ptr<boost::asio::streambuf> buffer(new boost::asio::streambuf);
	ostream out(buffer.get());
	out << inData;

	boost::asio::async_write(mPty, *buffer, [buffer](const boost::system::error_code &, std::size_t) {});
}

void MPtyTerminalChannel::SendSignal(const string &inSignal)
{
}

void MPtyTerminalChannel::ReadData(const ReadCallback &inCallback)
{
	MAppExecutor my_executor{&gApp->get_context()};

	auto cb = boost::asio::bind_executor(
		my_executor,
		[this, inCallback](const boost::system::error_code &ec, size_t inBytesReceived) {
			if (this->mRefCount > 0)
				inCallback(ec, this->mResponse);
		});

	boost::asio::async_read(mPty, mResponse, boost::asio::transfer_at_least(1), std::move(cb));
}

// --------------------------------------------------------------------
// MTerminalChannel factory

MTerminalChannel *MTerminalChannel::Create()
{
	return new MPtyTerminalChannel();
}
