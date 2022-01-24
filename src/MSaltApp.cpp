// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <filesystem>
#include <fstream>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
// #include <boost/algorithm/string/regex.hpp>

#include <pinch/ssh_agent.hpp>

#include <zeep/crypto.hpp>

#include "MAcceleratorTable.hpp"
#include "MApplicationImpl.hpp"
#include "MConnectDialog.hpp"
#include "MMenu.hpp"
#include "MPreferences.hpp"
#include "MPreferencesDialog.hpp"
#include "MSaltApp.hpp"
#include "MTerminalWindow.hpp"
// #include "MSaltVersion.hpp"
#include "MAddTOTPHashDialog.hpp"
#include "MAlerts.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MUtils.hpp"
#include "mrsrc.hpp"

#if defined(_MSC_VER)
#pragma comment(lib, "libpinch")
#pragma comment(lib, "libz")
#endif

using namespace std;
namespace ba = boost::algorithm;
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
	, mConnectionPool(inImpl->mIOContext)
{
	MAcceleratorTable &at = MAcceleratorTable::Instance();

	at.RegisterAcceleratorKey(cmd_New, 'N', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Connect, 'S', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Close, 'W', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Quit, 'Q', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Cut, 'X', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Copy, 'C', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Paste, 'V', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_SelectAll, 'A', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_CloneTerminal, 'D', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_Reset, 'R', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_NextTerminal, kTabKeyCode, kControlKey);
	at.RegisterAcceleratorKey(cmd_PrevTerminal, kTabKeyCode, kControlKey | kShiftKey);

	at.RegisterAcceleratorKey(cmd_Find, 'F', kControlKey | kShiftKey);
	at.RegisterAcceleratorKey(cmd_FindNext, kF3KeyCode, kControlKey);
	at.RegisterAcceleratorKey(cmd_FindPrev, kF3KeyCode, kControlKey | kShiftKey);
}

MSaltApp::~MSaltApp()
{
}

void MSaltApp::Initialise()
{
	MApplication::Initialise();

	if (Preferences::GetBoolean("act-as-pageant", true))
		pinch::ssh_agent::instance().expose_pageant(true);

	// recent menu
	vector<string> recent;
	Preferences::GetArray("recent-sessions", recent);
	for (const string &r : recent)
	{
		if (std::regex_match(r, kRecentRE))
			mRecent.push_back(r);
	}

	// known hosts
	auto &known_hosts = pinch::known_hosts::instance();

	if (fs::exists(gPrefsDir / "known_hosts"))
	{
		std::ifstream host_file(gPrefsDir / "known_hosts");
		if (host_file.is_open())
			known_hosts.load_host_file(host_file);
	}

	vector<string> knownHosts;
	Preferences::GetArray("known-hosts", knownHosts);

	std::regex rx("^(\\S+) (\\S+) (.+)");

	for (const string &known : knownHosts)
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
}

void MSaltApp::SaveGlobals()
{
	vector<string> recent(mRecent.begin(), mRecent.end());
	Preferences::SetArray("recent-sessions", recent);

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

bool MSaltApp::ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_Connect:
		{
			MDialog *d = new MConnectDialog();
			d->Select();
			break;
		}

		case cmd_About:
			DoAbout();
			break;

		case cmd_OpenRecentSession:
			if (inItemIndex - 2 < mRecent.size())
				OpenRecent(*(mRecent.begin() + inItemIndex - 2));
			break;

		case cmd_ClearRecentSessions:
			mRecent.clear();
			break;

		case cmd_SelectWindowFromMenu:
		{
			MTerminalWindow *term = MTerminalWindow::GetFirstTerminal();
			while (inItemIndex-- > 3 and term != nullptr)
				term = term->GetNextTerminal();
			if (term != nullptr)
				term->Select();
			break;
		}

		case cmd_Preferences:
			MPreferencesDialog::Instance().Select();
			break;

		case cmd_AddNewTOTPHash:
		{
			MDialog *d = new MAddTOTPHashDialog();
			d->Select();
			break;
		}

			//#if DEBUG
			//		case 'test':
			//			new MTestWindow();
			//			break;
			//#endif

		default:
			result = MApplication::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}

	return result;
}

