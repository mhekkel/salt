//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>

#include <zeep/xml/document.hpp>

#include "MAlerts.hpp"
#include "MResources.hpp"
#include "MError.hpp"
#include "MStrings.hpp"
#include "MGtkWindowImpl.hpp"

using namespace std;
namespace xml = zeep::xml;
namespace io = boost::iostreams;
namespace ba = boost::algorithm;

GtkWidget* CreateAlertWithArgs(
	const char* 	inResourceName,
	vector<string>&	inArgs)
{
	GtkWidget* dlg = nullptr;
	
	string resource = string("Alerts/") + inResourceName + ".xml";
	
	mrsrc::rsrc rsrc(resource);
	if (not rsrc)
		THROW(("Could not load resource Alerts/%s.xml", inResourceName));
	
	// parse the XML data
	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
	xml::document doc(data);
	
	// build an alert
	xml::element* root = doc.find_first("/alert");
	
	if (root->name() != "alert")
		THROW(("Invalid resource for alert %s, first tag should be <alert>", inResourceName));
	
	string instruction, content;
	vector<pair<string,uint32_t> > btns;
	int32_t defaultButton = -1;
	GtkMessageType type = GTK_MESSAGE_INFO;
	
	if (root->get_attribute("type") == "warning")
		type = GTK_MESSAGE_WARNING;
	else if (root->get_attribute("type") == "error")
		type = GTK_MESSAGE_ERROR;
	
	for (auto item: *root)
	{
		if (item.name() == "content")
		{
			// replace parameters
			char s[] = "^0";
			string text = GetLocalisedStringForContext(inResourceName, item.get_content());
	
			for (auto a: inArgs)
			{
				string::size_type p = text.find(s);
				if (p != string::npos)
					text.replace(p, 2, a);
				++s[1];
			}

			content = text;
		}
		else if (item.name() == "instruction")
		{
			// replace parameters
			char s[] = "^0";
			string text = GetLocalisedStringForContext(inResourceName, item.get_content());
	
			for (auto a: inArgs)
			{
				ba::replace_all(text, s, a);
				++s[1];
			}

			instruction = text;
		}
		else if (item.name() == "buttons")
		{
			for (auto button: item)
			{
				if (button.name() == "button")
				{
					string label = GetLocalisedStringForContext(inResourceName, button.get_attribute("title"));
					uint32_t cmd = stoul(button.get_attribute("cmd"));

					if (button.get_attribute("default") == "true")
						defaultButton = cmd;
					
					btns.push_back(make_pair(label, cmd));
				}
			}
		}
	}

	dlg = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL,
		type, GTK_BUTTONS_NONE, "%s", instruction.c_str());
	THROW_IF_NIL(dlg);
	if (not content.empty())
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dlg), content.c_str());
	
	for (auto b: btns)
		gtk_dialog_add_button(GTK_DIALOG(dlg), b.first.c_str(), b.second);

	if (defaultButton >= 0)
		gtk_dialog_set_default_response(GTK_DIALOG(dlg), defaultButton);
	
	return dlg;
}

int32_t DisplayAlert(MWindow* inParent, const string& inResourceName, vector<string>& inArgs)
{
	int32_t result = -1;

	try
	{
		GtkWidget* dlg = CreateAlertWithArgs(inResourceName.c_str(), inArgs);
	
		if (inParent != nullptr)
		{
			inParent->Select();
			
			MGtkWindowImpl* impl =
				static_cast<MGtkWindowImpl*>(inParent->GetImpl());
			
			gtk_window_set_transient_for(
				GTK_WINDOW(dlg),
				GTK_WINDOW(impl->GetWidget()));
		}
		else if (MWindow::GetFirstWindow() != nullptr)
		{
			MWindow::GetFirstWindow()->Select();
			
			MGtkWindowImpl* impl =
				static_cast<MGtkWindowImpl*>(MWindow::GetFirstWindow()->GetImpl());
			
			gtk_window_set_transient_for(
				GTK_WINDOW(dlg),
				GTK_WINDOW(impl->GetWidget()));
		}
	
		result = gtk_dialog_run(GTK_DIALOG(dlg));
		
		gtk_widget_destroy(dlg);
	}
	catch (exception& e)
	{
		DisplayError(e);
	}

	return result;
}

