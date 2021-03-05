//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MHandler.hpp"
#include "MP2PEvents.hpp"

class MDocument;
class MWindow;

enum MCloseReason {
	kSaveChangesClosingDocument,
	kSaveChangesClosingAllDocuments,
	kSaveChangesQuittingApplication
};

class MController : public MHandler
{
  public:
						MController(MWindow* inWindow);
						~MController();

	void				SetDocument(MDocument* inDocument);

	MDocument*			GetDocument() const				{ return mDocument; }
	MWindow*			GetWindow() const				{ return mWindow; }
	
	bool				SaveDocument();
	void				RevertDocument();
	bool				DoSaveAs(const std::string& inURL);
	void				SaveDocumentAs();

	void				CloseAfterNavigationDialog();

	virtual bool		TryCloseDocument(MCloseReason inAction);
	virtual bool		TryCloseController(MCloseReason inAction);
	void				TryDiscardChanges();
	
	virtual bool		UpdateCommandStatus(uint32 inCommand, MMenu* inMenu, uint32 inItemIndex, bool& outEnabled, bool& outChecked);
	virtual bool		ProcessCommand(uint32 inCommand, const MMenu* inMenu, uint32 inItemIndex, uint32 inModifiers);
	virtual bool		HandleKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
	virtual bool		HandleCharacter(const std::string& inText, bool inRepeat);

	MEventOut<void(MDocument*)>		eDocumentChanged;
	MEventOut<void(MDocument*)>		eAboutToCloseDocument;

  protected:

	virtual void		Print();

						MController(const MController&);
	MController&		operator=(const MController&);

	MDocument*			mDocument;
	MWindow*			mWindow;
};
