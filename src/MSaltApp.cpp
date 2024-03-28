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

// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSaltApp.hpp"
#include "MAddTOTPHashDialog.hpp"
#include "MAlerts.hpp"
#include "MConnectDialog.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MPreferences.hpp"
#include "MPreferencesDialog.hpp"
#include "MTerminalWindow.hpp"
#include "MUtils.hpp"
#include "mrsrc.hpp"
#include "revision.hpp"

#include <pinch.hpp>

#include <mcfp/mcfp.hpp>
#include <zeep/crypto.hpp>
#include <zeep/unicode-support.hpp>

#include <filesystem>
#include <fstream>
#include <regex>

#if defined(_MSC_VER)
# pragma comment(lib, "libpinch")
# pragma comment(lib, "libz")
#endif

namespace fs = std::filesystem;

const char
	kAppName[] = "salt";

namespace
{
#define USER "(?:([-$_.+!*'(),[:alnum:];?&=]+)@)?"
#define HOST "([-[:alnum:].]+)"
#define PORT "(?::(\\d+))?"

std::regex kRecentRE("^" USER HOST PORT "(?:;" USER HOST PORT ";(.+)"
					 ")?(?: >> (.+))?$");
} // namespace

// --------------------------------------------------------------------

MSaltApp::MSaltApp(MApplicationImpl *inImpl)
	: MApplication(inImpl)
	, mIOContext(1)
	, mConnectionPool(mIOContext)

	, cNew(this, "new", &MSaltApp::OnNew, 'n', kControlKey | kShiftKey)
	, cConnect(this, "connect", &MSaltApp::OnConnect, 's', kControlKey | kShiftKey)
	, cQuit(this, "quit", &MSaltApp::OnQuit, 'q', kControlKey | kShiftKey)
	, cAbout(this, "about", &MSaltApp::OnAbout)
	, cSelectTerminal(this, "select-terminal", &MSaltApp::OnSelectTerminal)

	, cClearRecentMenu(this, "clear-recent", &MSaltApp::OnClearRecentMenu)
	, cOpenRecent(this, "open-recent", &MSaltApp::OnOpenRecent)
{
}

MSaltApp::~MSaltApp()
{
	if (not mIOContext.stopped())
		mIOContext.stop();
	if (mIOContextThread.joinable())
		mIOContextThread.join();
}

