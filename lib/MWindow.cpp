//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <iostream>

#include "zeep/xml/document.hpp"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include "MCommands.hpp"
#include "MWindow.hpp"
#include "MError.hpp"
#include "MApplication.hpp"
#include "MWindowImpl.hpp"
#include "MMenu.hpp"
#include "MResources.hpp"
#include "MUtils.hpp"

using namespace std;
using namespace zeep;
namespace io = boost::iostreams;

// --------------------------------------------------------------------
//
//	MWindowImpl
//

MMenuBar* MWindowImpl::CreateMenu(const std::string& inMenu)
{
	string resource = string("Menus/") + inMenu + ".xml";
	mrsrc::rsrc rsrc(resource);
	
	if (not rsrc)
		THROW(("Menu resource not found: %s", resource.c_str()));

	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
	
	xml::document doc(data);

	return MMenuBar::Create(&doc.front());
}

// --------------------------------------------------------------------
//
//	MWindow
//

list<MWindow*> MWindow::sWindowList;

MWindow::MWindow(const string& inTitle, const MRect& inBounds,
		MWindowFlags inFlags, const string& inMenu)
	: MView("window", inBounds)
	, MHandler(gApp)
	, mImpl(MWindowImpl::Create(inTitle, inBounds, inFlags, inMenu, this))
	, mModified(false)
	, mLatentFocus(this)
{
	mVisible = eTriStateLatent;
	
	mBounds.x = mBounds.y = 0;

	SetBindings(true, true, true, true);
	
	sWindowList.push_back(this);
}

MWindow::MWindow(MWindowImpl* inImpl)
	: MView("window", MRect(0, 0, 100, 100))
	, MHandler(gApp)
	, mImpl(inImpl)
	, mLatentFocus(this)
{
	SetBindings(true, true, true, true);

	sWindowList.push_back(this);
}

MWindow::~MWindow()
{
	delete mImpl;

	sWindowList.erase(
		remove(sWindowList.begin(), sWindowList.end(), this),
		sWindowList.end());
}

void MWindow::SetImpl(
	MWindowImpl*	inImpl)
{
	if (mImpl != nullptr)
		delete mImpl;
	mImpl = inImpl;
}

void MWindow::Mapped()
{
	SuperShow();
}

void MWindow::Unmapped()
{
	SuperHide();
}

MWindowFlags MWindow::GetFlags() const
{
	return mImpl->GetFlags();
}

MWindow* MWindow::GetFirstWindow()
{
	MWindow* result = nullptr;
	if (not sWindowList.empty())
		result = sWindowList.front();
	return result;
}

MWindow* MWindow::GetNextWindow() const
{
	MWindow* result = nullptr;

	list<MWindow*>::const_iterator w =
		find(sWindowList.begin(), sWindowList.end(), this);

	if (w != sWindowList.end())
	{
		++w;
		if (w != sWindowList.end())
			result = *w;
	}

	return result;
}

MWindow* MWindow::GetWindow() const
{
	return const_cast<MWindow*>(this);
}

void MWindow::Show()
{
	mVisible = eTriStateOn;
	ShowSelf();
	
	MView::Show();
}

void MWindow::ShowSelf()
{
	mImpl->Show();
}

void MWindow::HideSelf()
{
	mImpl->Hide();
}

void MWindow::Select()
{
	if (not mImpl->Visible())
		Show();
	mImpl->Select();
}

void MWindow::Activate()
{
	if (mActive == eTriStateOff and IsVisible())
	{
		mActive = eTriStateOn;
		ActivateSelf();
		MView::Activate();

		if (mLatentFocus != nullptr)
			mLatentFocus->SetFocus();
	}

	if (not sWindowList.empty() and sWindowList.front() != this)
	{
		if (sWindowList.front()->IsActive())
			sWindowList.front()->Deactivate();

		sWindowList.erase(find(sWindowList.begin(), sWindowList.end(), this));
		sWindowList.push_front(this);
	}
}

void MWindow::ActivateSelf()
{
}

void MWindow::UpdateNow()
{
	mImpl->UpdateNow();
}

