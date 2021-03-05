// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#include "MSalt.hpp"

#include <boost/bind.hpp>

#include "MExploreBrowserWindow.hpp"
#include "MExploreBrowserView.hpp"
#include "MStrings.hpp"

using namespace std;

MExploreBrowserWindow::MExploreBrowserWindow(std::shared_ptr<pinch::basic_connection> inConnection)
	: MWindow("Explore", MRect(0, 0, 640, 480),
		 MWindowFlags(kMPostionDefault | kMNoEraseOnUpdate),
		"terminal-window-menu")
	, mSFTPChannel(inConnection)
	, mBrowserView(nullptr)
{
	MRect bounds;

	// create status bar
	GetBounds(bounds);
	bounds.y += bounds.height - kScrollbarWidth;
	bounds.height = kScrollbarWidth;

	int32_t partWidths[4] = { 250, 250, 45, -1 };
	mStatusbar = new MStatusbar("status", bounds, 4, partWidths);
	AddChild(mStatusbar);
	mStatusbar->GetBounds(bounds);

	int32_t statusbarHeight = bounds.height;
	
	// create browse view
	GetBounds(bounds);
	bounds.height -= statusbarHeight;
	
	mBrowserView = MExploreBrowserView::Create("browser", bounds);
	mBrowserView->SetBindings(true, true, true, true);
	AddChild(mBrowserView);
	
	mStatusbar->SetStatusText(0, _("Trying to connect"), false);
	mSFTPChannel.open(boost::bind(&MExploreBrowserWindow::ChannelOpened, this, _1));
}

MExploreBrowserWindow::~MExploreBrowserWindow()
{
	
}

void MExploreBrowserWindow::ChannelOpened(const boost::system::error_code& ec)
{
	if (ec)
		DisplayError(ec);
	else
	{
		mStatusbar->SetStatusText(0, _("Connected"), false);
		mStatusbar->SetStatusText(1, mSFTPChannel.get_connection_parameters(pinch::client2server), false);
		
		mSFTPChannel.read_dir(".", boost::bind(&MExploreBrowserWindow::ReadDir, this, _1, _2, _3, _4));
	}
}

bool MExploreBrowserWindow::ReadDir(const boost::system::error_code& ec,
	const string& inName, const string& inLongName,
	const pinch::sftp_channel::file_attributes& attr)
{
	Item item = { inName, inLongName, attr };
	mItems.push_back(item);

	mBrowserView->AddItem(inName, boost::lexical_cast<string>(attr.uid), attr.size, attr.mtime);

	return true;
}

bool MExploreBrowserWindow::AllowClose(bool inQuitting)
{
	return true;
}

void MExploreBrowserWindow::Close()
{
	mSFTPChannel.close();
	MWindow::Close();
}