void MSaltApp::Initialise()
{
	MApplication::Initialise();
	MMenuBar::Init("terminal-window-menu");

#if defined _MSC_VER
	if (Preferences::GetBoolean("act-as-pageant", true))
		pinch::ssh_agent::instance().expose_pageant(true);
#endif

	// recent menu
	for (auto &r : MConnectDialog::GetRecentHosts())
		mRecent.emplace_back(r, mNextRecentNr++);
	UpdateRecentSessionMenu();

	// known hosts
	auto &known_hosts = pinch::known_hosts::instance();

	if (fs::exists(gPrefsDir / "known_hosts"))
	{
		std::ifstream host_file(gPrefsDir / "known_hosts");
		if (host_file.is_open())
			known_hosts.load_host_file(host_file);
	}

	std::regex rx("^(\\S+) (\\S+) (.+)");

	for (const std::string &known : Preferences::GetArray("known-hosts"))
	{
		std::smatch m;
		if (std::regex_match(known, m, rx))
			known_hosts.add_host_key(m[1].str(), m[2].str(), m[3].str());
	}

	// set preferred algorithms
	pinch::key_exchange::set_algorithm(pinch::algorithm::encryption, pinch::direction::both,
		Preferences::GetString("enc", pinch::kEncryptionAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::verification, pinch::direction::both,
		Preferences::GetString("mac", pinch::kMacAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::compression, pinch::direction::both,
		Preferences::GetString("cmp", pinch::kCompressionAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::keyexchange, pinch::direction::both,
		Preferences::GetString("kex", pinch::kKeyExchangeAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::serverhostkey, pinch::direction::both,
		Preferences::GetString("shk", pinch::kServerHostKeyAlgorithms));

	mIOContextThread = std::thread([this]()
		{
		try
		{
			auto wg = asio_ns::make_work_guard(mIOContext.get_executor());
			mIOContext.run();
		}
		catch (const std::exception &ex)
		{
			std::cerr << "Exception in io_context thread: " << ex.what() << '\n';
		} });

	UpdateRecentSessionMenu();
	UpdatePublicKeyMenu();
	UpdateTOTPMenu();
}

void MSaltApp::SaveGlobals()
{
	std::vector<std::string> recent_v;
	for (const auto &[r, nr] : mRecent)
		recent_v.emplace_back(r.str());
	Preferences::SetArray("recent-sessions", recent_v);

	// save new format of known hosts
	std::ofstream known_host_file(gPrefsDir / "known_hosts");
	if (known_host_file.is_open())
		pinch::known_hosts::instance().save_host_file(known_host_file);

	Preferences::SetArray("known-hosts", {});

	MApplication::SaveGlobals();
}

MApplication *MApplication::Create(MApplicationImpl *inImpl)
{
	return new MSaltApp(inImpl);
}

// --------------------------------------------------------------------

void MSaltApp::OnNew()
{
	DoNew();
}

void MSaltApp::OnConnect()
{
	MDialog *d = new MConnectDialog();
	d->Select();
}

void MSaltApp::OnQuit()
{
	if (AllowQuit(false))
		DoQuit();
}

void MSaltApp::OnAbout()
{
	DisplayAlert(nullptr, "about-alert", { kVersionNumber, kRevisionGitTag, std::to_string(kBuildNumber), kRevisionDate });
}

void MSaltApp::OnSelectTerminal(int inTerminalNr)
{
	for (auto w = MTerminalWindow::GetFirstTerminal(); w != nullptr; w = w->GetNextTerminal())
	{
		if (w->GetTerminalNr() != inTerminalNr)
			continue;

		w->Select();
		break;
	}
}

void MSaltApp::OnClearRecentMenu()
{
	mRecent.clear();
	UpdateRecentSessionMenu();
	Preferences::SetArray("recent-sessions", {});
	SaveGlobals();
}

void MSaltApp::OnOpenRecent(int inConnectionNr)
{
	for (const auto &[ci, nr] : mRecent)
	{
		if (nr != inConnectionNr)
			continue;
		
		Open(ci);
		break;
	}
}

// bool MSaltApp::ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers)
// {
// 	bool result = true;

// 	switch (inCommand)
// 	{
// 		case cmd_Connect:
// 		{
// 			break;
// 		}

// 		case cmd_About:
// 			DoAbout();
// 			break;

// 		case cmd_OpenRecentSession:
// 			if (inItemIndex - 2 < mRecent.size())
// 				Open(*(mRecent.begin() + inItemIndex - 2));
// 			break;

// 		case cmd_ClearRecentSessions:
// 			mRecent.clear();
// 			break;

// 		case cmd_SelectWindowFromMenu:
// 		{
// 			MTerminalWindow *term = MTerminalWindow::GetFirstTerminal();
// 			while (inItemIndex-- > 3 and term != nullptr)
// 				term = term->GetNextTerminal();
// 			if (term != nullptr)
// 				term->Select();
// 			break;
// 		}

// 		case cmd_Preferences:
// 			MPreferencesDialog::Instance().Select();
// 			break;

// 		case cmd_AddNewTOTPHash:
// 		{
// 			MDialog *d = new MAddTOTPHashDialog();
// 			d->Select();
// 			break;
// 		}

// 			// #if DEBUG
// 			//		case 'test':
// 			//			new MTestWindow();
// 			//			break;
// 			// #endif

// 		default:
// 			result = MApplication::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
// 			break;
// 	}

// 	return result;
// }

void MSaltApp::UpdateWindowMenu()
{
	auto m = MMenuBar::Instance().FindMenuByID("window");
	assert(m);

	std::vector<std::tuple<std::string, uint32_t>> labels;
	for (auto w = MTerminalWindow::GetFirstTerminal(); w != nullptr; w = w->GetNextTerminal())
		labels.emplace_back(w->GetTitle(), w->GetTerminalNr());

	m->ReplaceItemsInSection(1, "app.select-terminal", labels);

	if (auto w = dynamic_cast<MTerminalWindow *>(GetActiveWindow()); w != nullptr)
		cSelectTerminal.SetState(w->GetTerminalNr());
}

void MSaltApp::UpdateRecentSessionMenu()
{
	std::vector<std::tuple<std::string, uint32_t>> items;
	for (const auto &[ci, nr] : mRecent)
		items.emplace_back(ci.str(), nr);
	MMenuBar::Instance().FindMenuByID("recent")->ReplaceItemsInSection(1, "app.open-recent", items);
}

void MSaltApp::UpdatePublicKeyMenu()
{
	std::vector<std::tuple<std::string, uint32_t>> items;
	pinch::ssh_agent &agent(pinch::ssh_agent::instance());
	for (uint32_t i = 0; auto &key : agent)
		items.emplace_back(key.get_comment(), i++);
	MMenuBar::Instance().FindMenuByID("public-keys")->ReplaceItemsInSection(0, "win.install-public-key", items);
}

void MSaltApp::UpdateTOTPMenu()
{
	std::vector<std::tuple<std::string, uint32_t>> items;
	const std::regex rx("(.+);[A-Z2-7]+");

	for (uint32_t i = 0; auto &p : Preferences::GetArray("totp"))
	{
		std::smatch m;
		if (regex_match(p, m, rx))
			items.emplace_back(m[1].str(), i++);
	}
	MMenuBar::Instance().FindMenuByID("totp")->ReplaceItemsInSection(1, "win.enter-totp", items);
}

void MSaltApp::Open(const ConnectInfo &inRecent, const std::string &inCommand)
{
	auto connection = inRecent.proxy.has_value() ? mConnectionPool.get(inRecent.user, inRecent.host, inRecent.port,
													   inRecent.proxy->user, inRecent.proxy->host, inRecent.proxy->port, inRecent.proxy->command)
	                                             : mConnectionPool.get(inRecent.user, inRecent.host, inRecent.port);

	auto w = MTerminalWindow::Create(inRecent.user, inRecent.host, inRecent.port, inCommand, connection);
	w->Select();

	mRecent.emplace_front(inRecent, mNextRecentNr++);
	for (auto i = mRecent.begin() + 1; i != mRecent.end(); ++i)
	{
		if (i->first != inRecent)
			continue;

		mRecent.erase(i);
		break;
	}

	if (mRecent.size() > 10)
		mRecent.pop_back();

	UpdateRecentSessionMenu();
	SaveGlobals();
}

bool MSaltApp::AllowQuit(bool inLogOff)
{
	if (mConnectionPool.has_open_channels() == false and MTerminalWindow::IsAnyTerminalOpen() == false)
		return true;

	DisplayAlert(nullptr, "close-all-sessions-alert", [this](int result)
		{ if (result == 1) DoQuit(); },
		{});

	return false;
}

void MSaltApp::DoQuit()
{
	mQuit = true;
	mQuitPending = true;

	mConnectionPool.disconnect_all();

	MApplication::DoQuit();
}

void MSaltApp::DoNew()
{
	MWindow *w = MTerminalWindow::Create({});
	w->Select();
}

void MSaltApp::Execute(const std::string &inCommand,
	const std::vector<std::string> &inArguments)
{
	if (inCommand == "New")
		OnNew();
	else if (inCommand == "Connect")
		OnConnect();
	else if (inCommand == "Open")
	{
		std::regex re("^(?:ssh://)?(?:([-$_.+!*'(),[:alnum:];?&=]+)(?::([-$_.+!*'(),[:alnum:];?&=]+))?@)?([-[:alnum:].]+)(?::(\\d+))?$");
		std::smatch m;

		if (std::regex_match(inArguments.front(), m, re))
		{
			std::string user = m[1];
			std::string host = m[3];
			uint16_t port = m[4].matched ? std::stoi(m[4]) : 22;
			std::string command;

			std::shared_ptr<pinch::basic_connection> connection = mConnectionPool.get(user, host, port);
			MWindow *w = MTerminalWindow::Create(user, host, port, command, connection);
			w->Select();
		}
	}
	else if (inCommand == "Execute")
	{
		MWindow *w = MTerminalWindow::Create(inArguments);
		w->Select();
	}
}

// #if not defined(_MSC_VER)

// # include <gtk/gtk.h>

// void MApplication::Install(const std::string &inPrefix)
// {
// 	if (getuid() != 0)
// 		throw std::runtime_error("You must be root to be able to install salt");

// 	if (not fs::exists(gExecutablePath))
// 		throw std::runtime_error(std::string("I don't seem to exist...?") + gExecutablePath.string());

// 	fs::path prefix(inPrefix);
// 	if (prefix.empty())
// 		prefix = "/usr/local";

// 	// copy the executable to the appropriate destination
// 	if (not fs::exists(prefix / "bin"))
// 	{
// 		std::cout << "Creating directory " << (prefix / "bin") << '\n';
// 		fs::create_directories(prefix / "bin");
// 	}
// 	fs::path exeDest = prefix / "bin" / "salt";

// 	if (not fs::exists(prefix / "share" / "salt"))
// 	{
// 		std::cout << "Creating directory " << (prefix / "share" / "salt") << '\n';
// 		fs::create_directories(prefix / "share" / "salt");
// 	}
// 	fs::path iconDest = prefix / "share" / "salt" / "icon.png";

// 	std::cout << "copying " << gExecutablePath.string() << " to " << exeDest.string() << '\n';

// 	if (fs::exists(exeDest))
// 		fs::remove(exeDest);

// 	fs::copy_file(gExecutablePath, exeDest);

// 	if (fs::exists(iconDest))
// 		fs::remove(iconDest);

// 	// create desktop file
// 	mrsrc::rsrc rsrc("salt.desktop");
// 	mrsrc::rsrc icon("Icons/appicon.png");

// 	if (not rsrc)
// 		throw std::runtime_error("salt.desktop file could not be created, missing data");

// 	std::string desktop(rsrc.data(), rsrc.size());
// 	zeep::replace_all(desktop, "__EXE__", exeDest.string());
// 	zeep::replace_all(desktop, "__ICON__", iconDest.string());

// 	// locate applications directory
// 	// don't use glib here,

// 	fs::path desktopFile, applicationsDir;

// 	const char *const *config_dirs = g_get_system_data_dirs();
// 	for (const char *const *dir = config_dirs; *dir != nullptr; ++dir)
// 	{
// 		applicationsDir = fs::path(*dir) / "applications";
// 		if (fs::exists(applicationsDir) and fs::is_directory(applicationsDir))
// 			break;
// 	}

// 	if (not fs::exists(applicationsDir))
// 	{
// 		std::cout << "Creating directory " << applicationsDir << '\n';
// 		fs::create_directories(applicationsDir);
// 	}

// 	std::cout << "writing icon file " << iconDest << '\n';

// 	desktopFile = applicationsDir / "salt.desktop";
// 	std::cout << "writing desktop file " << desktopFile << '\n';

// 	std::ofstream file(desktopFile, std::ios::trunc);
// 	file << desktop;
// 	file.close();

// 	std::cout << "writing icon file " << iconDest << '\n';
// 	file.open(iconDest, std::ios::trunc | std::ios::binary);
// 	file.write(icon.data(), icon.size());
// 	file.close();

// 	exit(0);
// }

// #endif

// --------------------------------------------------------------------

int main(int argc, char *const argv[])
{
	// // #if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
	// // 	int err_fd = open(("/tmp/salt-debug-" + std::to_string(getpid()) + ".log").c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
	// // 	if (err_fd > 0)
	// // 		dup2(err_fd, STDERR_FILENO);
	// // #endif

	// auto &config = mcfp::config::instance();

	// config.init("usage: salt [options] [-- program [args...]]",
	// 	mcfp::make_option("help,h", "Display this message"),
	// 	mcfp::make_option("version", "Show version number"),
	// 	mcfp::make_option("verbose", "More verbose"),
	// 	mcfp::make_option<std::string>("connect,c", "Connect to remote host"),
	// 	mcfp::make_option("select-host", "Show connection dialog"),
	// 	mcfp::make_option("install,i", "Install the application"),
	// 	mcfp::make_option<std::string>("prefix,p", "/usr/local", "Installation prefix"));

	// // for now
	// config.set_ignore_unknown(true);

	// std::error_code ec;
	// config.parse(argc, argv, ec);
	// if (ec)
	// {
	// 	std::cerr << ec.message() << '\n';
	// 	exit(1);
	// }

	// if (config.has("help"))
	// {
	// 	std::cerr << config << '\n';
	// 	exit(0);
	// }

	// if (config.has("version"))
	// {
	// 	write_version_string(std::cout, config.has("verbose"));
	// 	exit(0);
	// }

	// if (config.has("install"))
	// {
	// 	gExecutablePath = fs::canonical(argv[0]);

	// 	std::string prefix = config.get<std::string>("prefix");
	// 	MApplication::Install(prefix);
	// 	exit(0);
	// }

	// std::string command;
	// std::vector<std::string> args;

	// if (config.has("select-host"))
	// 	command = "Connect";
	// else if (config.has("connect"))
	// {
	// 	command = "Open";
	// 	args.emplace_back(config.get("connect"));
	// }
	// else if (config.operands().empty())
	// 	command = "New";
	// else
	// {
	// 	command = "Execute";
	// 	args = config.operands();
	// }

	// return MApplication::Main(command, args);

	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i)
		args.emplace_back(argv[i]);

	return MSaltApp::Main(args);
}