//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.h"

#include "zeep/xml/document.hpp"
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include "MDialog.h"
#include "MControls.h"
#include "MGtkWindowImpl.h"
#include "MError.h"
#include "MStrings.h"
#include "MGtkApplicationImpl.h"
#include "MGtkControlsImpl.h"
#include "MUtils.h"
#include "MDevice.h"
#include "MResources.h"
#include "MAcceleratorTable.h"

using namespace std;
using namespace zeep;
namespace io = boost::iostreams;
namespace ba = boost::algorithm;

class MGtkDialogImpl : public MGtkWindowImpl
{
  public:
	MGtkDialogImpl(const string& inResource, MWindow* inParent)
		: MGtkWindowImpl(MWindowFlags(0), "", inParent)
		, mResponse(this, &MGtkDialogImpl::OnResponse)
		, mRsrc(inResource)
		, mResultIsOK(false)
	{
	}

	virtual bool ShowModal();
	
	virtual void Create(MRect inBounds, const std::string& inTitle);
	virtual void Finish();

	virtual bool OnKeyPressEvent(GdkEventKey* inEvent);

	virtual void Append(MGtkWidgetMixin* inChild, MControlPacking inPacking,
						bool inExpand, bool inFill, uint32 inPadding);

	void GetMargins(xml::element* inTemplate,
		int32& outLeftMargin, int32& outTopMargin, int32& outRightMargin, int32& outBottomMargin);

	MView* CreateControls(xml::element* inTemplate, int32 inX, int32 inY);

