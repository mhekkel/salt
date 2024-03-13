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

#include "MPreferencesDialog.hpp"
#include "MAlerts.hpp"
#include "MColorPicker.hpp"
#include "MControls.hpp"
#include "MDevice.hpp"
#include "MPreferences.hpp"
#include "MSaltApp.hpp"
#include "MTerminalWindow.hpp"

#include <pinch.hpp>

#include <zeep/unicode-support.hpp>

#include <boost/algorithm/string.hpp>

#include <set>
#include <sstream>

using namespace std;
namespace ba = boost::algorithm;

MPreferencesDialog *MPreferencesDialog::sInstance;
MEventOut<void()> MPreferencesDialog::ePreferencesChanged;
MEventOut<void(MColor)> MPreferencesDialog::eColorPreview;

MPreferencesDialog &MPreferencesDialog::Instance()
{
	if (sInstance == nullptr)
		sInstance = new MPreferencesDialog;
	return *sInstance;
}

MPreferencesDialog::MPreferencesDialog()
	: MDialog("prefs-dialog")
	, ePreviewColor(this, &MPreferencesDialog::ColorPreview)
{
	vector<string> fonts;
	MDevice::ListFonts(true, fonts);
	sort(fonts.begin(), fonts.end());

	SetChoices("font", fonts);

	string font = Preferences::GetString("font", "Consolas 10");
	string::size_type s = font.rfind(' ');
	if (s == string::npos)
		font = "Consolas 10";

	try
	{
		SetText("font", font.substr(0, s));
		SetText("size", font.substr(s + 1));
	}
	catch (...)
	{
		SetValue("font", 1);
		SetValue("size", 1);
	}

	SetText("buffer", std::to_string(Preferences::GetInteger("buffer-size", 5000)));
	SetChecked("block-cursor", Preferences::GetBoolean("block-cursor", false));
	SetChecked("blink-cursor", Preferences::GetBoolean("blink-cursor", false));
	SetText("terminal-type", Preferences::GetString("terminal-type", "xterm"));
	SetChecked("ignore-color", Preferences::GetBoolean("ignore-color", false));
	SetChecked("show-status-bar", Preferences::GetBoolean("show-status-bar", false));

	// connection page
#if defined _MSC_VER
	bool useCertificates = Preferences::GetBoolean("use-certificate", true);

	SetChecked("use-certificate", useCertificates);
	SetEnabled("forward-ssh-agent", useCertificates);
	SetEnabled("act-as-pageant", useCertificates);
#else
	SetChecked("forward-ssh-agent", Preferences::GetBoolean("forward-ssh-agent", true));
#endif

	SetChecked("forward-gpg-agent", Preferences::GetBoolean("forward-gpg-agent", true));

	std::vector<std::string> dpc;
	Preferences::GetArray("disallowed-paste-characters", dpc);
	if (dpc.empty())
	{
		dpc = std::vector<std::string>{ "BS", "DEL", "ENQ", "EOT", "ESC", "NUL" };
		Preferences::SetArray("disallowed-paste-characters", dpc);
	}

	SetText("disallowed-paste-characters", zeep::join(dpc, ","));

	SetChecked("forward-x11", Preferences::GetBoolean("forward-x11", true));
	SetChecked("udk-with-shift", Preferences::GetBoolean("udk-with-shift", true));

	string answerback = Preferences::GetString("answer-back", "salt\r");
	zeep::replace_all(answerback, "\r", "\\r");
	zeep::replace_all(answerback, "\n", "\\n");
	zeep::replace_all(answerback, "\t", "\\t");
	SetText("answer-back", answerback);

	SetColor("back-color", MColor(Preferences::GetString("back-color", "#0f290e")));

	MColorSwatch *swatch = dynamic_cast<MColorSwatch *>(FindSubViewByID("back-color"));
	if (swatch != nullptr)
		AddRoute(swatch->eColorPreview, ePreviewColor);

	SetChecked("audible-beep", Preferences::GetBoolean("audible-beep", true));
	SetChecked("graphical-beep", Preferences::GetBoolean("graphical-beep", true));

	stringstream env;
	vector<string> envs;
	Preferences::GetArray("env", envs);
	for (const string &s : envs)
		env << s << endl;
	SetText("env", env.str());

	SetText("enc", Preferences::GetString("enc", pinch::kEncryptionAlgorithms));
	SetText("mac", Preferences::GetString("mac", pinch::kMacAlgorithms));
	SetText("kex", Preferences::GetString("kex", pinch::kKeyExchangeAlgorithms));
	SetText("cmp", Preferences::GetString("cmp", pinch::kCompressionAlgorithms));
	SetText("shk", Preferences::GetString("shk", pinch::kServerHostKeyAlgorithms));

	//	SetEnabled("apply", false);
}

MPreferencesDialog::~MPreferencesDialog()
{
	sInstance = nullptr;
}

bool MPreferencesDialog::AllowClose(bool inLogOff)
{
	sInstance = nullptr;
	return true;
}

bool MPreferencesDialog::OKClicked()
{
	Apply();
	return true;
}

