// Copyright Maarten L. Hekkelman 2017
// All rights reserved

#pragma once

#include "MDialog.h"

class MAddTOTPHashDialog : public MDialog
{
  public:
	 MAddTOTPHashDialog();
	 ~MAddTOTPHashDialog();

	virtual bool OKClicked();
};
