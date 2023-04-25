//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include "MCanvasImpl.hpp"
#include "MApplication.hpp"
#include "MControls.inl"

using namespace std;

MCanvas::MCanvas(
	const string&	inID,
	MRect			inBounds,
	bool			inAcceptDropFiles,
	bool			inAcceptDropText)
	: MControl<MCanvasImpl>(inID, inBounds, MCanvasImpl::Create(this, inBounds.width, inBounds.height))
	, mAcceptDropFiles(inAcceptDropFiles)
	, mAcceptDropText(inAcceptDropText)
{
	mWillDraw = true;
}

MCanvas::~MCanvas()
{
	delete mImpl;
}

//void MCanvas::MoveFrame(
//	int32_t			inXDelta,
//	int32_t			inYDelta)
//{
//	mImpl->MoveFrame(inXDelta, inYDelta);
//}
//
//void MCanvas::ResizeFrame(
//	int32_t			inWidthDelta,
//	int32_t			inHeightDelta)
//{
//	mImpl->ResizeFrame(inWidthDelta, inHeightDelta);
//}

void MCanvas::AddedToWindow()
{
	MControl::AddedToWindow();

	if (mAcceptDropFiles or mAcceptDropText)
		mImpl->AcceptDragAndDrop(mAcceptDropFiles, mAcceptDropText);
}

void MCanvas::Invalidate()
{
	mImpl->Invalidate();
}

void MCanvas::DragEnter()
{
}

bool MCanvas::DragWithin(int32_t inX, int32_t inY)
{
	return false;
}

void MCanvas::DragLeave()
{
}

bool MCanvas::Drop(bool inMove, int32_t inX, int32_t inY, const string& inText)
{
	return false;
}

bool MCanvas::Drop(int32_t inX, int32_t inY, const std::filesystem::path& inFile)
{
	try
	{
		//gApp->OpenOneDocument(inFile.string());
	}
	catch (...) {}
	return true;
}

void MCanvas::StartDrag()
{
	mImpl->StartDrag();
}

void MCanvas::DragSendData(std::string& outData)
{
}

void MCanvas::DragDeleteData()
{
}

//void MCanvas::SetFocus()
//{
//	mImpl->SetFocus();
//}
//
//void MCanvas::ReleaseFocus()
//{
//	mImpl->ReleaseFocus();
//}
//
//bool MCanvas::IsFocus() const
//{
//	return mImpl->IsFocus();
//}
//
//void MCanvas::TrackMouse(bool inTrackMove, bool inTrackExit)
//{
//	mImpl->TrackMouse(inTrackMove, inTrackExit);
//}
