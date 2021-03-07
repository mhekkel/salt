// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <set>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include <pinch/types.hpp>

#include "MPreferencesDialog.hpp"
#include "MTerminalWindow.hpp"
#include "MPreferences.hpp"
#include "MDevice.hpp"
#include "MSaltApp.hpp"
#include "MColorPicker.hpp"
#include "MControls.hpp"
#include "MAlerts.hpp"

using namespace std;
namespace ba = boost::algorithm;

MPreferencesDialog*		MPreferencesDialog::sInstance;
MEventOut<void()>		MPreferencesDialog::ePreferencesChanged;
MEventOut<void(MColor)>	MPreferencesDialog::eColorPreview;

MPreferencesDialog& MPreferencesDialog::Instance()
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
	bool useCertificates = Preferences::GetBoolean("use-certificate", true);

	SetChecked("use-certificate", useCertificates);
	SetChecked("forward-agent", Preferences::GetBoolean("forward-agent", true));
	SetChecked("act-as-pageant", Preferences::GetBoolean("act-as-pageant", true));

	SetEnabled("forward-agent", useCertificates);
	SetEnabled("act-as-pageant", useCertificates);

	SetChecked("forward-x11", Preferences::GetBoolean("forward-x11", true));
	SetChecked("udk-with-shift", Preferences::GetBoolean("udk-with-shift", true));
	
	string answerback = Preferences::GetString("answer-back", "salt\r");
	ba::replace_all(answerback, "\r", "\\r");
	ba::replace_all(answerback, "\n", "\\n");
	ba::replace_all(answerback, "\t", "\\t");
	SetText("answer-back", answerback);
	
	SetColor("back-color", MColor(Preferences::GetString("back-color", "#0f290e")));
	
	MColorSwatch* swatch = dynamic_cast<MColorSwatch*>(FindSubViewByID("back-color"));
	if (swatch != nullptr)
		AddRoute(swatch->eColorPreview, ePreviewColor);

	SetChecked("audible-beep", Preferences::GetBoolean("audible-beep", true));
	SetChecked("graphical-beep", Preferences::GetBoolean("graphical-beep", true));
	
	stringstream env;
	vector<string> envs;
	Preferences::GetArray("env", envs);
	for (const string& s: envs)
		env << s << endl;
	SetText("env", env.str());
	
	SetText("enc", Preferences::GetString("enc", pinch::kEncryptionAlgorithms));
	SetText("mac", Preferences::GetString("mac", pinch::kMacAlgorithms));
	SetText("kex", Preferences::GetString("kex", pinch::kKeyExchangeAlgorithms));
	SetText("cmp", Preferences::GetString("cmp", pinch::kCompressionAlgorithms));

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
	Preferences::SetInteger("buffer-size",  std::stoul(GetText("buffer")));
	Preferences::SetBoolean("block-cursor", IsChecked("block-cursor"));
	Preferences::SetBoolean("blink-cursor", IsChecked("blink-cursor"));
	Preferences::SetString("terminal-type", GetText("terminal-type"));
	Preferences::SetBoolean("ignore-color", IsChecked("ignore-color"));
	Preferences::SetBoolean("show-status-bar", IsChecked("show-status-bar"));
	
	//
	Preferences::SetBoolean("use-certificate", IsChecked("use-certificate"));
	Preferences::SetBoolean("forward-agent", IsChecked("forward-agent"));
	Preferences::SetBoolean("act-as-pageant", IsChecked("act-as-pageant"));
	Preferences::SetBoolean("forward-x11", IsChecked("forward-x11"));
	Preferences::SetBoolean("udk-with-shift", IsChecked("udk-with-shift"));

	// commit pageant setting
	pinch::ssh_agent::instance().expose_pageant(IsChecked("act-as-pageant"));

	string answerback = GetText("answer-back");
	ba::replace_all(answerback, "\\r", "\r");
	ba::replace_all(answerback, "\\n", "\n");
	ba::replace_all(answerback, "\\t", "\t");
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
	struct { pinch::algorithm type; const char* conf; const char* desc; string def; } kAlgs[] = {
		{ pinch::algorithm::encryption, "enc", "encryption", pinch::kEncryptionAlgorithms },
		{ pinch::algorithm::verification, "mac", "verification", pinch::kMacAlgorithms },
		{ pinch::algorithm::keyexchange, "kex", "key exchange", pinch::kKeyExchangeAlgorithms },
		{ pinch::algorithm::compression, "cmp", "compression", pinch::kCompressionAlgorithms }
	};
	
	auto& connectionPool = static_cast<MSaltApp*>(gApp)->GetConnectionPool();

	for (auto alg: kAlgs) 
	{
		bool ok = true;
		
		std::set<std::string> allowed;
		ba::split(allowed, alg.def, ba::is_any_of(","));
		
		vector<string> v;
		string s = GetText(alg.conf);
		ba::split(v, s, ba::is_any_of(","));
		
		for (auto& a: v)
		{
			ba::trim(a);
			if (allowed.count(a) == 0)
			{
				DisplayAlert(this, "algo-unsupported", alg.desc, a);
				ok = false;
			}
		}
		
		if (ok)
		{
			string algo = ba::join(v, ",");
			Preferences::SetString(alg.conf, algo);
			connectionPool.set_algorithm(alg.type, pinch::direction::both, algo);
		}
	}
	
	ePreferencesChanged();
}

void MPreferencesDialog::ButtonClicked(const string& inID)
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
		SetText("cmp", pinch::kCompressionAlgorithms);
	}
	else
		MDialog::ButtonClicked(inID);
}

void MPreferencesDialog::TextChanged(const string& inID, const string& inText)
{
//	SetEnabled("apply", true);
}

void MPreferencesDialog::ValueChanged(const string& inID, int32_t inValue)
{
	if (inID == "page-selector")
		static_cast<MPager*>(FindSubViewByID("pages"))->SelectPage(inValue);
//	else
//		SetEnabled("apply", true);
}

void MPreferencesDialog::ColorChanged(const string& inID, MColor inValue)
{
//	SetEnabled("apply", true);
}

void MPreferencesDialog::CheckboxChanged(const string& inID, bool inValue)
{
//	SetEnabled("apply", true);
//	SetEnabled("apply", true);

	bool useCertificates = IsChecked("use-certificate");
	SetEnabled("forward-agent", useCertificates);
	SetEnabled("act-as-pageant", useCertificates);
	
	if (not useCertificates)
	{
		SetChecked("forward-agent", false);
		SetChecked("act-as-pageant", false);
	}
}

void MPreferencesDialog::ColorPreview(const string& inID, MColor inValue)
{
	eColorPreview(inValue);
}
