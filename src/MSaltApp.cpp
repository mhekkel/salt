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
#include "MUnicode.hpp"
#include "MUtils.hpp"
#include "mrsrc.hpp"
#include "revision.hpp"

#include <pinch.hpp>

#include <mcfp/mcfp.hpp>
#include <zeep/http/uri.hpp>

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

	, cNew(this, "new-terminal", &MSaltApp::OnNew, 'n', kControlKey | kShiftKey)
	, cConnect(this, "connect", &MSaltApp::OnConnect, 's', kControlKey | kShiftKey)
	, cAddNewTOTP(this, "add-totp", &MSaltApp::OnAddNewTOTP)
	, cQuit(this, "quit", &MSaltApp::OnQuit, 'q', kControlKey | kShiftKey)

	, cShowPreferences(this, "preferences", &MSaltApp::OnShowPreferences)

	, cManual(this, "manual", &MSaltApp::OnManual)
	, cAbout(this, "about", &MSaltApp::OnAbout)

	, cSelectTerminal(this, "select-terminal", &MSaltApp::OnSelectTerminal)

	, cClearRecentMenu(this, "clear-recent", &MSaltApp::OnClearRecentMenu)
	, cOpenRecent(this, "open-recent", &MSaltApp::OnOpenRecent)

	, ePreferencesChanged(this, &MSaltApp::OnPreferencesChanged)

	, mIOContext(1)
	, mConnectionPool(mIOContext)
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
	SetIconName("salt");

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

	for (const std::string &known : MPrefs::GetArray("known-hosts"))
	{
		std::smatch m;
		if (std::regex_match(known, m, rx))
			known_hosts.add_host_key(m[1].str(), m[2].str(), m[3].str());
	}

	// set preferred algorithms
	pinch::key_exchange::set_algorithm(pinch::algorithm::encryption, pinch::direction::both,
		MPrefs::GetString("enc", pinch::kEncryptionAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::verification, pinch::direction::both,
		MPrefs::GetString("mac", pinch::kMacAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::compression, pinch::direction::both,
		MPrefs::GetString("cmp", pinch::kCompressionAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::keyexchange, pinch::direction::both,
		MPrefs::GetString("kex", pinch::kKeyExchangeAlgorithms));
	pinch::key_exchange::set_algorithm(pinch::algorithm::serverhostkey, pinch::direction::both,
		MPrefs::GetString("shk", pinch::kServerHostKeyAlgorithms));

	// clang-format off
	mIOContextThread = std::thread(
		[this]
		{
			for (;;)
			{
				try
				{
					auto wg = asio_ns::make_work_guard(mIOContext.get_executor());
					mIOContext.run();
					break;
				}
				catch (const std::exception &ex)
				{
					std::cerr << "Exception in io_context thread: " << ex.what() << '\n';
				}
			}
		});
	// clang-format on

	UpdateRecentSessionMenu();
	UpdatePublicKeyMenu();
	UpdateTOTPMenu();
}

void MSaltApp::OnPreferencesChanged()
{
	// only recent for now

	mRecent.clear();
	for (auto &r : MConnectDialog::GetRecentHosts())
		mRecent.emplace_back(r, mNextRecentNr++);
	UpdateRecentSessionMenu();

}

void MSaltApp::SaveGlobals()
{
	std::vector<std::string> recent_v;
	for (const auto &[r, nr] : mRecent)
		recent_v.emplace_back(r.str());
	MPrefs::SetArray("recent-sessions", recent_v);

	// save new format of known hosts
	std::ofstream known_host_file(gPrefsDir / "known_hosts");
	if (known_host_file.is_open())
		pinch::known_hosts::instance().save_host_file(known_host_file);

	MPrefs::SetArray("known-hosts", {});

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

void MSaltApp::OnAddNewTOTP()
{
	MDialog *d = new MAddTOTPHashDialog();
	d->Select();
}

void MSaltApp::OnQuit()
{
	if (AllowQuit(false))
		DoQuit();
}

void MSaltApp::OnShowPreferences()
{
	auto &dlog = MPreferencesDialog::Instance();
	AddRoute(ePreferencesChanged, dlog.ePreferencesChanged);
	dlog.Select();
}

void MSaltApp::OnManual()
{
	mrsrc::istream manual("salt.1");
	if (manual)
	{
		auto manpage = std::filesystem::temp_directory_path() / "salt.1";
		std::ofstream f(manpage);
		f << manual.rdbuf();
		f.close();

		MWindow *w = MTerminalWindow::Create({ "man", manpage.string() });
		w->Select();
	}
}

void MSaltApp::OnAbout()
{
	DisplayAlert(nullptr, "about-alert", { kVersionNumber, kRevisionGitTag, std::to_string(kBuildNumber), kRevisionDate });
}

void MSaltApp::OnSelectTerminal(int inTerminalNr)
{
	for (auto w = MTerminalWindow::GetFirstTerminal(); w != nullptr; w = w->GetNextTerminal())
	{
		if (w->GetTerminalNr() != static_cast<uint32_t>(inTerminalNr))
			continue;

		w->Select();
		break;
	}
}

void MSaltApp::OnClearRecentMenu()
{
	mRecent.clear();
	UpdateRecentSessionMenu();
	MPrefs::SetArray("recent-sessions", {});
	SaveGlobals();
}

void MSaltApp::OnOpenRecent(int inConnectionNr)
{
	for (const auto &[ci, nr] : mRecent)
	{
		if (static_cast<int>(nr) != inConnectionNr)
			continue;

		Open(ci);
		break;
	}
}

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

	for (uint32_t i = 0; auto &p : MPrefs::GetArray("totp"))
	{
		std::smatch m;
		if (regex_match(p, m, rx))
			items.emplace_back(m[1].str(), i++);
	}
	MMenuBar::Instance().FindMenuByID("totp")->ReplaceItemsInSection(1, "win.enter-totp", items);
}

void MSaltApp::Open(const ConnectInfo &inRecent, const std::string &inCommand)
{
	auto connection =
		inRecent.proxy.has_value() ? mConnectionPool.get(inRecent.user, inRecent.host, inRecent.port,
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

	while (static_cast<int>(mRecent.size()) > MPrefs::GetInteger("recent-count", 10))
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

int MSaltApp::HandleCommandLine(int argc, const char *const argv[])
{
	auto &config = mcfp::config::instance();

	config.init("usage: salt [options] [-- program [args...]]",
		mcfp::make_option<std::string>("connect,c", "Connect to remote host"),
		mcfp::make_option("select-host", "Show connection dialog"));

	std::error_code ec;
	config.parse(argc, argv, ec);
	if (ec)
	{
		std::cerr << ec.message() << '\n';
		return 1;
	}

	if (config.has("connect"))
		Execute("Open", { config.get("connect") });
	else if (config.has("select-host"))
		Execute("Connect", {});
	else if (config.operands().empty())
		Execute("New", {});
	else
		Execute("Execute", config.operands());

	return 0;
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
		ConnectInfo ci;

		auto url = inArguments.front();

		if (zeep::http::is_valid_uri(url))
		{
			zeep::http::uri uri(url);
			if (auto scheme = uri.get_scheme(); not(scheme.empty() or IEquals(scheme, "ssh")))
				return;

			ci.host = uri.get_host();
			ci.port = uri.get_port();
			ci.user = uri.get_userinfo();
		}
		else
			ci.host = url;

		if (ci.user.empty())
			ci.user = GetUserName();

		if (ci.port == 0)
			ci.port = 22;

		std::shared_ptr<pinch::basic_connection> connection = mConnectionPool.get(ci.user, ci.host, ci.port);
		MWindow *w = MTerminalWindow::Create(ci.user, ci.host, ci.port, "", connection);
		w->Select();
	}
	else if (inCommand == "Execute")
	{
		MWindow *w = MTerminalWindow::Create(inArguments);
		w->Select();
	}
}

// --------------------------------------------------------------------

void SetStdinEcho(bool inEnable)
{
	struct termios tty;
	::tcgetattr(STDIN_FILENO, &tty);
	if (not inEnable)
		tty.c_lflag &= ~ECHO;
	else
		tty.c_lflag |= ECHO;

	(void)::tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

bool askYesNo(const std::string &msg, bool defaultYes)
{
	std::string yesno;
	std::cout << msg << (defaultYes ? " [Y/n]: " : " [y/N]: ");
	std::cout.flush();
	std::getline(std::cin, yesno);

	return yesno.empty() ? defaultYes : IEquals(yesno, "y") or IEquals(yesno, "yes");
}

std::string ask(const std::string &msg, std::string defaultAnswer = {})
{
	std::cout << msg << " [" << defaultAnswer << "]: ";
	std::cout.flush();

	std::string answer;
	std::getline(std::cin, answer);
	return answer.empty() ? defaultAnswer : answer;
}

void Install(const std::string &inPrefix)
{
	std::error_code ec;
	if (auto exefile = fs::read_symlink("/proc/self/exe", ec); not ec and fs::exists(exefile, ec))
		gExecutablePath = exefile;

	fs::path prefix(inPrefix);
	if (prefix.empty())
	{
		if (getuid() == 0)
			prefix = "/usr/local";
		else if (auto path = getenv("PATH"); path != nullptr)
		{
			std::cmatch m;
			std::regex rx(R"((?!<:)/home/[^/:]+/\.local/bin(?!=:))");
			if (std::regex_search(path, m, rx))
			{
				prefix = m[0].str();
				prefix = prefix.parent_path();
			}
		}

		// --------------------------------------------------------------------
		// Ask if this is ok

		std::cout << "No prefix was specied, where do you want to install salt?\n";

		prefix = ask("prefix path", prefix);
	}

	if (prefix.empty() or not fs::exists(prefix, ec))
	{
		std::cout << "Not a valid location, exiting\n";
		exit(1);
	}

	fs::path bindir = prefix / "bin";
	fs::path datadir = prefix / "share";
	fs::path icondir = datadir / "icons" / "hicolor" / "48x48" / "apps";
	fs::path appdir = datadir / "applications";
	fs::path mandir = datadir / "man" / "man1";

	// Create directories
	for (auto &p : { bindir, datadir, icondir, appdir })
	{
		if (not fs::exists(p, ec))
		{
			std::cout << "Creating directory " << (p) << '\n';
			fs::create_directories(p, ec);

			if (ec)
			{
				std::cout << "Error creating directory " << p << ": " << ec.message() << "\n";
				exit(1);
			}
		}
	}

	// copy executable

	std::cout << "copying " << gExecutablePath.string() << " to " << bindir << '\n';

	if (fs::exists(bindir / "salt", ec))
		fs::remove(bindir / "salt", ec);

	if (ec)
	{
		std::cout << "Removing old executable failed: " << ec.message() << "\n";
		exit(1);
	}

	fs::copy_file(gExecutablePath, bindir / "salt", ec);

	if (ec)
	{
		std::cout << "Copying executable failed: " << ec.message() << "\n";
		exit(1);
	}

	// copy icon

	if (fs::exists(icondir / "salt.png", ec))
		fs::remove(icondir / "salt.png", ec);

	if (ec)
	{
		std::cout << "Removing old icon failed: " << ec.message() << "\n";
		exit(1);
	}

	mrsrc::rsrc icon("Icons/appicon.png");
	if (not icon)
	{
		std::cout << "Icon resource is missing\n";
		exit(1);
	}

	std::cout << "writing icon file " << (icondir / "salt.png") << '\n';

	std::ofstream file;
	file.open((icondir / "salt.png"), std::ios::trunc | std::ios::binary);
	file.write(icon.data(), icon.size());
	if (file.bad())
	{
		std::cout << "Writing icon failed\n";
		exit(1);
	}
	file.close();

	// Copy man page

	mrsrc::rsrc manpage("salt.1");
	if (not manpage)
	{
		std::cout << "Manual page is missing\n";
		exit(1);
	}

	std::cout << "writing manual page " << (mandir / "salt.1") << '\n';

	file.open((mandir / "salt.1"), std::ios::trunc | std::ios::binary);
	file.write(manpage.data(), manpage.size());
	if (file.bad())
	{
		std::cout << "Writing manpage failed\n";
		exit(1);
	}
	file.close();

	// create desktop file
	mrsrc::rsrc rsrc("salt.desktop.in");
	if (not rsrc)
	{
		std::cout << "Missing destkop resource\n";
		exit(1);
	}

	std::string desktop(rsrc.data(), rsrc.size());
	ReplaceAll(desktop, "@__EXE__@", (bindir / "salt").string());
	ReplaceAll(desktop, "@__ICON__@", (icondir / "salt.png").string());

	fs::path desktopFile = appdir / "salt.desktop";
	std::cout << "writing desktop file " << desktopFile << '\n';

	file.open(desktopFile, std::ios::trunc);
	file << desktop;
	file.close();

	exit(0);
}

// --------------------------------------------------------------------

int main(int argc, char *const argv[])
{
#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
	int err_fd = open(("/tmp/salt-debug-" + std::to_string(getpid()) + ".log").c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
	if (err_fd > 0)
		dup2(err_fd, STDERR_FILENO);
#endif

	auto &config = mcfp::config::instance();

	config.init("usage: salt [options] [-- program [args...]]",
		mcfp::make_option("help,h", "Display this message"),
		mcfp::make_option("version", "Show version number"),
		mcfp::make_option("verbose", "More verbose"),
		mcfp::make_option<std::string>("connect,c", "Connect to remote host"),
		mcfp::make_option("select-host", "Show connection dialog"),
		mcfp::make_option("install,i", "Install the application"),
		mcfp::make_option<std::string>("prefix,p", "Installation prefix"));

	// for now
	config.set_ignore_unknown(true);

	std::error_code ec;
	config.parse(argc, argv, ec);
	if (ec)
	{
		std::cerr << ec.message() << '\n';
		exit(1);
	}

	if (config.has("help"))
	{
		std::cerr << config << '\n';
		exit(0);
	}

	if (config.has("version"))
	{
		write_version_string(std::cout, config.has("verbose"));
		exit(0);
	}

	if (config.has("install"))
	{
		Install(config.has("prefix") ? config.get("prefix") : "");
		exit(0);
	}

	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i)
		args.emplace_back(argv[i]);

	return MSaltApp::Main("com.hekkelman.salt", args);
}