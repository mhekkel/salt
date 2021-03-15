//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*
	
	MDocument is the model in the Model-View-Controller triad
	
*/

#pragma once

#include "MP2PEvents.hpp"
#include "MFile.hpp"
#include "MHandler.hpp"

class MDocClosedNotifier;
class MController;
class MDocWindow;
class MMenu;

typedef std::list<MController*> MControllerList;

class MDocument : public MHandler
{
  public:
						MDocument(MHandler* inSuper);
						MDocument(MHandler* inSuper, const std::string& inURL);

	virtual				~MDocument();

	virtual void		SetFile(const std::string& inURL);
	const MFile&		GetFile() const						{ return mFile; }

	virtual void		DoLoad();

	virtual bool		DoSave();
	virtual bool		DoSaveAs(const std::string& inURL);
	bool				DoSaveAs(const fs::path& inPath);

	virtual void		RevertDocument();
	
	virtual bool		IsSpecified() const					{ return mFile.IsValid(); }
	virtual bool		IsReadOnly() const					{ return mFile.IsReadOnly(); }

	virtual bool		UsesFile(const std::string& inURL) const;
	static MDocument*	GetDocumentForFile(const std::string& inURL);
	
	virtual void		AddNotifier(MDocClosedNotifier& inNotifier, bool inRead);
	
	// the MVC interface
	void				AddController(MController* inController);
	void				RemoveController(MController* inController);
	std::size_t				CountControllers() const			{ return mControllers.size(); }
	virtual MController*
						GetFirstController() const;
	MControllerList		GetControllers() const				{ return mControllers; }
	
	MWindow*			GetWindow() const;

	static MDocument*	GetFirstDocument()					{ return sFirst; }
	MDocument*			GetNextDocument()					{ return mNext; }
	void				MakeFirstDocument();
	
	bool				IsModified() const					{ return mDirty; }
	virtual void		SetModified(bool inModified);

	virtual void		CheckIfModifiedOnDisk(MWindow* inDocWindow);

	virtual std::string	GetWindowTitle() const;
	virtual std::string	GetDocumentName() const;

	// note that MDocument is NOT a MHandler, but does 
	// implement the UpdateCommandStatus and ProcessCommand methods
	// Reason for this is that documents can be shared by multiple controllers
	
	virtual bool		UpdateCommandStatus(uint32_t inCommand, MMenu* inMenu, uint32_t inItemIndex, bool& outEnabled, bool& outChecked);

	virtual bool		ProcessCommand(uint32_t inCommand, const MMenu* inMenu, uint32_t inItemIndex, uint32_t inModifiers);

	virtual bool		HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
	virtual bool		HandleCharacter(const std::string& inText, bool inRepeat);

	MEventOut<void(bool)>					eModifiedChanged;
	MEventOut<void(MDocument*)>				eDocumentClosed;
	MEventOut<void(MDocument*)>				eFileSpecChanged;
	MEventOut<void(const fs::path&)>		eBaseDirChanged;

	// Asynchronous IO support
	virtual void		IOReadFile(std::istream& inFile) = 0;
	virtual void		IOWriteFile(std::ostream& inFile) = 0;
	virtual void		IOProgress(float inProgress, const std::string&);
	virtual void		IOError(const std::string& inError);
	virtual bool		IOAskOverwriteNewer();
	
  protected:

	virtual void		CloseDocument();

	typedef std::list<MDocClosedNotifier>	MDocClosedNotifierList;
	
	MControllerList		mControllers;
	MDocClosedNotifierList
						mNotifiers;
	MFile				mFile;
	bool				mWarnedReadOnly;
	bool				mDirty;

  private:
	MDocument*			mNext;
	static MDocument*	sFirst;
};
