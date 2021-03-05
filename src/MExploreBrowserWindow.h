// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include <assh/sftp_channel.hpp>

#include "MWindow.h"
#include "MControls.h"

class MExploreBrowserView;

class MExploreBrowserWindow : public MWindow
{
  public:
					MExploreBrowserWindow(assh::basic_connection* inConnection);
					~MExploreBrowserWindow();

	virtual void	Close();
	virtual bool	AllowClose(bool inQuitting);

  private:
	
	void					ChannelOpened(const boost::system::error_code& ec);
	bool					ReadDir(const boost::system::error_code& ec,
								const std::string& inName, const std::string& inLongName,
								const assh::sftp_channel::file_attributes& attr);

	struct Item
	{
		std::string			mName;
		std::string			mLongName;
		assh::sftp_channel::file_attributes
							mAttr;
	};

	typedef std::list<Item>	ItemList;

	assh::sftp_channel		mSFTPChannel;
	MExploreBrowserView*	mBrowserView;
	ItemList				mItems;
	MStatusbar*				mStatusbar;
};
