//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include <boost/algorithm/string.hpp>

#include <zeep/xml/document.hpp>

#include "MAlerts.hpp"
#include "mrsrc.hpp"
#include "MError.hpp"
#include "MStrings.hpp"
#include "MWinUtils.hpp"
#include "MWinApplicationImpl.hpp"
#include "MWinWindowImpl.hpp"
#include "MUtils.hpp"

using namespace std;
namespace xml = zeep::xml;
namespace ba = boost::algorithm;

string localise(const string& inResourceName, const string& inText)
{
	return GetLocalisedStringForContext(inResourceName, inText);
}

int32_t DisplayAlert(
	MWindow*			inParent,
	const string&		inResourceName,
	vector<string>&		inArguments)
{
	using namespace std::literals;

	mrsrc::rsrc rsrc("Alerts/"s + inResourceName + ".xml");
	xml::document doc(rsrc);
	
	// build an alert
	xml::element* root = doc.find_first("/alert");
	
	if (root->name() != "alert")
		THROW(("Invalid resource for alert %s, first tag should be <alert>", inResourceName));

	// OK, setup a standard Task dialog

	TASKDIALOGCONFIG config = { sizeof(TASKDIALOGCONFIG) };
	vector<TASKDIALOG_BUTTON> buttons;
	list<wstring> buttonLabels;

	config.hInstance = MWinApplicationImpl::GetInstance()->GetHInstance();

	if (inParent != nullptr)
	{
		inParent->Select();
		config.hwndParent = static_cast<MWinWindowImpl*>(inParent->GetImpl())->GetHandle();
	}

	config.pszMainIcon = TD_INFORMATION_ICON;
	
	wstring instruction, content;

	if (root->get_attribute("type") == "warning")
		config.pszMainIcon = TD_WARNING_ICON;
	else if (root->get_attribute("type") == "error")
		config.pszMainIcon = TD_ERROR_ICON;
	else
		config.pszMainIcon = TD_INFORMATION_ICON;
	
	uint32_t cancelID = 0;
	const uint32_t kButtenNrOffset = 0x01000;

	for (auto &item: *root)
	{
		if (item.name() == "content")
		{
			// replace parameters
			char s[] = "^0";
			string text = localise(inResourceName, item.get_content());
	
			for (string a: inArguments)
			{
				string::size_type p = text.find(s);
				if (p != string::npos)
					text.replace(p, 2, a);
				++s[1];
			}

			content = c2w(text);
		}
		else if (item.name() == "instruction")
		{
			// replace parameters
			char s[] = "^0";
			string text = localise(inResourceName, item.get_content());
	
			for (string a: inArguments)
			{
				ba::replace_all(text, s, a);
				++s[1];
			}

			instruction = c2w(text);
		}
		else if (item.name() == "buttons")
		{
			for (auto &button: item)
			{
				if (button.name() == "button")
				{
					string label = localise(inResourceName, button.get_attribute("title"));
					uint32_t cmd = atoi(button.get_attribute("cmd").c_str());

					assert(cmd < 10);
					cmd += kButtenNrOffset;

					if (label == "Cancel")
					{
						config.dwCommonButtons = TDCBF_CANCEL_BUTTON;
						cancelID = cmd;
						cmd = IDCANCEL;
					}
					else
					{
						buttonLabels.push_back(c2w(_(label)));

						TASKDIALOG_BUTTON button = { cmd, buttonLabels.back().c_str() };
						buttons.push_back(button);
					}

					if (button.get_attribute("default") == "true")
						config.nDefaultButton = cmd;
				}
			}
		}
	}

	if (not instruction.empty())
		config.pszMainInstruction = instruction.c_str();
	if (not content.empty())
		config.pszContent = content.c_str();
	config.pButtons = &buttons[0];
	config.cButtons = buttons.size();

	int pressedButton;
	THROW_IF_HRESULT_ERROR(::TaskDialogIndirect(&config, &pressedButton, NULL, NULL));

	int result = pressedButton;
	if (pressedButton == IDCANCEL)
		result = cancelID;
	result -= kButtenNrOffset;

	return result;
}