bool MSaltApp::UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_Connect:
		case cmd_Preferences:
		case cmd_About:
		case cmd_Find:
		case cmd_OpenRecentSession:
		case 'test':
		case cmd_AddNewTOTPHash:
			outEnabled = true;
			break;

		case cmd_ClearRecentSessions:
			outEnabled = not mRecent.empty();
			break;

		default:
			result = MApplication::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
			break;
	}

	return result;
}

void MSaltApp::UpdateSpecialMenu(const std::string &inName, MMenu *inMenu)
{
	// PRINT(("UpdateSpecialMenu %s", inName.c_str()));

	if (inName == "window")
		UpdateWindowMenu(inMenu);
	else if (inName == "recent-session")
		UpdateRecentSessionMenu(inMenu);
	else if (inName == "public-keys")
		UpdatePublicKeyMenu(inMenu);
	else if (inName == "totps")
		UpdateTOTPMenu(inMenu);
	else
		MApplication::UpdateSpecialMenu(inName, inMenu);
}

void MSaltApp::UpdateWindowMenu(MMenu *inMenu)
{
	inMenu->RemoveItems(3, inMenu->CountItems() - 3);

	MTerminalWindow *term = MTerminalWindow::GetFirstTerminal();
	while (term != nullptr)
	{
		string label = term->GetTitle();
		inMenu->AppendItem(label, cmd_SelectWindowFromMenu);
		term = term->GetNextTerminal();
	}
}

void MSaltApp::UpdateRecentSessionMenu(MMenu *inMenu)
{
	using namespace std::literals;

	inMenu->RemoveItems(2, inMenu->CountItems() - 2);

	for (const string &recent : mRecent)
	{
		std::smatch m;

		if (std::regex_match(recent, m, kRecentRE))
		{
			inMenu->AppendItem(m[1].str() + "@"s + m[2].str() +
								   (m[3].matched ? (":"s + m[3].str()) : ""),
				cmd_OpenRecentSession);
		}
	}
}

void MSaltApp::UpdatePublicKeyMenu(MMenu *inMenu)
{
	inMenu->RemoveItems(0, inMenu->CountItems());

	pinch::ssh_agent &agent(pinch::ssh_agent::instance());
	for (auto key = agent.begin(); key != agent.end(); ++key)
		inMenu->AppendItem(key->get_comment(), cmd_DropPublicKey);
}

void MSaltApp::UpdateTOTPMenu(MMenu *inMenu)
{
	inMenu->RemoveItems(2, inMenu->CountItems() - 2);

	vector<string> totp;
	Preferences::GetArray("totp", totp);
	const regex rx("(.+);[A-Z2-7]+");

	for (auto p : totp)
	{
		smatch m;
		if (regex_match(p, m, rx))
			inMenu->AppendItem(m[1].str(), cmd_EnterTOTP);
	}
}

void MSaltApp::AddRecent(const string &inRecent)
{
	if (std::regex_match(inRecent, kRecentRE))
	{
		mRecent.erase(remove(mRecent.begin(), mRecent.end(), inRecent), mRecent.end());
		mRecent.push_front(inRecent);
		if (mRecent.size() > 10)
			mRecent.pop_back();

		vector<string> recent_v(mRecent.begin(), mRecent.end());
		Preferences::SetArray("recent-sessions", recent_v);
	}
}

void MSaltApp::OpenRecent(const string &inRecent)
{
	std::smatch m;

	if (std::regex_match(inRecent, m, kRecentRE))
	{
		string user = m[1];
		string host = m[2];
		uint16_t port = m[3].matched ? std::stoi(m[3]) : 22;
		string command = m[6];

		MWindow *w;

		if (m[5].matched)
		{
			string proxy_user = m[4];
			string proxy_host = m[5];
			uint16_t proxy_port = m[6].matched ? std::stoi(m[6]) : 22;
			string proxy_cmd = m[7];

			std::shared_ptr<pinch::basic_connection> connection = mConnectionPool.get(
				user, host, port, proxy_user, proxy_host, proxy_port, proxy_cmd);
			w = MTerminalWindow::Create(host, user, port, command, connection);
		}
		else
		{
			std::shared_ptr<pinch::basic_connection> connection = mConnectionPool.get(user, host, port);
			w = MTerminalWindow::Create(host, user, port, command, connection);
		}

		w->Select();
	}
}

void MSaltApp::DoAbout()
{
// TODO: fix package version string
	DisplayAlert(nullptr, "about-alert", {GetApplicationVersion(), "PACKAGE_VERSION"});
}

