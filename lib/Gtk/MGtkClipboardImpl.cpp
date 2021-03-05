//          Copyright Maarten L. Hekkelman 2006-2014
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include <boost/algorithm/string.hpp>

#include "MClipboardImpl.hpp"
#include "MGtkWidgetMixin.hpp"
#include "MUtils.hpp"

using namespace std;
namespace ba = boost::algorithm;

class MGtkClipboardImpl : public MClipboardImpl
{
  public:
	MGtkClipboardImpl(MClipboard* inClipboard);

	virtual void LoadClipboardIfNeeded();
	virtual void Reset();
	virtual void Commit();

	void OnOwnerChange(GdkEventOwnerChange*inEvent)
	{
		if (not mClipboardIsMine)
			mOwnerChanged = true;
	}
	
	static void GtkClipboardGet(GtkClipboard* inClipboard, GtkSelectionData* inSelectionData,
					guint inInfo, gpointer inUserDataOrOwner);
	static void GtkClipboardClear(GtkClipboard* inClipboard, gpointer inUserDataOrOwner);
	
	MSlot<void(GdkEventOwnerChange*)> mOwnerChange;
	GtkClipboard*	mGtkClipboard;
	bool mClipboardIsMine;
	bool mOwnerChanged;
};

MGtkClipboardImpl::MGtkClipboardImpl(MClipboard* inClipboard)
	: MClipboardImpl(inClipboard)
	, mOwnerChange(this, &MGtkClipboardImpl::OnOwnerChange)
	, mGtkClipboard(gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD))
	, mClipboardIsMine(false)
	, mOwnerChanged(false)
{
	mOwnerChange.Connect(G_OBJECT(mGtkClipboard), "owner-change");
}

void MGtkClipboardImpl::LoadClipboardIfNeeded()
{
	if (not mClipboardIsMine and
		mOwnerChanged and
		gtk_clipboard_wait_is_text_available(mGtkClipboard))
	{
//cout << "Reloading clipboard" << endl;
		gchar* text = gtk_clipboard_wait_for_text(mGtkClipboard);
		if (text != nullptr)
		{
			mClipboard->SetData(text, false);
			g_free(text);
		}
		mOwnerChanged = false;
	}
}

void MGtkClipboardImpl::Reset()
{
}

void MGtkClipboardImpl::Commit()
{
	GtkTargetEntry targets[] = {
		{ const_cast<gchar*>("UTF8_STRING"), 0, 0 },
		{ const_cast<gchar*>("COMPOUND_TEXT"), 0, 0 },
		{ const_cast<gchar*>("TEXT"), 0, 0 },
		{ const_cast<gchar*>("STRING"), 0, 0 },
	};

	gtk_clipboard_set_with_data(mGtkClipboard, 
		targets, sizeof(targets) / sizeof(GtkTargetEntry),
		&MGtkClipboardImpl::GtkClipboardGet, &MGtkClipboardImpl::GtkClipboardClear, this);
	
//	gtk_clipboard_set_text(mGtkClipboard, inText.c_str(), inText.length());
	
	mOwnerChanged = false;
	mClipboardIsMine = true;
}

void MGtkClipboardImpl::GtkClipboardGet(GtkClipboard* inClipboard, GtkSelectionData* inSelectionData,
					guint inInfo, gpointer inUserDataOrOwner)
{
	string text;
	bool block;

	MClipboard::Instance().GetData(text, block);
	
	gtk_selection_data_set_text(inSelectionData, text.c_str(), text.length());
}

void MGtkClipboardImpl::GtkClipboardClear(GtkClipboard* inClipboard, gpointer inUserDataOrOwner)
{
	MGtkClipboardImpl* self = reinterpret_cast<MGtkClipboardImpl*>(inUserDataOrOwner);
	
	self->mOwnerChanged = true;
	self->mClipboardIsMine = false;
}

MClipboardImpl* MClipboardImpl::Create(MClipboard* inClipboard)
{
	return new MGtkClipboardImpl(inClipboard);
}