void MPreferencesDialog::Apply()
{
	Preferences::SetString("font", GetText("font") + ' ' + GetText("size"));
	Preferences::SetString("back-color", GetColor("back-color").str());
	Preferences::SetInteger("buffer-size", std::stoul(GetText("buffer")));
	Preferences::SetBoolean("block-cursor", IsChecked("block-cursor"));
	Preferences::SetBoolean("blink-cursor", IsChecked("blink-cursor"));
	Preferences::SetString("terminal-type", GetText("terminal-type"));
	Preferences::SetBoolean("ignore-color", IsChecked("ignore-color"));
	Preferences::SetBoolean("show-status-bar", IsChecked("show-status-bar"));

	//
#if defined _MSC_VER
	Preferences::SetBoolean("use-certificate", IsChecked("use-certificate"));
	Preferences::SetBoolean("forward-ssh-agent", IsChecked("forward-ssh-agent"));
	Preferences::SetBoolean("act-as-pageant", IsChecked("act-as-pageant"));

	// commit pageant setting
	pinch::ssh_agent::instance().expose_pageant(IsChecked("act-as-pageant"));
#else
	Preferences::SetBoolean("forward-ssh-agent", IsChecked("forward-ssh-agent"));
#endif

	Preferences::SetBoolean("forward-gpg-agent", IsChecked("forward-gpg-agent"));

	Preferences::SetBoolean("forward-x11", IsChecked("forward-x11"));
	Preferences::SetBoolean("udk-with-shift", IsChecked("udk-with-shift"));

	const std::set<std::string> kDisallowedPasteCharacters{
		"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS", "HT", "LF", "VT",
		"FF", "CR", "SO", "SI", "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
		"CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US", "DEL"
	};

	std::string s = GetText("disallowed-paste-characters");
	std::vector<std::string> dpc;
	auto i = 0;
	for (;;)
	{
		auto j = s.find_first_of(",; ", i);

		auto s1 = s.substr(i, j - i);
		if (kDisallowedPasteCharacters.count(s1))
			dpc.emplace_back(s1);

		if (j == std::string::npos)
			break;

		i = j + 1;
	}

	Preferences::SetArray("disallowed-paste-characters", dpc);
	SetText("disallowed-paste-characters", zeep::join(dpc, ","));

	string answerback = GetText("answer-back");
	zeep::replace_all(answerback, "\\r", "\r");
	zeep::replace_all(answerback, "\\n", "\n");
	zeep::replace_all(answerback, "\\t", "\t");
	Preferences::SetString("answer-back", answerback);

	Preferences::SetBoolean("audible-beep", IsChecked("audible-beep"));
	Preferences::SetBoolean("graphical-beep", IsChecked("graphical-beep"));

	stringstream envs(GetText("env"));
	vector<string> env;

	for (;;)
	{
		string v;
		getline(envs, v);
		if (v.empty())
		{
			if (envs.eof())
				break;
			continue;
		}

		string::size_type s = v.find('=');
		if (s != string::npos)
			env.push_back(v);
	}

	Preferences::SetArray("env", env);

	// protocols
	struct
	{
		pinch::algorithm type;
		const char *conf;
		const char *desc;
		string def;
	} kAlgs[] = {
		{ pinch::algorithm::encryption, "enc", "encryption", pinch::kEncryptionAlgorithms },
		{ pinch::algorithm::verification, "mac", "verification", pinch::kMacAlgorithms },
		{ pinch::algorithm::keyexchange, "kex", "key exchange", pinch::kKeyExchangeAlgorithms },
		{ pinch::algorithm::serverhostkey, "shk", "host key validation", pinch::kServerHostKeyAlgorithms },
		{ pinch::algorithm::compression, "cmp", "compression", pinch::kCompressionAlgorithms }
	};

	for (auto alg : kAlgs)
	{
		bool ok = true;

		std::set<std::string> allowed;
		ba::split(allowed, alg.def, ba::is_any_of(","));

		vector<string> v;
		string s = GetText(alg.conf);
		ba::split(v, s, ba::is_any_of(","));

		for (auto &a : v)
		{
			ba::trim(a);
			if (allowed.count(a) == 0)
			{
				DisplayAlert(this, "algo-unsupported", { alg.desc, a });
				ok = false;
				break;
			}
		}

		if (ok)
		{
			string algo = zeep::join(v, ",");
			Preferences::SetString(alg.conf, algo);

			pinch::key_exchange::set_algorithm(alg.type, pinch::direction::both, algo);
		}
	}

	ePreferencesChanged();
}

void MPreferencesDialog::ButtonClicked(const string &inID)
{
	if (inID == "apply")
	{
		Apply();
		//		SetEnabled("apply", false);
	}
	else if (inID == "reset-protocols")
	{
		SetText("enc", pinch::kEncryptionAlgorithms);
		SetText("mac", pinch::kMacAlgorithms);
		SetText("kex", pinch::kKeyExchangeAlgorithms);
		SetText("shk", pinch::kServerHostKeyAlgorithms);
		SetText("cmp", pinch::kCompressionAlgorithms);
	}
	else
		MDialog::ButtonClicked(inID);
}

void MPreferencesDialog::TextChanged(const string &inID, const string &inText)
{
	//	SetEnabled("apply", true);
}

void MPreferencesDialog::ValueChanged(const string &inID, int32_t inValue)
{
	if (inID == "page-selector")
		static_cast<MPager *>(FindSubViewByID("pages"))->SelectPage(inValue);
	//	else
	//		SetEnabled("apply", true);
}

void MPreferencesDialog::ColorChanged(const string &inID, MColor inValue)
{
	//	SetEnabled("apply", true);
}

void MPreferencesDialog::CheckboxChanged(const string &inID, bool inValue)
{
	//	SetEnabled("apply", true);
	//	SetEnabled("apply", true);

#if defined _MSC_VER
	bool useCertificates = IsChecked("use-certificate");
	SetEnabled("forward-ssh-agent", useCertificates);
	SetEnabled("act-as-pageant", useCertificates);

	if (not useCertificates)
	{
		SetChecked("forward-ssh-agent", false);
		SetChecked("act-as-pageant", false);
	}
#endif
}

void MPreferencesDialog::ColorPreview(const string &inID, MColor inValue)
{
	eColorPreview(inValue);
}
