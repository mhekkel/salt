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

#include "MPtyTerminalChannel.hpp"
#include "MError.hpp"
#include "MSaltApp.hpp"
#include "MTerminalChannel.hpp"
#include "MUtils.hpp"

#include <fstream>

#include <pinch.hpp>

#include <pwd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pty.h>
#include <pwd.h>
#include <stdarg.h>
#include <termios.h>
#include <unistd.h>

// --------------------------------------------------------------------

MPtyTerminalChannel::MPtyTerminalChannel(MTerminalChannel *inCloneFrom)
	: mPid(-1)
	, mPty(MSaltApp::Instance().get_io_context())
{
	if (auto c = dynamic_cast<MPtyTerminalChannel *>(inCloneFrom))
		SetCWD(c->GetCWD());
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

void MPtyTerminalChannel::Open(const std::string &inTerminalType,
	bool inForwardAgent, bool inForwardX11,
	const std::vector<std::string> &inArgv, const std::vector<std::string> &env,
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
			throw std::runtime_error(strerror(errno));

		mTtyName = ttyname(ttyfd);
		mConnectionInfo = std::vector<std::string>({ mTtyName });

		mPid = fork();
		switch (mPid)
		{
			case -1:
				throw std::runtime_error(strerror(errno));

			case 0:
				close(ptyfd);
				Execute(inArgv, inTerminalType, ttyfd);
				// does not return

			default:
				break;
		}

		close(ttyfd);
		mPty.assign(ptyfd);

		inOpenCallback(std::error_code());
	}
	catch (const std::exception &e)
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

void MPtyTerminalChannel::Execute(const std::vector<std::string> &inArgv, const std::string &inTerminalType, int inTtyFD)
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
		std::cerr << "Failed to disconnect from controlling tty.\n";
		close(fd);
	}

	/* Make it our controlling tty. */
	if (ioctl(inTtyFD, TIOCSCTTY, nullptr) < 0)
		std::cerr << "ioctl(TIOCSCTTY): " << strerror(errno) << '\n';

	fd = open(mTtyName.c_str(), O_RDWR);
	if (fd < 0)
		std::cerr << mTtyName << ": " << strerror(errno) << '\n';
	else
		close(fd);

	/* Verify that we now have a controlling tty. */
	fd = open("/dev/tty", O_WRONLY);
	if (fd < 0)
		std::cerr << "open /dev/tty failed - could not set controlling tty: " << strerror(errno) << '\n';
	else
		close(fd);

	// redirect stdin/stdout/stderr from the pseudo tty

	if (dup2(inTtyFD, STDIN_FILENO) < 0)
		std::cerr << "dup2 stdin: " << strerror(errno) << '\n';
	if (dup2(inTtyFD, STDOUT_FILENO) < 0)
		std::cerr << "dup2 stdout: " << strerror(errno) << '\n';
	if (dup2(inTtyFD, STDERR_FILENO) < 0)
		std::cerr << "dup2 stderr: " << strerror(errno) << '\n';

	close(inTtyFD);

	//
	struct passwd *pw = getpwuid(getuid());
	if (pw == nullptr)
	{
		std::cerr << "user not found" << strerror(errno) << '\n';
		exit(1);
	}

	std::ifstream motd("/etc/motd");
	while (motd.is_open() and not motd.eof())
	{
		std::string line;
		std::getline(motd, line);
		std::cout << line << '\n';
	}
	motd.close();

	// force a flush of all buffers
	fflush(nullptr);

	std::string shell = pw->pw_shell;
	if (shell.empty())
		shell = "/bin/sh";

	std::error_code ec;
	if (mCWD.empty())
		std::filesystem::current_path(pw->pw_dir, ec);
	else
		std::filesystem::current_path(mCWD, ec);

	// close all other file descriptors
	endpwent();
	closefrom(STDERR_FILENO + 1);

	// export TERM
	setenv("TERM", inTerminalType.c_str(), true);

	// char *argv[] = { strdup(shell.c_str()), nullptr };

	std::vector<char *> argv;

	if (inArgv.empty())
		argv.push_back(const_cast<char *>(shell.c_str()));

	for (auto &a : inArgv)
		argv.push_back(const_cast<char *>(a.c_str()));

	argv.push_back(nullptr);

	execvp(argv.front(), argv.data());
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

std::filesystem::path MPtyTerminalChannel::GetCWD() const
{
	std::filesystem::path result;

	if (mPid > 0)
	{
		std::error_code ec;

		std::filesystem::path cwd = std::filesystem::path("/proc") / std::to_string(mPid) / "cwd";

		if (std::filesystem::exists(cwd, ec))
			result = std::filesystem::read_symlink(cwd, ec);
	}

	return result;
}

void MPtyTerminalChannel::SendData(std::string &&inData)
{
	std::shared_ptr<asio_ns::streambuf> buffer(new asio_ns::streambuf);
	std::ostream out(buffer.get());
	out << inData;

	asio_ns::async_write(mPty, *buffer, [buffer](const std::error_code &, std::size_t) {});
}

void MPtyTerminalChannel::SendSignal(const std::string &inSignal)
{
	if (inSignal == "STOP")
		killpg(mPid, SIGSTOP);
	else if (inSignal == "CONT")
		killpg(mPid, SIGCONT);
	else if (inSignal == "INT")
		killpg(mPid, SIGINT);
	else if (inSignal == "HUP")
		killpg(mPid, SIGHUP);
	else if (inSignal == "TERM")
		killpg(mPid, SIGTERM);
	else if (inSignal == "KILL")
		killpg(mPid, SIGKILL);
}

void MPtyTerminalChannel::ReadData(const ReadCallback &inCallback)
{
	MAppExecutor my_executor{ &MSaltApp::Instance().get_context() };

	auto cb = asio_ns::bind_executor(
		my_executor,
		[this, inCallback](const std::error_code &ec, std::size_t inBytesReceived)
		{
			if (this->mRefCount > 0)
				inCallback(ec, this->mResponse);
		});

	asio_ns::async_read(mPty, mResponse, asio_ns::transfer_at_least(1), std::move(cb));
}

// --------------------------------------------------------------------
// MTerminalChannel factory

MTerminalChannel *MTerminalChannel::Create(MTerminalChannel *inCloneFrom)
{
	return new MPtyTerminalChannel(inCloneFrom);
}
