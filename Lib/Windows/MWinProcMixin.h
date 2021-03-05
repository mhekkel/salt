//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <string>
#include <map>

class MHandler;

class MWinProcMixin
{
  public:
					MWinProcMixin(MHandler* inHandler);
	virtual			~MWinProcMixin();

	HWND			GetHandle() const						{ return mHandle; }
	virtual void	SetHandle(HWND inHandle);

	MHandler*		GetHandler() const						{ return mHandler; }

	static MWinProcMixin*
					Fetch(HWND inHandle);

	virtual void	CreateHandle(MWinProcMixin* inParent, MRect inBounds,
						const std::wstring& inTitle);

	virtual void	SubClass();

	typedef boost::function<bool(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult)> MWMCall;
	
	void			AddHandler(UINT inMessage, MWMCall inCallback)
						{ mHandlers[inMessage] = inCallback; }

	typedef boost::function<bool(WPARAM inWParam, LPARAM inLParam, LRESULT& outResult)> MNotification;

	void			AddNotify(uint32 inCode, HWND inHWND, MNotification inCallback)
					{
						MNotifyHandler h = { inHWND, inCode };
						mNotificationHandlers[h] = inCallback;
					}

	virtual bool	WMCommand(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);

	virtual bool	WMDrawItem(DRAWITEMSTRUCT* inDrawItemStruct);
	
	virtual bool	DispatchKeyDown(uint32 inKeyCode, uint32 inModifiers, bool inRepeat);
	virtual bool	DispatchCharacter(const std::string& inText, bool inRepeat);
	virtual bool	DispatchCommand(uint32 inCommand, uint32 inModifiers);

	virtual void	CreateParams(DWORD& outStyle, DWORD& outExStyle,
						std::wstring& outClassName, HMENU& outMenu);
	virtual void	RegisterParams(UINT& outStyle, int& outWndExtra,
						HCURSOR& outCursor, HICON& outIcon,
						HICON& outSmallIcon, HBRUSH& outBackground);

	virtual bool	WMDestroy(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMKeydown(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMChar(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMUniChar(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMMouseWheel(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);

	virtual bool	WMSetFocus(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMKillFocus(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);

	virtual bool	WMQueryEndSession(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);
	virtual bool	WMEndSession(HWND inHWnd, UINT inUMsg, WPARAM inWParam,
						LPARAM inLParam, LRESULT& outResult);

	virtual LRESULT	WinProc(HWND inHWnd, UINT inUMsg,
						WPARAM inWParam, LPARAM inLParam);
	virtual LRESULT	DefProc(HWND inHWnd, UINT inUMsg, 
						WPARAM inWParam, LPARAM inLParam);
	
  private:
	typedef std::map<UINT,MWMCall>				MHandlerTable;

	struct MNotifyHandler
	{
		HWND			mHWND;
		UINT			mCode;

		bool			operator<(const MNotifyHandler& rhs) const
							{ return mHWND < rhs.mHWND or (mHWND == rhs.mHWND and mCode < rhs.mCode);	}
	};

	typedef std::map<MNotifyHandler,MNotification>		MNotificationTable;

	HWND				mHandle;
	MHandler*			mHandler;
	WNDPROC				mOldWinProc;
	MHandlerTable		mHandlers;
	MNotificationTable	mNotificationHandlers;
	bool				mHandledKeyDown;

	static LRESULT CALLBACK
					WinProcCallBack(HWND inHWnd, UINT inUMsg,
						WPARAM inWParam, LPARAM inLParam);
};
