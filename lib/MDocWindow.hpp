//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MWindow.hpp"
#include "MDocument.hpp"
#include "MController.hpp"
#include "MMenu.hpp"

class MDocWindow : public MWindow
{
  public:
						MDocWindow(const std::string& inTitle, const MRect& inBounds, MWindowFlags inFlags, const std::string& inMenu);

	static MDocWindow*	FindWindowForDocument(MDocument* inDocument);
	
	MEventIn<void(bool)>					eModifiedChanged;
	MEventIn<void(MDocument*)>				eFileSpecChanged;
	MEventIn<void(MDocument*)>				eDocumentChanged;

	void				SetDocument(MDocument* inDocument);

	MDocument*			GetDocument();

						// document is about to be closed
	virtual void		SaveState();

	virtual void		AddRoutes(MDocument* inDocument);
	
	virtual void		RemoveRoutes(MDocument* inDocument);

	virtual bool		UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex, bool& outEnabled, bool& outChecked);

	virtual bool		ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex, uint32 inModifiers);

  protected:

	static std::string	GetUntitledTitle();

	virtual void		DocumentChanged(MDocument* inDocument);

	virtual bool		AllowClose(bool inQuit);

	virtual void		ModifiedChanged(bool inModified);

	virtual void		FileSpecChanged(MDocument* inDocument);

	virtual void		ActivateSelf();
	
  protected:

	MController			mController;

	virtual				~MDocWindow();
};