	MView* CreateButton(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateColorSwatch(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateCaption(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateCheckbox(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateRadiobutton(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateExpander(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateCombobox(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateEdittext(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreatePopup(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateScrollbar(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateSeparator(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateBox(xml::element* inTemplate, int32 inX, int32 inY, bool inHorizontal);
	MView* CreateTable(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateNotebook(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreatePager(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateListBox(xml::element* inTemplate, int32 inX, int32 inY);
	MView* CreateListView(xml::element* inTemplate, int32 inX, int32 inY);

	uint32 GetTextWidth(const string& inText, const wchar_t* inClass, int inPartID, int inStateID);

	string l(const string& s)					{ return GetLocalisedStringForContext(mRsrc, s); }

	void OnResponse(int32 inResponseID)
	{
		PRINT(("Response: %d", inResponseID));

		MDialog* dlog = static_cast<MDialog*>(mWindow);
		
		switch (inResponseID)
		{
			case GTK_RESPONSE_OK:
				mResultIsOK = true;
				if (dlog->OKClicked())
					dlog->Close();
				break;
			
			case GTK_RESPONSE_CANCEL:
			case GTK_RESPONSE_DELETE_EVENT:
				if (dlog->CancelClicked() and inResponseID != GTK_RESPONSE_DELETE_EVENT)
					dlog->Close();
				break;
			
			default:
				if (inResponseID > 0 and static_cast<uint32>(inResponseID) <= mResponseIDs.size())
					dlog->ButtonClicked(mResponseIDs[inResponseID - 1]);
				break;
		}
	}
	
	MSlot<void(int32)> mResponse;

	float mDLUX, mDLUY;

	string mRsrc;
	list<MRadiobutton*> mRadioGroup;
	vector<string> mResponseIDs;
	int32 mDefaultResponse;
	bool mResultIsOK;
};

bool MGtkDialogImpl::OnKeyPressEvent(GdkEventKey* inEvent)
{
	bool result = MGtkWidgetMixin::OnKeyPressEvent(inEvent);
	
	if (not result)
	{
		PRINT(("MGtkDialogImpl::OnKeyPressEvent"));

		uint32 keyCode = MapKeyCode(inEvent->keyval);
		uint32 modifiers = MapModifier(inEvent->state);
		
		if ((keyCode == kEnterKeyCode or keyCode == kReturnKeyCode) and modifiers == 0)
		{
			OnResponse(mDefaultResponse);
			result = true;
		}
	}
	
	return result;
}

bool MGtkDialogImpl::ShowModal()
{
	MGtkWindowImpl::Select();

	(void)gtk_dialog_run(GTK_DIALOG(GetWidget()));
	return mResultIsOK;
}

void MGtkDialogImpl::Create(MRect inBounds, const std::string& inTitle)
{
	GtkWidget* widget = gtk_dialog_new();
	THROW_IF_NIL(widget);

	gtk_window_set_default_size(GTK_WINDOW(widget), inBounds.width, inBounds.height);
	gtk_window_set_title(GTK_WINDOW(widget), inTitle.c_str());

	SetWidget(widget);
	
	mMapEvent.Connect(widget, "map-event");
	mResponse.Connect(widget, "response");
}

void MGtkDialogImpl::Finish()
{
	string resource = string("Dialogs/") + mRsrc + ".xml";
	mrsrc::rsrc rsrc(resource);
	
	if (not rsrc)
		THROW(("Dialog resource not found: %s", resource.c_str()));

	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
	xml::document doc(data);

	xml::element* dialog = doc.find_first("/dialog");
	if (dialog == nullptr)
		THROW(("Invalid dialog resource"));
	
	string title = l(dialog->attr("title"));

	mFlags = kMFixedSize;
	string flags = dialog->attr("flags");
	if (ba::contains(flags, "flexible"))
		mFlags = MWindowFlags(mFlags & ~kMFixedSize);
	if (ba::contains(flags, "nosizebox"))
		mFlags = MWindowFlags(mFlags | kMNoSizeBox);

	uint32 minWidth = 40;
	if (not dialog->attr("width").empty())
		minWidth = boost::lexical_cast<uint32>(dialog->attr("width"));
	uint32 minHeight = 40;
	if (not dialog->attr("height").empty())
		minHeight = boost::lexical_cast<uint32>(dialog->attr("height"));

	MRect bounds(0, 0, minWidth, minHeight);

	// now create the dialog
	Create(bounds, title);

	// setup the DLU values
	
	MDevice dev;
	dev.SetText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	mDLUX = dev.GetTextWidth() / (52 * 4.0f);
	mDLUY = dev.GetLineHeight() / 8.0f;

	// create the dialog controls, all stacked on top of each other
	xml::element* vbox = dialog->find_first("vbox");
	if (vbox == nullptr)
		THROW(("Invalid dialog resource"));
	
	MView* content = CreateControls(vbox, 0, 0);
//	content->SetBindings(true, true, true, true);

	for (MRadiobutton* radiobutton: mRadioGroup)
		radiobutton->SetGroup(mRadioGroup);

//	MControlBase* control = dynamic_cast<MControlBase*>(content);
//	if (control != nullptr)
//		control->SetPadding(4);

	mWindow->AddChild(content);
	
	// the buttons
	
	xml::element* buttons = dialog->find_first("hbox");
	if (buttons == nullptr)
		THROW(("Invalid dialog resource"));

	mDefaultResponse = 0;
	
	for (auto button: *buttons)
	{
		if (button.name() == "button")
		{
			mResponseIDs.push_back(button.attr("id"));
			
			int32 response = mResponseIDs.size();
			if (button.attr("id") == "ok")
				response = GTK_RESPONSE_OK;
			else if (button.attr("id") == "cancel")
				response = GTK_RESPONSE_CANCEL;
			
			GtkWidget* wdgt = gtk_dialog_add_button(GTK_DIALOG(GetWidget()),
				l(button.attr("title")).c_str(), response);

			if (button.attr("default") == "true")
			{
				gtk_widget_grab_default(wdgt);
				gtk_dialog_set_default_response(GTK_DIALOG(GetWidget()), response);
				mDefaultResponse = response;
			}
		}
	}
}

void MGtkDialogImpl::Append(MGtkWidgetMixin* inChild, MControlPacking inPacking,
	bool inExpand, bool inFill, uint32 inPadding)
{
	GtkWidget* box = 
		gtk_dialog_get_content_area(GTK_DIALOG(GetWidget()));
	
	if (inPacking == ePackStart)
		gtk_box_pack_start(GTK_BOX(box), inChild->GetWidget(), inExpand, inFill, inPadding);
	else
		gtk_box_pack_end(GTK_BOX(box), inChild->GetWidget(), inExpand, inFill, inPadding);
}

void MGtkDialogImpl::GetMargins(xml::element* inTemplate,
	int32& outLeftMargin, int32& outTopMargin, int32& outRightMargin, int32& outBottomMargin)
{
	outLeftMargin = outTopMargin = outRightMargin = outBottomMargin = 0;
	
	if (inTemplate->name() == "dialog" or inTemplate->name() == "notebook")
		outLeftMargin = outTopMargin = outRightMargin = outBottomMargin = 7;
	
	string m = inTemplate->attr("margin");
	if (not m.empty())
		outLeftMargin = outRightMargin =
		outTopMargin = outBottomMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->attr("margin-left-right");
	if (not m.empty())
		outLeftMargin = outRightMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->attr("margin-top-bottom");
	if (not m.empty())
		outTopMargin = outBottomMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->attr("margin-left");
	if (not m.empty())
		outLeftMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->attr("margin-top");
	if (not m.empty())
		outTopMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->attr("margin-right");
	if (not m.empty())
		outRightMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->attr("margin-bottom");
	if (not m.empty())
		outBottomMargin = boost::lexical_cast<int32>(m);

//	outLeftMargin = static_cast<int32>(outLeftMargin * mDLUX);
//	outRightMargin = static_cast<int32>(outRightMargin * mDLUX);
//	outTopMargin = static_cast<int32>(outTopMargin * mDLUY);
//	outBottomMargin = static_cast<int32>(outBottomMargin * mDLUY);
}

MView* MGtkDialogImpl::CreateButton(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	string title = l(inTemplate->attr("title"));
	
//	float idealWidth = GetTextWidth(title, VSCLASS_BUTTON, BP_PUSHBUTTON, PBS_NORMAL) + 10 * mDLUX;
//	if (idealWidth < 50 * mDLUX)
//		idealWidth = 50 * mDLUX;
	MRect bounds;//(inX, inY, static_cast<int32>(idealWidth), static_cast<int32>(14 * mDLUY));

	MButtonFlags flags = eBF_None;
	
	if (inTemplate->attr("split") == "true")
		flags = eBF_Split;

	MButton* button = new MButton(id, bounds, title, flags);
	
//	if (inTemplate->attr("default") == "true")
//		button->MakeDefault(true);
//
//	if (id == "ok" and mOKButton == nullptr)
//		mOKButton = button;
//	
//	if (id == "cancel")
//		mCancelButton = button;

	AddRoute(button->eClicked, static_cast<MDialog*>(mWindow)->eButtonClicked);

	return button;
}

MView* MGtkDialogImpl::CreateColorSwatch(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	
	MRect bounds;//(inX, inY, static_cast<int32>(25 * mDLUX), static_cast<int32>(14 * mDLUY));

	MColor color(inTemplate->attr("color").c_str());
	MColorSwatch* swatch = new MColorSwatch(id, bounds, color);
	
	AddRoute(swatch->eColorChanged, static_cast<MDialog*>(mWindow)->eColorChanged);

	return swatch;
}

MView* MGtkDialogImpl::CreateExpander(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	string title = l(inTemplate->attr("title"));
	
	MRect bounds;//(inX, inY,
//		static_cast<int32>((13 + 3) * mDLUX) +
//			GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_LABEL, 0),
//		static_cast<int32>(12 * mDLUY));

	MExpander* expander = new MExpander(id, bounds, title);
	AddRoute(expander->eClicked, static_cast<MDialog*>(mWindow)->eButtonClicked);

	for (auto b: *inTemplate)
		expander->AddChild(CreateControls(&b, 0, 0));

	return expander;
}

MView* MGtkDialogImpl::CreateCaption(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	if (id.empty())
		id = "caption";
	string text = l(inTemplate->attr("text"));

	MRect bounds;//(inX, static_cast<int32>(inY), 0, static_cast<int32>(10 * mDLUY));
//	bounds.width = GetTextWidth(text, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	return new MCaption(id, bounds, text);
}

MView* MGtkDialogImpl::CreateCheckbox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	string title = l(inTemplate->attr("title"));

	MRect bounds;//(inX, inY, 0, static_cast<int32>(10 * mDLUY));
//	bounds.width = static_cast<int32>(14 * mDLUX) +
//		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
//		//GetTextWidth(title, VSCLASS_BUTTON, BP_CHECKBOX, PBS_NORMAL);

	MCheckbox* checkbox = new MCheckbox(id, bounds, title);
	AddRoute(checkbox->eValueChanged,
		static_cast<MDialog*>(mWindow)->eCheckboxClicked);
	return checkbox;
}

MView* MGtkDialogImpl::CreateRadiobutton(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	string title = l(inTemplate->attr("title"));

	MRect bounds;//(inX, inY, 0, static_cast<int32>(10 * mDLUY));
//	bounds.width = static_cast<int32>(14 * mDLUX) +
//		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
//		GetTextWidth(title, VSCLASS_BUTTON, BP_RADIOBUTTON, PBS_NORMAL);
	MRadiobutton* radiobutton = new MRadiobutton(id, bounds, title);
	AddRoute(radiobutton->eValueChanged,
		static_cast<MDialog*>(mWindow)->eRadiobuttonClicked);

	mRadioGroup.push_back(radiobutton);

	return radiobutton;
}

MView* MGtkDialogImpl::CreateCombobox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");

	MRect bounds;//(inX, inY, static_cast<int32>(50 * mDLUX), static_cast<int32>(14 * mDLUY));
	MCombobox* combobox = new MCombobox(id, bounds);
	AddRoute(combobox->eValueChanged,
		static_cast<MDialog*>(mWindow)->eTextChanged);
	return combobox;
}

MView* MGtkDialogImpl::CreateEdittext(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	
	uint32 flags = eMEditTextNoFlags;
	if (ba::contains(inTemplate->attr("style"), "right"))
		flags |= eMEditTextAlignRight;
	if (ba::contains(inTemplate->attr("style"), "number"))
		flags |= eMEditTextNumbers;
	if (ba::contains(inTemplate->attr("style"), "multiline"))
		flags |= eMEditTextMultiLine;
	if (ba::contains(inTemplate->attr("style"), "readonly"))
		flags |= eMEditTextReadOnly;

	MRect bounds;//(inX, inY, static_cast<int32>(5 * mDLUX), static_cast<int32>(14 * mDLUY));
	MEdittext* edittext = new MEdittext(id, bounds, flags);

	if (inTemplate->attr("password") == "true")
		edittext->SetPasswordChar();

	AddRoute(edittext->eValueChanged,
		static_cast<MDialog*>(mWindow)->eTextChanged);
	return edittext;
}

MView* MGtkDialogImpl::CreatePopup(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");

	MRect bounds;//(inX, inY, 0, static_cast<int32>(14 * mDLUY));
	
	vector<string> choices;
	for (xml::element* option: inTemplate->find("./option"))
	{
		string label = option->content();
//		int32 width = GetTextWidth(label, VSCLASS_COMBOBOX, CP_DROPDOWNBUTTON, CBXSL_NORMAL);
//		if (bounds.width < width)
//			bounds.width = width;
		choices.push_back(label);
	}

//	bounds.width += static_cast<int32>(14 * mDLUX);

	MPopup* popup = new MPopup(id, bounds);

	popup->SetChoices(choices);
	AddRoute(popup->eValueChanged,
		static_cast<MDialog*>(mWindow)->eValueChanged);

	return popup;
}

MView* MGtkDialogImpl::CreateNotebook(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");

	MRect r(inX, inY, 0, 0);
	MNotebook* result = new MNotebook(id, r);
	
	MRect b;
	
	for (xml::element* page: inTemplate->find("./page"))
	{
		string title = l(page->attr("title"));
		
		MView* control = CreateControls(page, 0, 0);
		control->SetBindings(true, true, true, true);
		result->AddPage(title, control);
		
		control->RecalculateLayout();
		
		MRect f;
		control->GetFrame(f);
		b |= f;
	}

//	// calculate a new width/height for our tab control
//	HTHEME hTheme = ::OpenThemeData(GetHandle(), VSCLASS_TAB);
//	if (hTheme != nullptr)
//	{
//		RECT r = { 0, 0, b.width, b.height };
//
//		RECT extend;
//		::GetThemeBackgroundContentRect(hTheme, mDC, TABP_PANE, 0, &r, &extend);
//
////		h += h - (extend.bottom - extend.top);
////		w += w - (extend.right - extend.left);
//
//		SIZE size;
//		::GetThemePartSize(hTheme, mDC, TABP_TABITEM, TTIS_NORMAL, &extend, TS_TRUE, &size);
//
//		::CloseThemeData(hTheme);
//
//		//h += size.cy;
//		//w += size.cx;
//	}
//
//	r.width = b.width;
//	r.height = b.height;
//
//	result->SetFrame(r);
//	result->SelectPage(0);
	
	return result;
}

MView* MGtkDialogImpl::CreatePager(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");

	MRect r(inX, inY, 0, 0);
	MPager* result = new MPager(id, r);
	
	MRect b;
	
	for (xml::element* page: inTemplate->find("./page"))
	{
		MView* control = CreateControls(page, 0, 0);
		control->SetBindings(true, true, true, true);
		result->AddPage(control);
		
		control->RecalculateLayout();
		
		MRect f;
		control->GetFrame(f);
		b |= f;
	}

	r.width = b.width;
	r.height = b.height;

	result->SetFrame(r);
	result->SelectPage(0);
	
	return result;
}

MView* MGtkDialogImpl::CreateListBox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");

	MRect r(inX, inY, 0, 0);
	MListBox* result = new MListBox(id, r);
	
	for (auto listitem: inTemplate->find("./listitem"))
	{
		string text = l(listitem->content());
//		int32 textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
//		if (r.width < textWidth)
//			r.width = textWidth;
		result->AddItem(text);
	}
	
//	r.width += static_cast<int32>(mDLUX * 6);
//	result->SetFrame(r);

	AddRoute(result->eValueChanged, static_cast<MDialog*>(mWindow)->eValueChanged);
	
	return result;
}

MView* MGtkDialogImpl::CreateListView(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");

	MRect r(inX, inY, 0, 0);
	MListView* result = new MListView(id, r);
	
	for (xml::element* listitem: inTemplate->find("./listitem"))
	{
		string text = l(listitem->content());
//		int32 textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
//		if (r.width < textWidth)
//			r.width = textWidth;
		result->AddItem(text);
	}
	
//	r.width += static_cast<int32>(mDLUX * 6);
//	result->SetFrame(r);

	AddRoute(result->eValueChanged, static_cast<MDialog*>(mWindow)->eValueChanged);

	return result;
}

MView* MGtkDialogImpl::CreateSeparator(xml::element* inTemplate, int32 inX, int32 inY)
{
	MRect bounds(inX, inY, 2, 2);
	return new MSeparator("separator", bounds);
}

MView* MGtkDialogImpl::CreateScrollbar(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");
	string orientation = inTemplate->attr("orientation");

	MRect bounds(inX, inY, kScrollbarWidth, kScrollbarWidth);
	
	if (orientation == "horizontal")
		bounds.width *= 2;
	else
		bounds.height *= 2;
	
	return new MScrollbar(id, bounds);
}

MView* MGtkDialogImpl::CreateBox(xml::element* inTemplate, int32 inX, int32 inY, bool inHorizontal)
{
	string id = inTemplate->attr("id");

	uint32 spacing = 4;
	if (not inTemplate->attr("spacing").empty())
		spacing = boost::lexical_cast<uint32>(inTemplate->attr("spacing"));

	uint32 padding = 4;
	if (not inTemplate->attr("padding").empty())
		spacing = boost::lexical_cast<uint32>(inTemplate->attr("padding"));

	bool expand = inTemplate->attr("expand") == "true";
	bool fill = inTemplate->attr("fill") == "true";
	bool homogeneous = inTemplate->attr("homogeneous") == "true";

	MRect r(inX, inY, 0, 0);
	MView* result = new MBoxControl(id, r, inHorizontal, homogeneous, expand, fill, spacing, padding); 
	
	for (auto b: *inTemplate)
		result->AddChild(CreateControls(&b, 0, 0));
	
	return result;
}

MView* MGtkDialogImpl::CreateTable(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->attr("id");

	vector<MView*> views;
	uint32 colCount = 0, rowCount = 0;
	
	for (xml::element* row: inTemplate->find("./row"))
	{
		uint32 cn = 0;
		
		for (auto col: *row)
		{
			++cn;
			if (colCount < cn)
				colCount = cn;
			views.push_back(CreateControls(&col, 0, 0));
		}
		
		++rowCount;
	}

	// fix me!
	while (views.size() < (rowCount * colCount))
		views.push_back(nullptr);
	
	MRect r(inX, inY, 0, 0);
	MTable* result = new MTable(id, r,
//		&views[0], colCount, rowCount, static_cast<int32>(4 * mDLUX), static_cast<int32>(4 * mDLUY));
		&views[0], colCount, rowCount, 4, 4);
	
	return result;
}

MView* MGtkDialogImpl::CreateControls(xml::element* inTemplate, int32 inX, int32 inY)
{
	MView* result = nullptr;

	string name = inTemplate->name();

	if (name == "button")
		result = CreateButton(inTemplate, inX, inY);
	else if (name == "caption")
		result = CreateCaption(inTemplate, inX, inY);
	else if (name == "checkbox")
		result = CreateCheckbox(inTemplate, inX, inY);
	else if (name == "swatch")
		result = CreateColorSwatch(inTemplate, inX, inY);
	else if (name == "radiobutton")
		result = CreateRadiobutton(inTemplate, inX, inY);
	else if (name == "expander")
		result = CreateExpander(inTemplate, inX, inY);
	else if (name == "combobox")
		result = CreateCombobox(inTemplate, inX, inY);
	else if (name == "edittext")
		result = CreateEdittext(inTemplate, inX, inY);
	else if (name == "popup")
		result = CreatePopup(inTemplate, inX, inY);
	else if (name == "scrollbar")
		result = CreateScrollbar(inTemplate, inX, inY);
	else if (name == "separator")
		result = CreateSeparator(inTemplate, inX, inY);
	else if (name == "table")
		result = CreateTable(inTemplate, inX, inY);
	else if (name == "vbox" or name == "dialog" or name == "page")
		result = CreateBox(inTemplate, inX, inY, false);
	else if (name == "hbox")
		result = CreateBox(inTemplate, inX, inY, true);
	else if (name == "notebook")
		result = CreateNotebook(inTemplate, inX, inY);
	else if (name == "pager")
		result = CreatePager(inTemplate, inX, inY);
	else if (name == "listbox")
		result = CreateListBox(inTemplate, inX, inY);
	else if (name == "listview")
		result = CreateListView(inTemplate, inX, inY);
	else if (name == "filler")
		result = new MView(inTemplate->attr("id"), MRect(inX, inY, 0, 0));

	MControlBase* control = dynamic_cast<MControlBase*>(result);

	int32 marginLeft, marginTop, marginRight, marginBottom;
	GetMargins(inTemplate, marginLeft, marginTop, marginRight, marginBottom);
//	result->SetMargins(marginLeft, marginTop, marginRight, marginBottom);

	if (not inTemplate->attr("width").empty())
	{
		int32 width = marginLeft + marginRight;
		
		if (inTemplate->attr("width") == "scrollbarwidth")
			width += kScrollbarWidth;
		else
//			width += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->attr("width")) * mDLUX);
			width += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->attr("width")));

		MGtkWidgetMixin* impl = dynamic_cast<MGtkWidgetMixin*>(control->GetControlImplBase());
		if (impl != nullptr)
			impl->RequestSize(width * mDLUX, -1);
	}
	
//	if (not inTemplate->attr("height").empty())
//	{
//		int32 height = marginTop + marginBottom;
//		
//		if (inTemplate->attr("height") == "scrollbarheight")
//			height += kScrollbarWidth;
//		else
////			height += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->attr("height")) * mDLUY);
//			height += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->attr("height")));
//		
//		MRect frame;
//		result->GetFrame(frame);
//		if (frame.height < height)
//			result->ResizeFrame(0, height - frame.height);
//	}

	if (control != nullptr)
	{
		control->SetLayout(
			inTemplate->attr("packing") == "end" ? ePackEnd : ePackStart,
			inTemplate->attr("expand") == "true" ? true : false,
			inTemplate->attr("fill") == "true" ? true : false,
			inTemplate->attr("padding").empty() ? 0 : boost::lexical_cast<int32>(inTemplate->attr("padding")));
	}

	return result;
}

uint32 MGtkDialogImpl::GetTextWidth(const string& inText,
	const wchar_t* inClass, int inPartID, int inStateID)
{
	uint32 result = 0;
//	wstring text(c2w(inText));
//	
//	HTHEME hTheme = ::OpenThemeData(GetHandle(), inClass);
//
//	if (hTheme != nullptr)
//	{
//		RECT r;
//		THROW_IF_HRESULT_ERROR(::GetThemeTextExtent(hTheme, mDC,
//			inPartID, inStateID, text.c_str(), text.length(), 0, nullptr, &r));
//		result = r.right - r.left;
//		::CloseThemeData(hTheme);
//	}
//	else
//	{
//		SIZE size;
//		::GetTextExtentPoint32(mDC, text.c_str(), text.length(), &size);
//		result = size.cx;
//	}
	
	return result;
}




// --------------------------------------------------------------------

MWindowImpl* MWindowImpl::CreateDialog(const string& inResource, MWindow* inWindow)
{
	return new MGtkDialogImpl(inResource, inWindow);
}
