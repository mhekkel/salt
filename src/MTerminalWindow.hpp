// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

#include <pinch/channel.hpp>
#include <pinch/connection.hpp>
#include <pinch/ssh_agent.hpp>

#include "MWindow.hpp"

class MStatusbar;
class MScrollbar;
class MTerminalView;
class MSearchPanel;
class MAnimationVariable;
class MAnimationManager;
class MTerminalChannel;

class MTerminalWindow : public MWindow
{
  public:
	static MTerminalWindow *Create();
	static MTerminalWindow *Create(const std::string &inUser, const std::string &inHost, uint16_t inPort,
		const std::string &inSSHCommand, std::shared_ptr<pinch::basic_connection> inConnection);

	virtual MTerminalWindow *Clone(MTerminalWindow *inOriginal) = 0;

	virtual void Mapped();

	static MTerminalWindow *GetFirstTerminal() { return sFirst; }
	MTerminalWindow *GetNextTerminal() { return mNext; }

	virtual ~MTerminalWindow();

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked);
	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers);

	virtual bool AllowClose(bool inLogOff);
	virtual void Close();

	static bool IsAnyTerminalOpen();

  protected:
	MTerminalWindow(MTerminalChannel *inChannel, const std::string &inCommand);

	void ShowSearchPanel();
	void HideSearchPanel();

	MEventIn<void()> eAnimate;
	void Animate();

	static MRect GetPrefferedBounds();

	static MTerminalWindow *sFirst;

	MTerminalChannel *mChannel;
	MStatusbar *mStatusbar;
	MScrollbar *mScrollbar;
	MSearchPanel *mSearchPanel;
	MTerminalView *mTerminalView;
	MTerminalWindow *mNext;
	MAnimationManager *mAnimationManager;
	MAnimationVariable *mAnimationVariable;
};