bool MSaltApp::AllowQuit(bool inLogOff)
{
	mQuitPending =
		inLogOff or
		mQuitPending or
		(mConnectionPool.has_open_channels() == false and MTerminalWindow::IsAnyTerminalOpen() == false) or
		DisplayAlert(nullptr, "close-all-sessions-alert", {}) == 1;

	return mQuitPending;
}

void MSaltApp::DoQuit()
{
	mQuit = true;
	mQuitPending = true;

	mConnectionPool.disconnect_all();

	vector<MWindow *> windows;

	for (MWindow *w = MWindow::GetFirstWindow(); w != nullptr; w = w->GetNextWindow())
		windows.push_back(w);

	for (auto w : windows)
		w->Close();

	MApplication::DoQuit();
}

void MSaltApp::DoNew()
{
	MWindow *w = MTerminalWindow::Create();
	w->Select();
}

void MSaltApp::Open(const string &inFile)
{
	std::regex re("^(?:ssh://)?(?:([-$_.+!*'(),[:alnum:];?&=]+)(?::([-$_.+!*'(),[:alnum:];?&=]+))?@)?([-[:alnum:].]+)(?::(\\d+))?$");
	std::smatch m;

	if (std::regex_match(inFile, m, re))
	{
		string user = m[1];
		string host = m[3];
		uint16_t port = m[4].matched ? std::stoi(m[4]) : 22;
		string command;

		std::shared_ptr<pinch::basic_connection> connection = mConnectionPool.get(user, host, port);
		MWindow *w = MTerminalWindow::Create(host, user, port, command, connection);
		w->Select();
	}
}

#if not defined(_MSC_VER)

#include <gtk/gtk.h>

void MApplication::Install(const string &inPrefix)
{
	if (getuid() != 0)
		throw runtime_error("You must be root to be able to install japi");

	if (not fs::exists(gExecutablePath))
		throw runtime_error(string("I don't seem to exist...?") + gExecutablePath.string());

	fs::path prefix(inPrefix);
	if (prefix.empty())
		prefix = "/usr/local";

	// copy the executable to the appropriate destination
	if (not fs::exists(prefix / "bin"))
	{
		cout << "Creating directory " << (prefix / "bin") << endl;
		fs::create_directories(prefix / "bin");
	}
	fs::path exeDest = prefix / "bin" / "salt";

	if (not fs::exists(prefix / "share" / "salt"))
	{
		cout << "Creating directory " << (prefix / "share" / "salt") << endl;
		fs::create_directories(prefix / "share" / "salt");
	}
	fs::path iconDest = prefix / "share" / "salt" / "icon.png";

	cout << "copying " << gExecutablePath.string() << " to " << exeDest.string() << endl;

	if (fs::exists(exeDest))
		fs::remove(exeDest);

	fs::copy_file(gExecutablePath, exeDest);

	if (fs::exists(iconDest))
		fs::remove(iconDest);

	// create desktop file
	mrsrc::rsrc rsrc("salt.desktop");
	mrsrc::rsrc icon("Icons/appicon.png");

	if (not rsrc)
		throw runtime_error("salt.desktop file could not be created, missing data");

	string desktop(rsrc.data(), rsrc.size());
	ba::replace_all(desktop, "__EXE__", exeDest.string());
	ba::replace_all(desktop, "__ICON__", iconDest.string());

	// locate applications directory
	// don't use glib here,

	fs::path desktopFile, applicationsDir;

	const char *const *config_dirs = g_get_system_data_dirs();
	for (const char *const *dir = config_dirs; *dir != nullptr; ++dir)
	{
		applicationsDir = fs::path(*dir) / "applications";
		if (fs::exists(applicationsDir) and fs::is_directory(applicationsDir))
			break;
	}

	if (not fs::exists(applicationsDir))
	{
		cout << "Creating directory " << applicationsDir << endl;
		fs::create_directories(applicationsDir);
	}

	cout << "writing icon file " << iconDest << endl;

	desktopFile = applicationsDir / "salt.desktop";
	cout << "writing desktop file " << desktopFile << endl;

	std::ofstream file(desktopFile, ios::trunc);
	file << desktop;
	file.close();

	cout << "writing icon file " << iconDest << endl;
	file.open(iconDest, ios::trunc | ios::binary);
	file.write(icon.data(), icon.size());
	file.close();

	exit(0);
}

#endif
