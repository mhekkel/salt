//          Copyright Maarten L. Hekkelman 2006-2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkLib.hpp"

#include "MGtkWidgetMixin.hpp"

#include "MTypes.hpp"
#include "MPrimary.hpp"
#include "MUnicode.hpp"
#include "MError.hpp"

using namespace std;

struct MPrimaryImpl
{
						MPrimaryImpl();
						~MPrimaryImpl();

	bool				HasText();
	void				GetText(string& outText);
	void				SetText(const string& inText);
	void				SetText(boost::function<void(std::string&)> provider);
	void				LoadClipboardIfNeeded();

	static void			GtkClipboardGet(
							GtkClipboard*		inClipboard,
							GtkSelectionData*	inSelectionData,
							guint				inInfo,
							gpointer			inUserDataOrOwner);
	
	static void			GtkClipboardClear(
							GtkClipboard*		inClipboard,
							gpointer			inUserDataOrOwner);

	void				OnOwnerChange(
							GdkEventOwnerChange*inEvent);
	
	string										mText;
	boost::function<void(string&)>				mProvider;
	MSlot<void(GdkEventOwnerChange*)>			mOwnerChange;
	GtkClipboard*								mGtkClipboard;
	bool										mClipboardIsMine;
	bool										mOwnerChanged;
	bool										mSettingOwner;
};

MPrimaryImpl::MPrimaryImpl()
	: mOwnerChange(this, &MPrimaryImpl::OnOwnerChange)
	, mGtkClipboard(gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY))
	, mClipboardIsMine(false)
	, mOwnerChanged(true)
{
	mOwnerChange.Connect(G_OBJECT(mGtkClipboard), "owner-change");
}

MPrimaryImpl::~MPrimaryImpl()
{
	if (mClipboardIsMine)
		gtk_clipboard_store(mGtkClipboard);
}

void MPrimaryImpl::GtkClipboardGet(
	GtkClipboard*		inClipboard,
	GtkSelectionData*	inSelectionData,
	guint				inInfo,
	gpointer			inUserDataOrOwner)
{
	MPrimaryImpl* self = reinterpret_cast<MPrimaryImpl*>(inUserDataOrOwner);
	
	if (self->mText.empty() and not self->mProvider.empty())
	{
		self->mProvider(self->mText);
		self->mProvider.clear();
	}
	
	gtk_selection_data_set_text(inSelectionData, self->mText.c_str(), self->mText.length());
}

void MPrimaryImpl::GtkClipboardClear(
	GtkClipboard*		inClipboard,
	gpointer			inUserDataOrOwner)
{
	MPrimaryImpl* self = reinterpret_cast<MPrimaryImpl*>(inUserDataOrOwner);

	if (self->mClipboardIsMine and not self->mSettingOwner)
	{
		self->mOwnerChanged = true;
		self->mClipboardIsMine = false;
		self->mText.clear();
		self->mProvider.clear();
	}
}

void MPrimaryImpl::OnOwnerChange(
	GdkEventOwnerChange*inEvent)
{
	if (not mClipboardIsMine and not mSettingOwner)
	{
		mOwnerChanged = true;
		mText.clear();
		mProvider.clear();
	}
}

void MPrimaryImpl::LoadClipboardIfNeeded()
{
	if (not mClipboardIsMine and
		mOwnerChanged and
		gtk_clipboard_wait_is_text_available(mGtkClipboard))
	{
		gchar* text = gtk_clipboard_wait_for_text(mGtkClipboard);
		if (text != nullptr)
		{
			SetText(string(text));
			g_free(text);
		}
		mOwnerChanged = false;
	}
}

bool MPrimaryImpl::HasText()
{
	LoadClipboardIfNeeded();
	return not (mText.empty() and mProvider.empty());
}

void MPrimaryImpl::GetText(string& outText)
{
	if (not mText.empty())
		outText = mText;
	else if (not mProvider.empty())
		mProvider(outText);
}

void MPrimaryImpl::SetText(const string& inText)
{
	mText = inText;
	mProvider.clear();
	
	GtkTargetEntry targets[] = {
		{ const_cast<gchar*>("UTF8_STRING"), 0, 0 },
		{ const_cast<gchar*>("COMPOUND_TEXT"), 0, 0 },
		{ const_cast<gchar*>("TEXT"), 0, 0 },
		{ const_cast<gchar*>("STRING"), 0, 0 },
	};

	mSettingOwner = true;
	gtk_clipboard_set_with_data(mGtkClipboard,
		targets, sizeof(targets) / sizeof(GtkTargetEntry),
		&MPrimaryImpl::GtkClipboardGet, &MPrimaryImpl::GtkClipboardClear, this);

	mSettingOwner = false;
	mOwnerChanged = false;
	mClipboardIsMine = true;
}

void MPrimaryImpl::SetText(boost::function<void(string&)> provider)
{
	SetText(string(""));
	mProvider = provider;
}

// --------------------------------------------------------------------

MPrimary& MPrimary::Instance()
{
	static MPrimary sInstance;
	return sInstance;
}

MPrimary::MPrimary()
	: mImpl(new MPrimaryImpl)
{
}

MPrimary::~MPrimary()
{
	delete mImpl;
}

bool MPrimary::HasText()
{
	return mImpl->HasText();
}

void MPrimary::GetText(string& text)
{
	mImpl->GetText(text);
}

void MPrimary::SetText(const string& text)
{
	mImpl->SetText(text);
}

void MPrimary::SetText(boost::function<void(std::string&)> provider)
{
	mImpl->SetText(provider);
}
