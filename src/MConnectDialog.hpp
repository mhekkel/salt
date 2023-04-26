// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include "MDialog.hpp"

class MConnectDialog : public MDialog
{
  public:
	MConnectDialog();

	virtual bool CancelClicked();
	virtual bool OKClicked();

	virtual void TextChanged(const std::string &inID, const std::string &inText);
	virtual void CheckboxChanged(const std::string &inID, bool inValue);
	virtual void ButtonClicked(const std::string &inID);

  private:
	void SelectProxy(const std::string &inProxy);
	void SelectRecent(const std::string &inRecent);

	std::vector<std::string> mRecentSessions, mRecentProxies;
};
