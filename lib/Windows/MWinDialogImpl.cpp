//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "zeep/xml/document.hpp"
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include "MWinWindowImpl.hpp"
#include "MWindow.hpp"
#include "MError.hpp"
#include "MWinApplicationImpl.hpp"
#include "MWinControlsImpl.hpp"
#include "MUtils.hpp"
#include "MWinUtils.hpp"
#include "MWinMenuImpl.hpp"
#include "MDevice.hpp"
#include "MResources.hpp"
#include "MAcceleratorTable.hpp"
#include "MControls.hpp"
#include "MDialog.hpp"
#include "MStrings.hpp"

using namespace std;
using namespace zeep;
namespace io = boost::iostreams;
namespace ba = boost::algorithm;

class MWinDialogImpl : public MWinWindowImpl
{
  public:
					MWinDialogImpl(const string& inResource, MWindow* inWindow);
	virtual			~MWinDialogImpl();

	virtual void	Finish();

	virtual bool	IsDialogMessage(MSG& inMesssage);

private:

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle, wstring& outClassName, HMENU& outMenu);
	virtual void	RegisterParams(UINT& outStyle, int& outWndExtra, HCURSOR& outCursor, HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground);

	void			GetMargins(xml::element* inTemplate,
						int32& outLeftMargin, int32& outTopMargin,
						int32& outRightMargin, int32& outBottomMargin);

	MView*			CreateControls(xml::element* inTemplate, int32 inX, int32 inY);

	MView*			CreateButton(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateColorSwatch(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateCaption(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateCheckbox(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateRadiobutton(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateExpander(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateCombobox(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateEdittext(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreatePopup(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateScrollbar(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateSeparator(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateVBox(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateHBox(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateTable(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateNotebook(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreatePager(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateListBox(xml::element* inTemplate, int32 inX, int32 inY);
	MView*			CreateListView(xml::element* inTemplate, int32 inX, int32 inY);
//	MView*			CreateTable(xml::element* inTemplate, int32 inX, int32 inY);

	uint32			GetTextWidth(const string& inText,
						const wchar_t* inClass, int inPartID, int inStateID);

	string			l(const string& s)					{ return GetLocalisedStringForContext(mRsrc, s); }

	string			mRsrc;
	HDC				mDC;
	float			mDLUX, mDLUY;
	MButton*		mOKButton;
	MButton*		mCancelButton;
	
	// TODO: improve this (separate radio groups)
	list<MRadiobutton*>
					mRadioGroup;
};

MWinDialogImpl::MWinDialogImpl(const string& inResource, MWindow* inWindow)
	: MWinWindowImpl(MWindowFlags(0), "", inWindow)
	, mRsrc(inResource)
	, mDC(nullptr)
	, mDLUX(1.75)
	, mDLUY(1.875)
	, mOKButton(nullptr)
	, mCancelButton(nullptr)
{
}

MWinDialogImpl::~MWinDialogImpl()
{
	::ReleaseDC(GetHandle(), mDC);
}

void MWinDialogImpl::CreateParams(DWORD& outStyle,
	DWORD& outExStyle, wstring& outClassName, HMENU& outMenu)
{
	MWinWindowImpl::CreateParams(outStyle, outExStyle, outClassName, outMenu);

	outClassName = L"MWinDialogImpl";
	if (mFlags & kMFixedSize)
		outStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	else
		outStyle = WS_OVERLAPPEDWINDOW;
	outExStyle = WS_EX_CONTROLPARENT;
	outMenu = nullptr;
}

void MWinDialogImpl::RegisterParams(UINT& outStyle, int& outWndExtra, HCURSOR& outCursor,
	HICON& outIcon, HICON& outSmallIcon, HBRUSH& outBackground)
{
	MWinWindowImpl::RegisterParams(outStyle, outWndExtra,
		outCursor, outIcon, outSmallIcon, outBackground);
	
	HINSTANCE inst = MWinApplicationImpl::GetInstance()->GetHInstance();
	
	outStyle = 0;
	outWndExtra = DLGWINDOWEXTRA;
	//outIcon = ::LoadIcon(inst, MAKEINTRESOURCE(ID_DEF_DOC_ICON));
	//outSmallIcon = ::LoadIcon(inst, MAKEINTRESOURCE(ID_DEF_DOC_ICON));
	outCursor = ::LoadCursor(NULL, IDC_ARROW);
	outBackground = (HBRUSH)(COLOR_BTNFACE + 1);
}

bool MWinDialogImpl::IsDialogMessage(MSG& inMessage)
{
	bool result = false;
	
	if (inMessage.message == WM_KEYDOWN and GetHandle() == ::GetForegroundWindow())
	{
		result = true;

		switch (inMessage.wParam)
		{
//			case VK_TAB:
//			case VK_SPACE:
//				break;
			
			case VK_ESCAPE:
				if (mCancelButton != nullptr)
					mCancelButton->SimulateClick();
				else
					mWindow->Close();
				break;
			
			case VK_RETURN:
			{
				MEdittext* edit = dynamic_cast<MEdittext*>(GetFocus());
				if (mOKButton != nullptr and (edit == nullptr or (edit->GetFlags() & eMEditTextMultiLine) == 0))
					mOKButton->SimulateClick();
				else
					result = false;
				break;
			}

			default:
			{
				LRESULT r;
				result = WMKeydown(GetHandle(), inMessage.message,
					inMessage.wParam, inMessage.lParam, r);
				if (result == false)
					result = ::IsDialogMessageW(GetHandle(), &inMessage) == TRUE;
				break;
			}
		}
	}
		
	return result;	
}

void MWinDialogImpl::Finish()
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
	
	wstring title = c2w(l(dialog->get_attribute("title")));

	mFlags = kMFixedSize;
	string flags = dialog->get_attribute("flags");
	if (ba::contains(flags, "flexible"))
		mFlags = MWindowFlags(mFlags & ~kMFixedSize);
	if (ba::contains(flags, "nosizebox"))
		mFlags = MWindowFlags(mFlags | kMNoSizeBox);

	uint32 minWidth = 40;
	if (not dialog->get_attribute("width").empty())
		minWidth = boost::lexical_cast<uint32>(dialog->get_attribute("width"));
	uint32 minHeight = 40;
	if (not dialog->get_attribute("height").empty())
		minHeight = boost::lexical_cast<uint32>(dialog->get_attribute("height"));

	MRect bounds(CW_USEDEFAULT, CW_USEDEFAULT, minWidth, minHeight);

	// now create the dialog
	MWinWindowImpl::Create(bounds, title);

	// now we have the handle, get the DC and theme font
	mDC = ::GetDC(GetHandle());

	HTHEME hTheme = ::OpenThemeData(GetHandle(), VSCLASS_TEXTSTYLE);

	TEXTMETRIC tm;
	if (hTheme != nullptr)
	{
		::GetThemeTextMetrics(hTheme, mDC, TEXT_BODYTEXT, TS_CONTROLLABEL_NORMAL, &tm);

		RECT r;
		THROW_IF_HRESULT_ERROR(::GetThemeTextExtent(hTheme, mDC, TEXT_BODYTEXT, TS_CONTROLLABEL_NORMAL, 
			L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, 0, nullptr, &r));

		mDLUY = tm.tmHeight / 8.0f;
		mDLUX = (r.right - r.left) / (52 * 4.0f);

		::CloseThemeData(hTheme);
	}
	else
	{
		::SelectObject(mDC, ::GetStockObject(DEFAULT_GUI_FONT));
		::GetTextMetrics(mDC, &tm);

		SIZE size;
		::GetTextExtentPoint32(mDC, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);

		mDLUY = tm.tmHeight / 8.0f;
		mDLUX = size.cx / (52 * 4.0f);
	}

	// create the dialog controls, all stacked on top of each other
	MView* content = CreateControls(dialog, 0, 0);
	content->SetBindings(true, true, true, true);

	for (MRadiobutton* radiobutton : mRadioGroup)
		radiobutton->SetGroup(mRadioGroup);

	RECT cr;
	::GetClientRect(GetHandle(), &cr);

	// final update
	//content->CalculateFrame(bounds);
	content->GetFrame(bounds);

	int32 dw = bounds.width - (cr.right - cr.left);		if (dw < 0) dw = 0;
	int32 dh = bounds.height - (cr.bottom - cr.top);	if (dh < 0) dh = 0;

	if (dw > 0 or dh > 0)
	{
		MRect p;
		mWindow->GetWindowPosition(p);
//		p.x -= dw / 2;
//		p.y -= dh / 2;
		p.width += dw;
		p.height += dh;
		mWindow->SetWindowPosition(p);
	}

	mWindow->AddChild(content);
}

void MWinDialogImpl::GetMargins(xml::element* inTemplate,
	int32& outLeftMargin, int32& outTopMargin,
	int32& outRightMargin, int32& outBottomMargin)
{
	outLeftMargin = outTopMargin = outRightMargin = outBottomMargin = 0;
	
	if (inTemplate->name() == "dialog" or inTemplate->name() == "notebook")
		outLeftMargin = outTopMargin = outRightMargin = outBottomMargin = 7;
	
	string m = inTemplate->get_attribute("margin");
	if (not m.empty())
		outLeftMargin = outRightMargin =
		outTopMargin = outBottomMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->get_attribute("margin-left-right");
	if (not m.empty())
		outLeftMargin = outRightMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->get_attribute("margin-top-bottom");
	if (not m.empty())
		outTopMargin = outBottomMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->get_attribute("margin-left");
	if (not m.empty())
		outLeftMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->get_attribute("margin-top");
	if (not m.empty())
		outTopMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->get_attribute("margin-right");
	if (not m.empty())
		outRightMargin = boost::lexical_cast<int32>(m);

	m = inTemplate->get_attribute("margin-bottom");
	if (not m.empty())
		outBottomMargin = boost::lexical_cast<int32>(m);

	outLeftMargin = static_cast<int32>(outLeftMargin * mDLUX);
	outRightMargin = static_cast<int32>(outRightMargin * mDLUX);
	outTopMargin = static_cast<int32>(outTopMargin * mDLUY);
	outBottomMargin = static_cast<int32>(outBottomMargin * mDLUY);
}

MView* MWinDialogImpl::CreateButton(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));
	
	float idealWidth = GetTextWidth(title, VSCLASS_BUTTON, BP_PUSHBUTTON, PBS_NORMAL) + 10 * mDLUX;
	if (idealWidth < 50 * mDLUX)
		idealWidth = 50 * mDLUX;
	MRect bounds(inX, inY, static_cast<int32>(idealWidth), static_cast<int32>(14 * mDLUY));

	MButtonFlags flags = eBF_None;
	
	if (inTemplate->get_attribute("split") == "true")
		flags = eBF_Split;

	MButton* button = new MButton(id, bounds, title, flags);
	
	if (inTemplate->get_attribute("default") == "true")
	{
		button->MakeDefault(true);
		mOKButton = button;
	}

	if (id == "ok" and mOKButton == nullptr)
		mOKButton = button;
	
	if (id == "cancel")
		mCancelButton = button;

	AddRoute(button->eClicked, static_cast<MDialog*>(mWindow)->eButtonClicked);

	return button;
}

MView* MWinDialogImpl::CreateColorSwatch(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	
	MRect bounds(inX, inY, static_cast<int32>(25 * mDLUX), static_cast<int32>(14 * mDLUY));

	MColor color(inTemplate->get_attribute("color").c_str());
	MColorSwatch* swatch = new MColorSwatch(id, bounds, color);
	
	AddRoute(swatch->eColorChanged, static_cast<MDialog*>(mWindow)->eColorChanged);

	return swatch;
}

MView* MWinDialogImpl::CreateExpander(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));
	
	MRect bounds(inX, inY,
		static_cast<int32>((13 + 3) * mDLUX) +
			GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_LABEL, 0),
		static_cast<int32>(12 * mDLUY));

	MExpander* expander = new MExpander(id, bounds, title);
	AddRoute(expander->eClicked, static_cast<MDialog*>(mWindow)->eButtonClicked);

	return expander;
}

MView* MWinDialogImpl::CreateCaption(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	if (id.empty())
		id = "caption";
	string text = l(inTemplate->get_attribute("text"));

	MRect bounds(inX, static_cast<int32>(inY), 0, static_cast<int32>(10 * mDLUY));
	bounds.width = GetTextWidth(text, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	return new MCaption(id, bounds, text);
}

MView* MWinDialogImpl::CreateCheckbox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));

	MRect bounds(inX, inY, 0, static_cast<int32>(10 * mDLUY));
	bounds.width = static_cast<int32>(14 * mDLUX) +
		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
		//GetTextWidth(title, VSCLASS_BUTTON, BP_CHECKBOX, PBS_NORMAL);

	MCheckbox* checkbox = new MCheckbox(id, bounds, title);
	AddRoute(checkbox->eValueChanged,
		static_cast<MDialog*>(mWindow)->eCheckboxClicked);
	return checkbox;
}

MView* MWinDialogImpl::CreateRadiobutton(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));

	MRect bounds(inX, inY, 0, static_cast<int32>(10 * mDLUY));
	bounds.width = static_cast<int32>(14 * mDLUX) +
		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
//		GetTextWidth(title, VSCLASS_BUTTON, BP_RADIOBUTTON, PBS_NORMAL);
	MRadiobutton* radiobutton = new MRadiobutton(id, bounds, title);
	AddRoute(radiobutton->eValueChanged,
		static_cast<MDialog*>(mWindow)->eRadiobuttonClicked);

	mRadioGroup.push_back(radiobutton);

	return radiobutton;
}

MView* MWinDialogImpl::CreateCombobox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	MRect bounds(inX, inY, static_cast<int32>(50 * mDLUX), static_cast<int32>(14 * mDLUY));
	MCombobox* combobox = new MCombobox(id, bounds);
	AddRoute(combobox->eValueChanged,
		static_cast<MDialog*>(mWindow)->eTextChanged);
	return combobox;
}

MView* MWinDialogImpl::CreateEdittext(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	
	uint32 flags = eMEditTextNoFlags;
	if (ba::contains(inTemplate->get_attribute("style"), "right"))
		flags |= eMEditTextAlignRight;
	if (ba::contains(inTemplate->get_attribute("style"), "number"))
		flags |= eMEditTextNumbers;
	if (ba::contains(inTemplate->get_attribute("style"), "multiline"))
		flags |= eMEditTextMultiLine;
	if (ba::contains(inTemplate->get_attribute("style"), "readonly"))
		flags |= eMEditTextReadOnly;

	MRect bounds(inX, inY, static_cast<int32>(5 * mDLUX), static_cast<int32>(14 * mDLUY));
	MEdittext* edittext = new MEdittext(id, bounds, flags);
	AddRoute(edittext->eValueChanged,
		static_cast<MDialog*>(mWindow)->eTextChanged);
	return edittext;
}

MView* MWinDialogImpl::CreatePopup(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	MRect bounds(inX, inY, 0, static_cast<int32>(14 * mDLUY));
	
	vector<string> choices;
	for (xml::element* option: inTemplate->find("./option"))
	{
		string label = option->content();
		int32 width = GetTextWidth(label, VSCLASS_COMBOBOX, CP_DROPDOWNBUTTON, CBXSL_NORMAL);
		if (bounds.width < width)
			bounds.width = width;
		choices.push_back(label);
	}

	bounds.width += static_cast<int32>(14 * mDLUX);

	MPopup* popup = new MPopup(id, bounds);

	popup->SetChoices(choices);
	AddRoute(popup->eValueChanged,
		static_cast<MDialog*>(mWindow)->eValueChanged);

	return popup;
}

MView* MWinDialogImpl::CreateNotebook(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MNotebook* result = new MNotebook(id, r);
	
	MRect b;
	
	for (xml::element* page: inTemplate->find("./page"))
	{
		string title = l(page->get_attribute("title"));
		
		MView* control = CreateControls(page, 0, 0);
		control->SetBindings(true, true, true, true);
		result->AddPage(title, control);
		
		control->RecalculateLayout();
		
		MRect f;
		control->GetFrame(f);
		b |= f;
	}

	// calculate a new width/height for our tab control
	HTHEME hTheme = ::OpenThemeData(GetHandle(), VSCLASS_TAB);
	if (hTheme != nullptr)
	{
		RECT r = { 0, 0, b.width, b.height };

		RECT extend;
		::GetThemeBackgroundContentRect(hTheme, mDC, TABP_PANE, 0, &r, &extend);

//		h += h - (extend.bottom - extend.top);
//		w += w - (extend.right - extend.left);

		SIZE size;
		::GetThemePartSize(hTheme, mDC, TABP_TABITEM, TTIS_NORMAL, &extend, TS_TRUE, &size);

		::CloseThemeData(hTheme);

		//h += size.cy;
		//w += size.cx;
	}

	r.width = b.width;
	r.height = b.height;

	result->SetFrame(r);
	result->SelectPage(0);
	
	return result;
}

MView* MWinDialogImpl::CreatePager(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

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

MView* MWinDialogImpl::CreateListBox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MListBox* result = new MListBox(id, r);
	
	for (xml::element* listitem: inTemplate->find("./listitem"))
	{
		string text = l(listitem->content());
		int32 textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
		if (r.width < textWidth)
			r.width = textWidth;
		result->AddItem(text);
	}
	
	r.width += static_cast<int32>(mDLUX * 6);
	result->SetFrame(r);

	AddRoute(result->eValueChanged, static_cast<MDialog*>(mWindow)->eValueChanged);
	
	return result;
}

MView* MWinDialogImpl::CreateListView(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MListView* result = new MListView(id, r);
	
	for (xml::element* listitem: inTemplate->find("./listitem"))
	{
		string text = l(listitem->content());
		int32 textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
		if (r.width < textWidth)
			r.width = textWidth;
		result->AddItem(text);
	}
	
	r.width += static_cast<int32>(mDLUX * 6);
	result->SetFrame(r);

	AddRoute(result->eValueChanged, static_cast<MDialog*>(mWindow)->eValueChanged);

	return result;
}

MView* MWinDialogImpl::CreateSeparator(xml::element* inTemplate, int32 inX, int32 inY)
{
	MRect bounds(inX, inY, 2, 2);
	return new MSeparator("separator", bounds);
}

MView* MWinDialogImpl::CreateScrollbar(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");
	string orientation = inTemplate->get_attribute("orientation");

	MRect bounds(inX, inY, kScrollbarWidth, kScrollbarWidth);
	
	if (orientation == "horizontal")
		bounds.width *= 2;
	else
		bounds.height *= 2;
	
	return new MScrollbar(id, bounds);
}

MView* MWinDialogImpl::CreateVBox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	uint32 spacing = 4;
	if (not inTemplate->get_attribute("spacing").empty())
		spacing = boost::lexical_cast<uint32>(inTemplate->get_attribute("spacing"));

	MRect r(inX, inY, 0, 0);
	MView* result = new MVBox(id, r, static_cast<int32>(spacing * mDLUY));
	
	for (xml::element* b: inTemplate->children<xml::element>())
		result->AddChild(CreateControls(b, 0, 0));
	
	return result;
}

MView* MWinDialogImpl::CreateHBox(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	uint32 spacing = 4;
	if (not inTemplate->get_attribute("spacing").empty())
		spacing = boost::lexical_cast<uint32>(inTemplate->get_attribute("spacing"));

	MRect r(inX, inY, 0, 0);
	MView* result = new MHBox(id, r, static_cast<int32>(spacing * mDLUX));
	
	for (xml::element* b: inTemplate->children<xml::element>())
		result->AddChild(CreateControls(b, 0, 0));
	
	return result;
}

MView* MWinDialogImpl::CreateTable(xml::element* inTemplate, int32 inX, int32 inY)
{
	string id = inTemplate->get_attribute("id");

	vector<MView*> views;
	uint32 colCount = 0, rowCount = 0;
	
	for (xml::element* row: inTemplate->find("./row"))
	{
		uint32 cn = 0;
		
		for (xml::element* col: row->children<xml::element>())
		{
			++cn;
			if (colCount < cn)
				colCount = cn;
			views.push_back(CreateControls(col, 0, 0));
		}
		
		++rowCount;
	}

	// fix me!
	while (views.size() < (rowCount * colCount))
		views.push_back(nullptr);
	
	MRect r(inX, inY, 0, 0);
	MTable* result = new MTable(id, r,
		&views[0], colCount, rowCount, static_cast<int32>(4 * mDLUX), static_cast<int32>(4 * mDLUY));
	
	return result;
}

MView* MWinDialogImpl::CreateControls(xml::element* inTemplate, int32 inX, int32 inY)
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
		result = CreateVBox(inTemplate, inX, inY);
	else if (name == "hbox")
		result = CreateHBox(inTemplate, inX, inY);
	else if (name == "notebook")
		result = CreateNotebook(inTemplate, inX, inY);
	else if (name == "pager")
		result = CreatePager(inTemplate, inX, inY);
	else if (name == "listbox")
		result = CreateListBox(inTemplate, inX, inY);
	else if (name == "listview")
		result = CreateListView(inTemplate, inX, inY);
	else if (name == "filler")
		result = new MView(inTemplate->get_attribute("id"), MRect(inX, inY, 0, 0));

	int32 marginLeft, marginTop, marginRight, marginBottom;
	GetMargins(inTemplate, marginLeft, marginTop, marginRight, marginBottom);
	result->SetMargins(marginLeft, marginTop, marginRight, marginBottom);

	if (not inTemplate->get_attribute("width").empty())
	{
		int32 width = marginLeft + marginRight;
		
		if (inTemplate->get_attribute("width") == "scrollbarwidth")
			width += kScrollbarWidth;
		else
			width += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->get_attribute("width")) * mDLUX);
		
		MRect frame;
		result->GetFrame(frame);
		if (frame.width < width)
			result->ResizeFrame(width - frame.width, 0);
	}

	if (not inTemplate->get_attribute("height").empty())
	{
		int32 height = marginTop + marginBottom;
		
		if (inTemplate->get_attribute("height") == "scrollbarheight")
			height += kScrollbarWidth;
		else
			height += static_cast<int32>(boost::lexical_cast<int32>(inTemplate->get_attribute("height")) * mDLUY);
		
		MRect frame;
		result->GetFrame(frame);
		if (frame.height < height)
			result->ResizeFrame(0, height - frame.height);
	}

	string bindings = inTemplate->get_attribute("bind");
	result->SetBindings(
		ba::contains(bindings, "left"),
		ba::contains(bindings, "top"),
		ba::contains(bindings, "right"),
		ba::contains(bindings, "bottom"));

	return result;
}

uint32 MWinDialogImpl::GetTextWidth(const string& inText,
	const wchar_t* inClass, int inPartID, int inStateID)
{
	uint32 result = 0;
	wstring text(c2w(inText));
	
	HTHEME hTheme = ::OpenThemeData(GetHandle(), inClass);

	if (hTheme != nullptr)
	{
		RECT r;
		THROW_IF_HRESULT_ERROR(::GetThemeTextExtent(hTheme, mDC,
			inPartID, inStateID, text.c_str(), text.length(), 0, nullptr, &r));
		result = r.right - r.left;
		::CloseThemeData(hTheme);
	}
	else
	{
		SIZE size;
		::GetTextExtentPoint32(mDC, text.c_str(), text.length(), &size);
		result = size.cx;
	}
	
	return result;
}

// --------------------------------------------------------------------

MWindowImpl* MWindowImpl::CreateDialog(const string& inResource, MWindow* inWindow)
{
	return new MWinDialogImpl(inResource, inWindow);
}
