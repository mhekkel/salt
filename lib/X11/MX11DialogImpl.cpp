//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include "zeep/xml/document.hpp"
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include "MDialog.hpp"
#include "MControls.hpp"
#include "MX11WindowImpl.hpp"
#include "MError.hpp"
#include "MStrings.hpp"
#include "MX11ApplicationImpl.hpp"
#include "MX11ControlsImpl.hpp"
#include "MUtils.hpp"
#include "MDevice.hpp"
#include "MResources.hpp"
#include "MAcceleratorTable.hpp"

using namespace std;
using namespace zeep;
namespace io = boost::iostreams;
namespace ba = boost::algorithm;

//class MX11DialogImpl : public MX11WindowImpl
//{
//  public:
//	MX11DialogImpl(const string& inResource, MWindow* inParent)
//		: MX11WindowImpl(MWindowFlags(0), "", inParent)
//		, mResponse(this, &MX11DialogImpl::OnResponse)
//		, mRsrc(inResource)
//		, mResultIsOK(false)
//	{
//	}
//
//	virtual bool ShowModal();
//	
//	virtual void Create(MRect inBounds, const std::string& inTitle);
//	virtual void Finish();
//
//	virtual bool OnKeyPressEvent(GdkEventKey* inEvent);
//
//	virtual void Append(MX11WidgetMixin* inChild, MControlPacking inPacking,
//						bool inExpand, bool inFill, uint32 inPadding);
//
//	void GetMargins(xml::element* inTemplate,
//		int32& outLeftMargin, int32& outTopMargin, int32& outRightMargin, int32& outBottomMargin);
//
//	MView* CreateControls(xml::element* inTemplate, int32 inX, int32 inY);
//
//	MView* CreateButton(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateColorSwatch(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateCaption(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateCheckbox(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateRadiobutton(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateExpander(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateCombobox(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateEdittext(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreatePopup(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateScrollbar(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateSeparator(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateBox(xml::element* inTemplate, int32 inX, int32 inY, bool inHorizontal);
//	MView* CreateTable(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateNotebook(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreatePager(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateListBox(xml::element* inTemplate, int32 inX, int32 inY);
//	MView* CreateListView(xml::element* inTemplate, int32 inX, int32 inY);
//
//	uint32 GetTextWidth(const string& inText, const wchar_t* inClass, int inPartID, int inStateID);
//
//	string l(const string& s)					{ return GetLocalisedStringForContext(mRsrc, s); }
//
//	void OnResponse(int32 inResponseID)
//	{
//		PRINT(("Response: %d", inResponseID));
//
//		MDialog* dlog = static_cast<MDialog*>(mWindow);
//		
//		switch (inResponseID)
//		{
//			case GTK_RESPONSE_OK:
//				mResultIsOK = true;
//				if (dlog->OKClicked())
//					dlog->Close();
//				break;
//			
//			case GTK_RESPONSE_CANCEL:
//			case GTK_RESPONSE_DELETE_EVENT:
//				if (dlog->CancelClicked() and inResponseID != GTK_RESPONSE_DELETE_EVENT)
//					dlog->Close();
//				break;
//			
//			default:
//				if (inResponseID > 0 and inResponseID <= mResponseIDs.size())
//					dlog->ButtonClicked(mResponseIDs[inResponseID - 1]);
//				break;
//		}
//	}
//	
//	MSlot<void(int32)> mResponse;
//
//	float mDLUX, mDLUY;
//
//	string mRsrc;
//	list<MRadiobutton*> mRadioGroup;
//	vector<string> mResponseIDs;
//	int32 mDefaultResponse;
//	bool mResultIsOK;
//};
//
//bool MX11DialogImpl::OnKeyPressEvent(GdkEventKey* inEvent)
//{
//	bool result = MX11WidgetMixin::OnKeyPressEvent(inEvent);
//	
//	if (not result)
//	{
//		PRINT(("MX11DialogImpl::OnKeyPressEvent"));
//
//		uint32 keyCode = MapKeyCode(inEvent->keyval);
//		uint32 modifiers = MapModifier(inEvent->state);
//		
//		if ((keyCode == kEnterKeyCode or keyCode == kReturnKeyCode) and modifiers == 0)
//		{
//			OnResponse(mDefaultResponse);
//			result = true;
//		}
//	}
//	
//	return result;
//}
//
//bool MX11DialogImpl::ShowModal()
//{
//	int result = gtk_dialog_run(GTK_DIALOG(GetWidget()));
//	return mResultIsOK;
//}
//
//void MX11DialogImpl::Create(MRect inBounds, const std::string& inTitle)
//{
//	GtkWidget* widget = gtk_dialog_new();
//	THROW_IF_NIL(widget);
//
//	gtk_window_set_default_size(GTK_WINDOW(widget), inBounds.width, inBounds.height);
//	gtk_window_set_title(GTK_WINDOW(widget), inTitle.c_str());
//
//	SetWidget(widget);
//	
//	mMapEvent.Connect(widget, "map-event");
//	mResponse.Connect(widget, "response");
//}
//
//void MX11DialogImpl::Finish()
//{
//	string resource = string("Dialogs/") + mRsrc + ".xml";
//	mrsrc::rsrc rsrc(resource);
//	
//	if (not rsrc)
//		THROW(("Dialog resource not found: %s", resource.c_str()));
//
//	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
//	xml::document doc(data);
//
//	xml::element* dialog = doc.find_first("/dialog");
//	if (dialog == nullptr)
//		THROW(("Invalid dialog resource"));
//	
//	string title = l(dialog->get_attribute("title"));
//
//	mFlags = kMFixedSize;
//	string flags = dialog->get_attribute("flags");
//	if (ba::contains(flags, "flexible"))
//		mFlags = MWindowFlags(mFlags & ~kMFixedSize);
//	if (ba::contains(flags, "nosizebox"))
//		mFlags = MWindowFlags(mFlags | kMNoSizeBox);
//
//	uint32 minWidth = 40;
//	if (not dialog->get_attribute("width").empty())
//		minWidth = boost::lexical_cast<uint32>(dialog->get_attribute("width"));
//	uint32 minHeight = 40;
//	if (not dialog->get_attribute("height").empty())
//		minHeight = boost::lexical_cast<uint32>(dialog->get_attribute("height"));
//
//	MRect bounds(0, 0, minWidth, minHeight);
//
//	// now create the dialog
//	Create(bounds, title);
//
//	// setup the DLU values
//	
//	MDevice dev;
//	dev.SetText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
//	mDLUX = dev.GetTextWidth() / (52 * 4.0f);
//	mDLUY = dev.GetLineHeight() / 8.0f;
//
//	// create the dialog controls, all stacked on top of each other
//	xml::element* vbox = dialog->find_first("vbox");
//	if (vbox == nullptr)
//		THROW(("Invalid dialog resource"));
//	
//	MView* content = CreateControls(vbox, 0, 0);
////	content->SetBindings(true, true, true, true);
//
//	for (MRadiobutton* radiobutton: mRadioGroup)
//		radiobutton->SetGroup(mRadioGroup);
//
////	MControlBase* control = dynamic_cast<MControlBase*>(content);
////	if (control != nullptr)
////		control->SetPadding(4);
//
//	mWindow->AddChild(content);
//	
//	// the buttons
//	
//	xml::element* buttons = dialog->find_first("hbox");
//	if (buttons == nullptr)
//		THROW(("Invalid dialog resource"));
//
//	mDefaultResponse = 0;
//	
//	for (auto button: *buttons)
//	{
//		if (button->name() == "button")
//		{
//			mResponseIDs.push_back(button->get_attribute("id"));
//			
//			int32 response = mResponseIDs.size();
//			if (button->get_attribute("id") == "ok")
//				response = GTK_RESPONSE_OK;
//			else if (button->get_attribute("id") == "cancel")
//				response = GTK_RESPONSE_CANCEL;
//			
//			GtkWidget* wdgt = gtk_dialog_add_button(GTK_DIALOG(GetWidget()),
//				l(button->get_attribute("title")).c_str(), response);
//
//			if (button->get_attribute("default") == "true")
//			{
//				gtk_widget_grab_default(wdgt);
//				gtk_dialog_set_default_response(GTK_DIALOG(GetWidget()), response);
//				mDefaultResponse = response;
//			}
//		}
//	}
//}
//
//void MX11DialogImpl::Append(MX11WidgetMixin* inChild, MControlPacking inPacking,
//	bool inExpand, bool inFill, uint32 inPadding)
//{
//	GtkWidget* box = 
//		gtk_dialog_get_content_area(GTK_DIALOG(GetWidget()));
//	
//	if (inPacking == ePackStart)
//		gtk_box_pack_start(GTK_BOX(box), inChild->GetWidget(), inExpand, inFill, inPadding);
//	else
//		gtk_box_pack_end(GTK_BOX(box), inChild->GetWidget(), inExpand, inFill, inPadding);
//}
//
//void MX11DialogImpl::GetMargins(xml::element* inTemplate,
//	int32& outLeftMargin, int32& outTopMargin, int32& outRightMargin, int32& outBottomMargin)
//{
//	outLeftMargin = outTopMargin = outRightMargin = outBottomMargin = 0;
//	
//	if (inTemplate->name() == "dialog" or inTemplate->name() == "notebook")
//		outLeftMargin = outTopMargin = outRightMargin = outBottomMargin = 7;
//	
//	string m = inTemplate->get_attribute("margin");
//	if (not m.empty())
//		outLeftMargin = outRightMargin =
//		outTopMargin = outBottomMargin = boost::lexical_cast<int32>(m);
//
//	m = inTemplate->get_attribute("margin-left-right");
//	if (not m.empty())
//		outLeftMargin = outRightMargin = boost::lexical_cast<int32>(m);
//
//	m = inTemplate->get_attribute("margin-top-bottom");
//	if (not m.empty())
//		outTopMargin = outBottomMargin = boost::lexical_cast<int32>(m);
//
//	m = inTemplate->get_attribute("margin-left");
//	if (not m.empty())
//		outLeftMargin = boost::lexical_cast<int32>(m);
//
//	m = inTemplate->get_attribute("margin-top");
//	if (not m.empty())
//		outTopMargin = boost::lexical_cast<int32>(m);
//
//	m = inTemplate->get_attribute("margin-right");
//	if (not m.empty())
//		outRightMargin = boost::lexical_cast<int32>(m);
//
//	m = inTemplate->get_attribute("margin-bottom");
//	if (not m.empty())
//		outBottomMargin = boost::lexical_cast<int32>(m);
//
////	outLeftMargin = static_cast<int32>(outLeftMargin * mDLUX);
////	outRightMargin = static_cast<int32>(outRightMargin * mDLUX);
////	outTopMargin = static_cast<int32>(outTopMargin * mDLUY);
////	outBottomMargin = static_cast<int32>(outBottomMargin * mDLUY);
//}
//
//MView* MX11DialogImpl::CreateButton(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	string title = l(inTemplate->get_attribute("title"));
//	
////	float idealWidth = GetTextWidth(title, VSCLASS_BUTTON, BP_PUSHBUTTON, PBS_NORMAL) + 10 * mDLUX;
////	if (idealWidth < 50 * mDLUX)
////		idealWidth = 50 * mDLUX;
//	MRect bounds;//(inX, inY, static_cast<int32>(idealWidth), static_cast<int32>(14 * mDLUY));
//
//	MButtonFlags flags = eBF_None;
//	
//	if (inTemplate->get_attribute("split") == "true")
//		flags = eBF_Split;
//
//	MButton* button = new MButton(id, bounds, title, flags);
//	
////	if (inTemplate->get_attribute("default") == "true")
////		button->MakeDefault(true);
////
////	if (id == "ok" and mOKButton == nullptr)
////		mOKButton = button;
////	
////	if (id == "cancel")
////		mCancelButton = button;
//
//	AddRoute(button->eClicked, static_cast<MDialog*>(mWindow)->eButtonClicked);
//
//	return button;
//}
//
//MView* MX11DialogImpl::CreateColorSwatch(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	
//	MRect bounds;//(inX, inY, static_cast<int32>(25 * mDLUX), static_cast<int32>(14 * mDLUY));
//
//	MColor color(inTemplate->get_attribute("color").c_str());
//	MColorSwatch* swatch = new MColorSwatch(id, bounds, color);
//	
//	AddRoute(swatch->eColorChanged, static_cast<MDialog*>(mWindow)->eColorChanged);
//
//	return swatch;
//}
//
//MView* MX11DialogImpl::CreateExpander(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	string title = l(inTemplate->get_attribute("title"));
//	
//	MRect bounds;//(inX, inY,
////		static_cast<int32>((13 + 3) * mDLUX) +
////			GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_LABEL, 0),
////		static_cast<int32>(12 * mDLUY));
//
//	MExpander* expander = new MExpander(id, bounds, title);
//	AddRoute(expander->eClicked, static_cast<MDialog*>(mWindow)->eButtonClicked);
//
//	for (xml::element* b: inTemplate->children<xml::element>())
//		expander->AddChild(CreateControls(b, 0, 0));
//
//	return expander;
//}
//
//MView* MX11DialogImpl::CreateCaption(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	if (id.empty())
//		id = "caption";
//	string text = l(inTemplate->get_attribute("text"));
//
//	MRect bounds;//(inX, static_cast<int32>(inY), 0, static_cast<int32>(10 * mDLUY));
////	bounds.width = GetTextWidth(text, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
//	return new MCaption(id, bounds, text);
//}
//
//MView* MX11DialogImpl::CreateCheckbox(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	string title = l(inTemplate->get_attribute("title"));
//
//	MRect bounds;//(inX, inY, 0, static_cast<int32>(10 * mDLUY));
////	bounds.width = static_cast<int32>(14 * mDLUX) +
////		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
////		//GetTextWidth(title, VSCLASS_BUTTON, BP_CHECKBOX, PBS_NORMAL);
//
//	MCheckbox* checkbox = new MCheckbox(id, bounds, title);
//	AddRoute(checkbox->eValueChanged,
//		static_cast<MDialog*>(mWindow)->eCheckboxClicked);
//	return checkbox;
//}
//
//MView* MX11DialogImpl::CreateRadiobutton(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	string title = l(inTemplate->get_attribute("title"));
//
//	MRect bounds;//(inX, inY, 0, static_cast<int32>(10 * mDLUY));
////	bounds.width = static_cast<int32>(14 * mDLUX) +
////		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
////		GetTextWidth(title, VSCLASS_BUTTON, BP_RADIOBUTTON, PBS_NORMAL);
//	MRadiobutton* radiobutton = new MRadiobutton(id, bounds, title);
//	AddRoute(radiobutton->eValueChanged,
//		static_cast<MDialog*>(mWindow)->eRadiobuttonClicked);
//
//	mRadioGroup.push_back(radiobutton);
//
//	return radiobutton;
//}
//
//MView* MX11DialogImpl::CreateCombobox(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//
//	MRect bounds;//(inX, inY, static_cast<int32>(50 * mDLUX), static_cast<int32>(14 * mDLUY));
//	MCombobox* combobox = new MCombobox(id, bounds);
//	AddRoute(combobox->eValueChanged,
//		static_cast<MDialog*>(mWindow)->eTextChanged);
//	return combobox;
//}
//
//MView* MX11DialogImpl::CreateEdittext(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	
//	uint32 flags = eMEditTextNoFlags;
//	if (ba::contains(inTemplate->get_attribute("style"), "right"))
//		flags |= eMEditTextAlignRight;
//	if (ba::contains(inTemplate->get_attribute("style"), "number"))
//		flags |= eMEditTextNumbers;
//	if (ba::contains(inTemplate->get_attribute("style"), "multiline"))
//		flags |= eMEditTextMultiLine;
//	if (ba::contains(inTemplate->get_attribute("style"), "readonly"))
//		flags |= eMEditTextReadOnly;
//
//	MRect bounds;//(inX, inY, static_cast<int32>(5 * mDLUX), static_cast<int32>(14 * mDLUY));
//	MEdittext* edittext = new MEdittext(id, bounds, flags);
//
//	if (inTemplate->get_attribute("password") == "true")
//		edittext->SetPasswordChar();
//
//	AddRoute(edittext->eValueChanged,
//		static_cast<MDialog*>(mWindow)->eTextChanged);
//	return edittext;
//}
//
//MView* MX11DialogImpl::CreatePopup(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//
//	MRect bounds;//(inX, inY, 0, static_cast<int32>(14 * mDLUY));
//	
//	vector<string> choices;
//	for (xml::element* option: inTemplate->find("./option"))
//	{
//		string label = option->content();
////		int32 width = GetTextWidth(label, VSCLASS_COMBOBOX, CP_DROPDOWNBUTTON, CBXSL_NORMAL);
////		if (bounds.width < width)
////			bounds.width = width;
//		choices.push_back(label);
//	}
//
////	bounds.width += static_cast<int32>(14 * mDLUX);
//
//	MPopup* popup = new MPopup(id, bounds);
//
//	popup->SetChoices(choices);
//	AddRoute(popup->eValueChanged,
//		static_cast<MDialog*>(mWindow)->eValueChanged);
//
//	return popup;
//}
//
//MView* MX11DialogImpl::CreateNotebook(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//
//	MRect r(inX, inY, 0, 0);
//	MNotebook* result = new MNotebook(id, r);
//	
//	MRect b;
//	
//	for (xml::element* page: inTemplate->find("./page"))
//	{
//		string title = l(page->get_attribute("title"));
//		
//		MView* control = CreateControls(page, 0, 0);
//		control->SetBindings(true, true, true, true);
//		result->AddPage(title, control);
//		
//		control->RecalculateLayout();
//		
//		MRect f;
//		control->GetFrame(f);
//		b |= f;
//	}
//
////	// calculate a new width/height for our tab control
////	HTHEME hTheme = ::OpenThemeData(GetHandle(), VSCLASS_TAB);
////	if (hTheme != nullptr)
////	{
////		RECT r = { 0, 0, b.width, b.height };
////
////		RECT extend;
////		::GetThemeBackgroundContentRect(hTheme, mDC, TABP_PANE, 0, &r, &extend);
////
//////		h += h - (extend.bottom - extend.top);
//////		w += w - (extend.right - extend.left);
////
////		SIZE size;
////		::GetThemePartSize(hTheme, mDC, TABP_TABITEM, TTIS_NORMAL, &extend, TS_TRUE, &size);
////
////		::CloseThemeData(hTheme);
////
////		//h += size.cy;
////		//w += size.cx;
////	}
////
////	r.width = b.width;
////	r.height = b.height;
////
////	result->SetFrame(r);
////	result->SelectPage(0);
//	
//	return result;
//}
//
//MView* MX11DialogImpl::CreatePager(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//
//	MRect r(inX, inY, 0, 0);
//	MPager* result = new MPager(id, r);
//	
//	MRect b;
//	
//	for (xml::element* page: inTemplate->find("./page"))
//	{
//		MView* control = CreateControls(page, 0, 0);
//		control->SetBindings(true, true, true, true);
//		result->AddPage(control);
//		
//		control->RecalculateLayout();
//		
//		MRect f;
//		control->GetFrame(f);
//		b |= f;
//	}
//
//	r.width = b.width;
//	r.height = b.height;
//
//	result->SetFrame(r);
//	result->SelectPage(0);
//	
//	return result;
//}
//
//MView* MX11DialogImpl::CreateListBox(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//
//	MRect r(inX, inY, 0, 0);
//	MListBox* result = new MListBox(id, r);
//	
//	for (xml::element* listitem: inTemplate->find("./listitem"))
//	{
//		string text = l(listitem->content());
////		int32 textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
////		if (r.width < textWidth)
////			r.width = textWidth;
//		result->AddItem(text);
//	}
//	
////	r.width += static_cast<int32>(mDLUX * 6);
////	result->SetFrame(r);
//
//	AddRoute(result->eValueChanged, static_cast<MDialog*>(mWindow)->eValueChanged);
//	
//	return result;
//}
//
//MView* MX11DialogImpl::CreateListView(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//
//	MRect r(inX, inY, 0, 0);
//	MListView* result = new MListView(id, r);
//	
//	for (xml::element* listitem: inTemplate->find("./listitem"))
//	{
//		string text = l(listitem->content());
////		int32 textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
////		if (r.width < textWidth)
////			r.width = textWidth;
//		result->AddItem(text);
//	}
//	
////	r.width += static_cast<int32>(mDLUX * 6);
////	result->SetFrame(r);
//
//	AddRoute(result->eValueChanged, static_cast<MDialog*>(mWindow)->eValueChanged);
//
//	return result;
//}
//
//MView* MX11DialogImpl::CreateSeparator(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	MRect bounds(inX, inY, 2, 2);
//	return new MSeparator("separator", bounds);
//}
//
//MView* MX11DialogImpl::CreateScrollbar(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//	string orientation = inTemplate->get_attribute("orientation");
//
//	MRect bounds(inX, inY, kScrollbarWidth, kScrollbarWidth);
//	
//	if (orientation == "horizontal")
//		bounds.width *= 2;
//	else
//		bounds.height *= 2;
//	
//	return new MScrollbar(id, bounds);
//}
//
//MView* MX11DialogImpl::CreateBox(xml::element* inTemplate, int32 inX, int32 inY, bool inHorizontal)
//{
//	string id = inTemplate->get_attribute("id");
//
//	uint32 spacing = 4;
//	if (not inTemplate->get_attribute("spacing").empty())
//		spacing = boost::lexical_cast<uint32>(inTemplate->get_attribute("spacing"));
//
//	uint32 padding = 4;
//	if (not inTemplate->get_attribute("padding").empty())
//		spacing = boost::lexical_cast<uint32>(inTemplate->get_attribute("padding"));
//
//	bool expand = inTemplate->get_attribute("expand") == "true";
//	bool fill = inTemplate->get_attribute("fill") == "true";
//	bool homogeneous = inTemplate->get_attribute("homogeneous") == "true";
//
//	MRect r(inX, inY, 0, 0);
//	MView* result = new MBoxControl(id, r, inHorizontal, homogeneous, expand, fill, spacing, padding); 
//	
//	for (xml::element* b: inTemplate->children<xml::element>())
//		result->AddChild(CreateControls(b, 0, 0));
//	
//	return result;
//}
//
//MView* MX11DialogImpl::CreateTable(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	string id = inTemplate->get_attribute("id");
//
//	vector<MView*> views;
//	uint32 colCount = 0, rowCount = 0;
//	
//	for (xml::element* row: inTemplate->find("./row"))
//	{
//		uint32 cn = 0;
//		
//		for (xml::element* col: row->children<xml::element>())
//		{
//			++cn;
//			if (colCount < cn)
//				colCount = cn;
//			views.push_back(CreateControls(col, 0, 0));
//		}
//		
//		++rowCount;
//	}
//
//	// fix me!
//	while (views.size() < (rowCount * colCount))
//		views.push_back(nullptr);
//	
//	MRect r(inX, inY, 0, 0);
//	MTable* result = new MTable(id, r,
////		&views[0], colCount, rowCount, static_cast<int32>(4 * mDLUX), static_cast<int32>(4 * mDLUY));
//		&views[0], colCount, rowCount, 4, 4);
//	
//	return result;
//}
//
//MView* MX11DialogImpl::CreateControls(xml::element* inTemplate, int32 inX, int32 inY)
//{
//	MView* result = nullptr;
//
//	string name = inTemplate->name();
//
//	if (name == "button")
//		result = CreateButton(inTemplate, inX, inY);
//	else if (name == "caption")
//		result = CreateCaption(inTemplate, inX, inY);
//	else if (name == "checkbox")
//		result = CreateCheckbox(inTemplate, inX, inY);
//	else if (name == "swatch")
//		result = CreateColorSwatch(inTemplate, inX, inY);
//	else if (name == "radiobutton")
//		result = CreateRadiobutton(inTemplate, inX, inY);
//	else if (name == "expander")
//		result = CreateExpander(inTemplate, inX, inY);
//	else if (name == "combobox")
//		result = CreateCombobox(inTemplate, inX, inY);
//	else if (name == "edittext")
//		result = CreateEdittext(inTemplate, inX, inY);
//	else if (name == "popup")
//		result = CreatePopup(inTemplate, inX, inY);
//	else if (name == "scrollbar")
//		result = CreateScrollbar(inTemplate, inX, inY);
//	else if (name == "separator")
//		result = CreateSeparator(inTemplate, inX, inY);
//	else if (name == "table")
//		result = CreateTable(inTemplate, inX, inY);
//	else if (name == "vbox" or name == "dialog" or name == "page")
//		result = CreateBox(inTemplate, inX, inY, false);
//	else if (name == "hbox")
//		result = CreateBox(inTemplate, inX, inY, true);
//	else if (name == "notebook")
//		result = CreateNotebook(inTemplate, inX, inY);
//	else if (name == "pager")
//		result = CreatePager(inTemplate, inX, inY);
//	else if (name == "listbox")
//		result = CreateListBox(inTemplate, inX, inY);
//	else if (name == "listview")
//		result = CreateListView(inTemplate, inX, inY);
//	else if (name == "filler")
//		result = new MView(inTemplate->get_attribute("id"), MRect(inX, inY, 0, 0));
//
//	MControlBase* control = dynamic_cast<MControlBase*>(result);
//
//	int32 marginLeft, marginTop, marginRight, marginBottom;
//	GetMargins(inTemplate, marginLeft, marginTop, marginRight, marginBottom);
////	result->SetMargins(marginLeft, marginTop, marginRight, marginBottom);
//
//	if (not inTemplate->get_attribute("width").empty())
//	{
//		int32 width = marginLeft + marginRight;
//		
//		if (inTemplate->get_attribute("width") == "scrollbarwidth")
//			width += kScrollbarWidth;
//		else
////			width += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->get_attribute("width")) * mDLUX);
//			width += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->get_attribute("width")));
//
//		MX11WidgetMixin* impl = dynamic_cast<MX11WidgetMixin*>(control->GetControlImplBase());
//		if (impl != nullptr)
//			impl->RequestSize(width * mDLUX, -1);
//	}
//	
////	if (not inTemplate->get_attribute("height").empty())
////	{
////		int32 height = marginTop + marginBottom;
////		
////		if (inTemplate->get_attribute("height") == "scrollbarheight")
////			height += kScrollbarWidth;
////		else
//////			height += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->get_attribute("height")) * mDLUY);
////			height += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->get_attribute("height")));
////		
////		MRect frame;
////		result->GetFrame(frame);
////		if (frame.height < height)
////			result->ResizeFrame(0, height - frame.height);
////	}
//
//	if (control != nullptr)
//	{
//		control->SetLayout(
//			inTemplate->get_attribute("packing") == "end" ? ePackEnd : ePackStart,
//			inTemplate->get_attribute("expand") == "true" ? true : false,
//			inTemplate->get_attribute("fill") == "true" ? true : false,
//			inTemplate->get_attribute("padding").empty() ? 0 : boost::lexical_cast<int32>(inTemplate->get_attribute("padding")));
//	}
//
//	return result;
//}
//
//uint32 MX11DialogImpl::GetTextWidth(const string& inText,
//	const wchar_t* inClass, int inPartID, int inStateID)
//{
//	uint32 result = 0;
////	wstring text(c2w(inText));
////	
////	HTHEME hTheme = ::OpenThemeData(GetHandle(), inClass);
////
////	if (hTheme != nullptr)
////	{
////		RECT r;
////		THROW_IF_HRESULT_ERROR(::GetThemeTextExtent(hTheme, mDC,
////			inPartID, inStateID, text.c_str(), text.length(), 0, nullptr, &r));
////		result = r.right - r.left;
////		::CloseThemeData(hTheme);
////	}
////	else
////	{
////		SIZE size;
////		::GetTextExtentPoint32(mDC, text.c_str(), text.length(), &size);
////		result = size.cx;
////	}
//	
//	return result;
//}
//
//
//
//
// --------------------------------------------------------------------

MWindowImpl* MWindowImpl::CreateDialog(const string& inResource, MWindow* inWindow)
{
//	return new MX11DialogImpl(inResource, inWindow);
	return nullptr;
}