bool MWindow::AllowClose(bool inQuit)
{
	return true;
}

void MWindow::Close()
{
	if (mImpl != nullptr)
		mImpl->Close();
}

void MWindow::SetTitle(
	const string&	inTitle)
{
	mTitle = inTitle;
	
	if (mModified)
		mImpl->SetTitle(mTitle + "*");
	else
		mImpl->SetTitle(mTitle);
}

string MWindow::GetTitle() const
{
	return mTitle;
}

void MWindow::SetModifiedMarkInTitle(
	bool		inModified)
{
	if (mModified != inModified)
	{
		mModified = inModified;
		SetTitle(mTitle);
	}
}

void MWindow::SetTransparency(
	float			inAlpha)
{
	mImpl->SetTransparency(inAlpha);
}

void MWindow::SetLatentFocus(
	MHandler*		inFocus)
{
	mLatentFocus = inFocus;

//	if (dynamic_cast<MView*>(inFocus) != nullptr)
//		mImpl->SetFocus(dynamic_cast<MView*>(inFocus));
}

void MWindow::BeFocus()
{
	if (mLatentFocus != nullptr and mLatentFocus != this)
		mLatentFocus->SetFocus();
}

bool MWindow::UpdateCommandStatus(
	uint32_t			inCommand,
	MMenu*			inMenu,
	uint32_t			inItemIndex,
	bool&			outEnabled,
	bool&			outChecked)
{
	bool result = true;	
	
	switch (inCommand)
	{
		case cmd_Close:
			outEnabled = true;
			break;
		
		default:
			result = MHandler::UpdateCommandStatus(
				inCommand, inMenu, inItemIndex, outEnabled, outChecked);
	}
	
	return result;
}

bool MWindow::ProcessCommand(
	uint32_t			inCommand,
	const MMenu*	inMenu,
	uint32_t			inItemIndex,
	uint32_t			inModifiers)
{
	bool result = true;	
	
	switch (inCommand)
	{
		case cmd_Close:
			if (AllowClose(false))
				Close();
			break;
		
		default:
			result = MHandler::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}
	
	return result;
}

void MWindow::ResizeFrame(
	int32_t			inWidthDelta,
	int32_t			inHeightDelta)
{
	ResizeWindow(inWidthDelta, inHeightDelta);
	//MView::ResizeFrame(0, 0);

	//MRect frame;
	//CalculateFrame(frame);
	//if (frame != mFrame)
		//mImpl->ResizeWindow(frame.width - mFrame.width, frame.height - mFrame.height);
}

void MWindow::ResizeWindow(
	int32_t			inWidthDelta,
	int32_t			inHeightDelta)
{
	mImpl->ResizeWindow(inWidthDelta, inHeightDelta);
}

void MWindow::GetWindowPosition(
	MRect&			outPosition)
{
	mImpl->GetWindowPosition(outPosition);
}

void MWindow::SetWindowPosition(
	const MRect&	inPosition,
	bool			inTransition)
{
	mImpl->SetWindowPosition(inPosition, inTransition);
}

void MWindow::ConvertToScreen(int32_t& ioX, int32_t& ioY) const
{
	mImpl->ConvertToScreen(ioX, ioY);
}

void MWindow::ConvertFromScreen(int32_t& ioX, int32_t& ioY) const
{
	mImpl->ConvertFromScreen(ioX, ioY);
}

void MWindow::GetMouse(int32_t& outX, int32_t& outY, uint32_t& outModifiers) const
{
	mImpl->GetMouse(outX, outY, outModifiers);
}

uint32_t MWindow::GetModifiers() const
{
	return mImpl->GetModifiers();
}

void MWindow::Invalidate(MRect inRect)
{
	mImpl->Invalidate(inRect);
}

void MWindow::ScrollRect(MRect inRect, int32_t inX, int32_t inY)
{
	mImpl->ScrollRect(inRect, inX, inY);
}

void MWindow::SetCursor(
	MCursor			inCursor)
{
	mImpl->SetCursor(inCursor);
}

void MWindow::ObscureCursor()
{
	mImpl->ObscureCursor();
}
