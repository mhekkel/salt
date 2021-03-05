//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MXcbLib.hpp"

#include <iostream>
#include <algorithm>
#include <cstring>

#include "MMenuImpl.hpp"
#include "MWindow.hpp"
#include "MAcceleratorTable.hpp"
#include "MFile.hpp"
#include "MStrings.hpp"
#include "MResources.hpp"
#include "MUtils.hpp"
#include "MError.hpp"
#include "MXcbApplicationImpl.hpp"
#include "MXcbWindowImpl.hpp"
#include "MXcbControlsImpl.hpp"

#include "MCairoGfxDeviceImpl.hpp"

using namespace std;

/*
	TODO
	
	- popup positie bepalen adhv scherm en soort
	- submenu
	- scrollbaar maken bij teveel items

*/

class MXcbMenubarImpl;

// --------------------------------------------------------------------
// MMenuItem

struct MMenuItem
{
	MMenuItem(const string& inLabel, uint32_t inCommand)
		: mLabel(inLabel), mCommand(inCommand) {}

	MMenuItem(MMenu* inSubmenu)
		: mLabel(inSubmenu->GetLabel()), mCommand(0), mSubmenu(inSubmenu) {}

	string				mLabel;
	uint32_t				mCommand;
	unique_ptr<MMenu>	mSubmenu = nullptr;
	bool				mEnabled = false;
	bool				mChecked = false;
};

// --------------------------------------------------------------------
// Helper interface

struct MParentMenu
{
	virtual ~MParentMenu() {}
	virtual bool ReturnToParent(int32_t x, int32_t y, MMenu* inChild) = 0;
	
	MParentMenu* mParent = nullptr;
};

// --------------------------------------------------------------------
// MXcbMenuImpl

class MXcbMenuImpl : public MMenuImpl, public MXcbWinMixin, public MParentMenu
{
  public:
	MXcbMenuImpl(MMenu* inMenu);
	~MXcbMenuImpl();

	virtual void SetTarget(MHandler* inHandler);
	virtual void SetItemState(uint32_t inItem, bool inEnabled, bool inChecked);
	virtual void AppendItem(const std::string& inLabel, uint32_t inCommand);
	virtual void AppendSubmenu(MMenu* inSubmenu);
	virtual void AppendSeparator();
	virtual void AppendCheckbox(const std::string& inLabel, uint32_t inCommand);
	virtual void AppendRadiobutton(const std::string& inLabel, uint32_t inCommand);
	virtual uint32_t CountItems() const;
	virtual void RemoveItems(uint32_t inFirstIndex, uint32_t inCount);
	virtual std::string GetItemLabel(uint32_t inIndex) const;
	virtual void SetItemCommand(uint32_t inIndex, uint32_t inCommand);
	virtual uint32_t GetItemCommand(uint32_t inIndex) const;
	virtual MMenu* GetSubmenu(uint32_t inIndex) const;
	virtual void AddToWindow(MWindowImpl* inWindow);
	virtual void MenuUpdated();

	virtual int32_t Popup(MWindow* inHandler, bool preferredIsDown,
		int32_t inXPreferred, int32_t inYPreferred,
		int32_t inXAlternative, int32_t inYAlternative);

	void Hide();

	virtual void ButtonPressEvent(xcb_button_press_event_t* inEvent);
	virtual void ButtonReleaseEvent(xcb_button_press_event_t* inEvent);
	virtual void MotionNotifyEvent(xcb_motion_notify_event_t* inEvent);
	virtual void ExposeEvent(xcb_expose_event_t* inEvent);
	virtual void ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent);
	virtual void MapNotifyEvent(xcb_map_notify_event_t* inEvent);

	void SelectItem(int32_t x, int32_t y);
	int32_t GetItemForMouse(int32_t x, int32_t y);

	int32_t TrackMouse();
	void SelectCommand(uint32_t inCommand);

	void UpdateMenu();
	void DrawMenu(MGfxDevice& dev);

  protected:

	MMenuItem* CreateNewItem(const std::string& inLabel, uint32_t inCommand);
	
	void CreateWindow(MWindow* inHandler);
	void CalculateExtends();

	MRect GetItemRect(uint32_t inIndex) const;

	float StringWidth(const string& inText)
	{
		cairo_text_extents_t textExtends;
		cairo_text_extents(mCairo, inText.c_str(), &textExtends);
		
		return textExtends.x_advance;
	}

	virtual bool ReturnToParent(int32_t x, int32_t y, MMenu* inChild);
	
	int32_t MenuLoop();
	
	vector<MMenuItem>	mItems;
	MHandler*			mTarget = nullptr;
	MWindow*			mParentWindow = nullptr;
	cairo_surface_t*	mSurface = nullptr;
	cairo_t*			mCairo = nullptr;
	int32_t				mWidth, mHeight;
	uint32_t				mPaddingX, mPaddingY;
	uint32_t				mSeparatorHeight, mItemHeight;
	int32_t				mActiveItem = -1;
	
	// for the menuloop
	static bool			sDone;
	bool				mClosed = false;
};

bool MXcbMenuImpl::sDone = false;

// --------------------------------------------------------------------

MXcbMenuImpl::MXcbMenuImpl(MMenu* inMenu)
	: MMenuImpl(inMenu)
{
}

MXcbMenuImpl::~MXcbMenuImpl()
{
	if (mCairo)
		cairo_destroy(mCairo);
	if (mSurface)
		cairo_surface_destroy(mSurface);
}

void MXcbMenuImpl::SetTarget(MHandler* inHandler)
{
	mTarget = inHandler;

	for (auto& mi: mItems)
	{
		if (mi.mSubmenu)
			mi.mSubmenu->SetTarget(inHandler);
	}
}

void MXcbMenuImpl::SetItemState(uint32_t inIndex, bool inEnabled, bool inChecked)
{
	MMenuItem& mi = mItems.at(inIndex);
	mi.mEnabled = inEnabled;
	mi.mChecked = inChecked;
}

void MXcbMenuImpl::AppendItem(const string& inLabel, uint32_t inCommand)
{
	mItems.emplace_back(inLabel, inCommand);
}

void MXcbMenuImpl::AppendSubmenu(MMenu* inSubmenu)
{
	mItems.emplace_back(inSubmenu);
}

void MXcbMenuImpl::AppendSeparator()
{
	mItems.emplace_back("-", 0);
}

void MXcbMenuImpl::AppendCheckbox(const string& inLabel, uint32_t inCommand)
{
	assert(false);
}

void MXcbMenuImpl::AppendRadiobutton(const string& inLabel, uint32_t inCommand)
{
	assert(false);
}

uint32_t MXcbMenuImpl::CountItems() const
{
	return mItems.size();
}

void MXcbMenuImpl::RemoveItems(uint32_t inFirstIndex, uint32_t inCount)
{
	if (inFirstIndex < mItems.size())
	{
		auto b = mItems.begin();
		advance(b, inFirstIndex);

		if (inFirstIndex + inCount > mItems.size())
			inCount = mItems.size() - inFirstIndex;

		auto e = b;
		advance(e, inCount);	
		
		mItems.erase(b, e);
	}
}

string MXcbMenuImpl::GetItemLabel(uint32_t inIndex) const
{
	return mItems.at(inIndex).mLabel;
}

void MXcbMenuImpl::SetItemCommand(uint32_t inIndex, uint32_t inCommand)
{
	mItems.at(inIndex).mCommand = inCommand;
}

uint32_t MXcbMenuImpl::GetItemCommand(uint32_t inIndex) const
{
	return mItems.at(inIndex).mCommand;
}

MMenu* MXcbMenuImpl::GetSubmenu(uint32_t inIndex) const
{
	return mItems.at(inIndex).mSubmenu.get();
}

bool MXcbMenuImpl::ReturnToParent(int32_t x, int32_t y, MMenu* inChild)
{
	ConvertFromScreen(x, y);
	
	bool result = false;
	
	if (x >= 0 and x < mWidth)
	{
		float iy = 0;
		for (auto& item: mItems)
		{
			if (item.mLabel == "-")
			{
				iy += mSeparatorHeight;
				continue;
			}
			
			if (y >= iy and y < iy + mItemHeight)
			{
				result = item.mSubmenu.get() != inChild;
				break;
			}
			
			iy += mItemHeight;
		}
	}

	return result;
}

int32_t MXcbMenuImpl::GetItemForMouse(int32_t x, int32_t y)
{
	int32_t result = -1;
	
	if (x >= 0 and x < mWidth)
	{
		float iy = 0;
		for (uint32_t i = 0; i < mItems.size(); ++i)
		{
			auto& item = mItems[i];
			
			if (item.mLabel == "-")
			{
				iy += mSeparatorHeight;
				continue;
			}
			
			if (y >= iy and y < iy + mItemHeight)
			{
				if (item.mEnabled)
					result = i;
				break;
			}
			
			iy += mItemHeight;
		}
	}

	return result;
}

void MXcbMenuImpl::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	int32_t i = GetItemForMouse(inEvent->event_x, inEvent->event_y);
	
	if (i >= 0 and i < static_cast<int32_t>(mItems.size()))
	{
		SelectCommand(mItems[i].mCommand);
		sDone = true;
	}
}

void MXcbMenuImpl::ButtonReleaseEvent(xcb_button_press_event_t* inEvent)
{
	int32_t i = GetItemForMouse(inEvent->event_x, inEvent->event_y);
	
	if (i >= 0 and i < static_cast<int32_t>(mItems.size()))
	{
		SelectCommand(mItems[i].mCommand);
		sDone = true;
	}
}

void MXcbMenuImpl::MotionNotifyEvent(xcb_motion_notify_event_t* inEvent)
{
	int32_t x = inEvent->event_x, y = inEvent->event_y;

	// 	

	int32_t activeItem = GetItemForMouse(x, y);
	
	if (activeItem != mActiveItem)
	{
		mActiveItem = activeItem;
		UpdateMenu();
		
//		if (mActiveItem >= 0 and mActiveItem < static_cast<int32_t>(mItems.size()) and mItems[mActiveItem].mSubmenu)
//			mActiveSubmenu = static_cast<MXcbMenuImpl*>(mItems[mActiveItem].mSubmenu->impl());
//		else
//			mActiveSubmenu = nullptr;
//		
//		if (mActiveSubmenu)
//		{
//			MRect r = GetItemRect(mActiveItem);
//			
//			int32_t preferredX = r.x + r.width;
//			int32_t preferredY = r.y;
//			
//			ConvertToScreen(preferredX, preferredY);
//
//			int32_t alterateX = r.x + r.width;
//			int32_t alterateY = r.y + r.height;
//			
//			ConvertToScreen(alterateX, alterateY);
//			
//			mActiveSubmenu->Popup(mParentWindow, true,
//				preferredX, preferredY, alterateX, alterateY);
//		}
	}
	
	if (mActiveItem == -1 and mParent != nullptr)
	{
		ConvertToScreen(x, y);
		if (mParent->ReturnToParent(x, y, mMenu))
		{
			Hide();
			mClosed = true;
		}
	}
}

int32_t MXcbMenuImpl::Popup(MWindow* inHandler, bool preferredIsDown,
	int32_t inXPreferred, int32_t inYPreferred, int32_t inXAlternative, int32_t inYAlternative)
{
	if (mCairo == nullptr)
	{
		CreateWindow(inHandler);
		mParentWindow = inHandler;
	}
	
	CalculateExtends();

	// decide where to show the menu
	MRect sr;
	MWindow::GetMainScreenBounds(sr);
	
	int32_t x, y;
	
	if (preferredIsDown)	// prefer a 'hanging' menu
	{
		x = inXPreferred;
		y = inYPreferred;
		
		if (y + mHeight > sr.height)	// but does't fit
		{
			x = inXAlternative;
			y = inYAlternative - mHeight;
		}
	}
	else
	{
		x = inXPreferred;
		y = inYPreferred - mHeight;
	}

	// move/resize window before showing
	ConfigureAux(x, y, mWidth, mHeight);
	
	/* Map the window on the screen */
	MXcbWinMixin::Show();
	
	uint8_t revert_to;
	xcb_window_t focus;
	tie(revert_to, focus) = GetInputFocus();

	auto result = MenuLoop();
	
	SetInputFocus(revert_to, focus);
	
	return result;
}

int32_t MXcbMenuImpl::MenuLoop()
{
	int32_t result = -1;

	sDone = false;
	mClosed = false;
	
	bool ignoreUp = true;
	
	int32_t startX, startY, x, y;
	uint32_t modifiers = 0;
	GetMouse(startX, startY, modifiers);
	
//	GrabPointer();
	
	auto& app = MXcbApplicationImpl::Instance();
	auto connection = app.GetXCBConnection();
	
	while (not (sDone or mClosed))
	{
		try
		{
//			int item = TrackMouse();
			
			GetMouse(x, y, modifiers);
			
			ignoreUp = ignoreUp and abs(x - startX) < 3 and abs(y - startY) < 3;

//			if (item >= 0 and
//				mItems[item].mSubmenu and
//				mItems[item].mSubmenu->CountItems())
//			{
//				auto sub = mItems[item].mSubmenu.get();
//				
//				MRect r = GetItemRect(item);
//				
//				int32_t preferredX = r.x + r.width;
//				int32_t preferredY = r.y;
//				
//				ConvertToScreen(preferredX, preferredY);
//	
//				int32_t alterateX = r.x + r.width;
//				int32_t alterateY = r.y + r.height;
//				
//				ConvertToScreen(alterateX, alterateY);
//				
//				auto subImpl = static_cast<MXcbMenuImpl*>(sub->impl());
//				
//				result = subImpl->Popup(mParentWindow, true,
//					preferredX, preferredY, alterateX, alterateY);
//				
//				if (result)
//					break;
//
//				GrabPointer();
//			}
//			else if (mParent != nullptr)
//			{
//				ConvertToScreen(x, y);
//				
//				if (mParent->ReturnToParent(x, y, mMenu))
//				{
//					Hide();
//					break;
//				}
//			}

			auto e = xcb_poll_for_event(connection);
			if (e == nullptr)
				continue;
			
			switch (e->response_type & ~0x80)
			{
				case XCB_KEY_PRESS:
					sDone = true;
					result = false;
					break;
				
				case XCB_BUTTON_RELEASE:
					if (not ignoreUp)
					{
						sDone = true;
						app.ProcessEvent(e);
						result = mActiveItem;
					}
					break;
				
				default:
					app.ProcessEvent(e);
					break;
			}
		}
		catch (const exception& e)
		{
			PRINT(("Exception in menu loop: %s", e.what()));
//			DisplayError(e);
		}
		catch (...)
		{
			PRINT(("Exception in menu loop..."));
//			DisplayError(HError(pError));
		}
	}

	UngrabPointer();

	return result;
}

void MXcbMenuImpl::Hide()
{
	/* Unmap the window on the screen */
	auto cookie0 = xcb_unmap_window_checked(mConnection, mWindowID);

	xcb_generic_error_t *error;
	if ((error = xcb_request_check(mConnection, cookie0)))
	{
		free(error);
		throw runtime_error("could not unmap menu window");
	}
}

void MXcbMenuImpl::CalculateExtends()
{
	assert(mCairo != nullptr);
	
	mWidth = 2 * mPaddingX;
	mHeight = mPaddingY;
	
	for (auto& item: mItems)
	{
		if (item.mLabel == "-")	// separator
			mHeight += mSeparatorHeight;
		else
		{
			auto w = StringWidth(item.mLabel);
			if (item.mSubmenu)
				w += mItemHeight;
			
			if (mWidth < 2 * mPaddingX + w)
				mWidth = 2 * mPaddingX + w;
			mHeight += mItemHeight;
		}
	}
}

MRect MXcbMenuImpl::GetItemRect(uint32_t inIndex) const
{
	MRect result = { 0, 0, mWidth, 0 };
	
	for (uint32_t i = 0; i <= inIndex; ++i)
	{
		result.y += result.height;
		result.height = mItems[i].mLabel == "-" ? mSeparatorHeight : mItemHeight;
	}
	
	return result;
}

void MXcbMenuImpl::CreateWindow(MWindow* inHandler)
{
	assert(mCairo == nullptr);
	
	MXcbWindowImpl* parent = static_cast<MXcbWindowImpl*>(inHandler->GetImpl());
	
	MRect bounds(0, 0, 200, 200);
	
	uint32_t mask = 0;
	vector<uint32_t> values;

	mask |= XCB_CW_OVERRIDE_REDIRECT;
	values.push_back(1);

	mask |= XCB_CW_SAVE_UNDER;
	values.push_back(1);
	
//	mask |= XCB_CW_BACK_PIXEL;
//	values.push_back(mScreen->white_pixel);
	
	mask |= XCB_CW_EVENT_MASK;
	values.push_back(
		XCB_EVENT_MASK_EXPOSURE              | XCB_EVENT_MASK_BUTTON_PRESS         |
		XCB_EVENT_MASK_BUTTON_RELEASE        | XCB_EVENT_MASK_POINTER_MOTION       |
		XCB_EVENT_MASK_ENTER_WINDOW          | XCB_EVENT_MASK_LEAVE_WINDOW         |
		XCB_EVENT_MASK_KEY_PRESS             | XCB_EVENT_MASK_KEY_RELEASE          |
		XCB_EVENT_MASK_VISIBILITY_CHANGE     | XCB_EVENT_MASK_STRUCTURE_NOTIFY     |
//		XCB_EVENT_MASK_RESIZE_REDIRECT       | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY  |
//		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE         |
		XCB_EVENT_MASK_PROPERTY_CHANGE       //| XCB_EVENT_MASK_COLOR_MAP_CHANGE     |
//		XCB_EVENT_MASK_OWNER_GRAB_BUTTON
	);

	auto cookie0 = xcb_create_window_checked(mConnection, XCB_COPY_FROM_PARENT,
		mWindowID, mScreen->root,
		bounds.x, bounds.y, bounds.width, bounds.height,
		0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		XCB_COPY_FROM_PARENT, mask, values.data());

	xcb_generic_error_t *error;
	if ((error = xcb_request_check(mConnection, cookie0)))
	{
		free(error);
		throw runtime_error("could not create menu window");
	}

	// set transient for, but now in xcb
	auto parentID = parent->GetWindowID();
	cookie0 = xcb_change_property_checked(mConnection, XCB_PROP_MODE_REPLACE,
		mWindowID, XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 32, 1, &parentID);

	if ((error = xcb_request_check(mConnection, cookie0)))
	{
		free(error);
		throw runtime_error("could not set window property");
	}

	xcb_visualtype_t* visual = nullptr;
    xcb_depth_iterator_t d = xcb_screen_allowed_depths_iterator(mScreen);
    for (; visual == nullptr and d.rem; xcb_depth_next(&d))
    {
    	xcb_visualtype_iterator_t v = xcb_depth_visuals_iterator(d.data);
    	for (; visual == nullptr and v.rem; xcb_visualtype_next(&v))
		    if (v.data->visual_id == mScreen->root_visual)
				visual = v.data;
    }
	
	mSurface = cairo_xcb_surface_create(mConnection, mWindowID, visual, bounds.width, bounds.height);
	mCairo = cairo_create(mSurface);
	
	// Setup cairo with the proper values

	cairo_select_font_face(mCairo, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(mCairo, 12);
	
	cairo_font_extents_t fontExtends;
	cairo_font_extents(mCairo, &fontExtends);
	
	mPaddingY = 0.4 * fontExtends.height;
	
	cairo_text_extents_t textExtends;
	cairo_text_extents(mCairo, "M", &textExtends);
	
	mPaddingX = 2 * textExtends.x_advance;
	
	mSeparatorHeight = mPaddingY;
	mItemHeight = fontExtends.height + mPaddingY;
}

void MXcbMenuImpl::ExposeEvent(xcb_expose_event_t* inEvent)
{
	MGfxDevice dev(new MCairoGfxDeviceImpl(mCairo));
	
	try
	{
		dev.ClipRect({ inEvent->x, inEvent->y, inEvent->width, inEvent->height });
		
		DrawMenu(dev);
	}
	catch (...) {}

	cairo_surface_flush(mSurface);
	
	xcb_flush(mConnection);
}

void MXcbMenuImpl::ConfigureNotifyEvent(xcb_configure_notify_event_t* inEvent)
{
	if (mSurface != nullptr)
		cairo_xcb_surface_set_size(mSurface, inEvent->width, inEvent->height);
}

void MXcbMenuImpl::MapNotifyEvent(xcb_map_notify_event_t* inEvent)
{
	GrabPointer();
}

int32_t MXcbMenuImpl::TrackMouse()
{
	int32_t x, y;
	uint32_t modifiers = 0;
	
	MXcbWinMixin::GetMouse(x, y, modifiers);
	
//	auto cookie = xcb_translate_coordinates(mConnection, mScreen->root, mWindowID, x, y);
//	
//	xcb_generic_error_t *error;
//	auto reply = xcb_translate_coordinates_reply(mConnection, cookie, &error);
//	
//	if (error != nullptr)
//	{
//		free(error);
//		throw runtime_error("could not translate coordinates");
//	}
//	
//	x = reply->dst_x;
//	y = reply->dst_y;
//	
//	free(reply);

	// 	

	int32_t activeItem = -1;
	
	if (x > 0 and x < mWidth)
	{
		int32_t iy = 0, i = 0;
		
		for (auto& item: mItems)
		{
			if (item.mLabel == "-")	// separator
				iy += mSeparatorHeight;
			else if (y >= iy and y < iy + static_cast<int32_t>(mItemHeight))
			{
				if (item.mEnabled)
				{
//					result = item.mCommand;
					activeItem = i;
				}
				break;
			}
			else
				iy += mItemHeight;

			++i;
		}
	}
	
	if (activeItem != mActiveItem)
	{
		mActiveItem = activeItem;
		UpdateMenu();
		
//		if (mActiveItem >= 0 and mActiveItem < static_cast<int32_t>(mItems.size()) and mItems[mActiveItem].mSubmenu)
//			mActiveSubmenu = static_cast<MXcbMenuImpl*>(mItems[mActiveItem].mSubmenu->impl());
//		else
//			mActiveSubmenu = nullptr;
//		
//		if (mActiveSubmenu)
//		{
//			MRect r = GetItemRect(mActiveItem);
//			
//			int32_t preferredX = r.x + r.width;
//			int32_t preferredY = r.y;
//			
//			ConvertToScreen(preferredX, preferredY);
//
//			int32_t alterateX = r.x + r.width;
//			int32_t alterateY = r.y + r.height;
//			
//			ConvertToScreen(alterateX, alterateY);
//			
//			mActiveSubmenu->Popup(mParentWindow, true,
//				preferredX, preferredY, alterateX, alterateY);
//		}
	}
	
	return mActiveItem;	
}

void MXcbMenuImpl::SelectCommand(uint32_t inCommand)
{
	if (mTarget != nullptr)
	{
		auto activeItem = mActiveItem;
		for (int i: { -1, activeItem, -1, activeItem })
		{
			mActiveItem = i;
			UpdateMenu();
			cairo_surface_flush(mSurface);
			xcb_flush(mConnection);
			usleep(30000);
		}
		
		mTarget->ProcessCommand(inCommand, mMenu, 0, 0);
	}
}

void MXcbMenuImpl::UpdateMenu()
{
	MGfxDevice dev(new MCairoGfxDeviceImpl(mCairo));
	
	try
	{
		DrawMenu(dev);
	}
	catch (...) {}

	cairo_surface_flush(mSurface);
	
	xcb_flush(mConnection);
}

void MXcbMenuImpl::DrawMenu(MGfxDevice& dev)
{
	MRect bounds(0, 0, mWidth, mHeight);
	
	// erase
	
	dev.SetThemeColor(kThemeColorMenuBackground);
	dev.FillRect(bounds.x, bounds.y, bounds.width, bounds.height);

	// box

	dev.SetThemeColor(kThemeColorMenuFrame);
	dev.StrokeRect(bounds.x, bounds.y, bounds.width, bounds.height);
	
	//
	
	auto fontExtends = dev.GetFontExtents();
	
	float y = mPaddingY;
	
	int32_t ix = 0;
	for (auto& item: mItems)
	{
		if (item.mLabel == "-")	// separator
		{
			dev.SetThemeColor(kThemeColorMenuFrame);
			
			float sy = trunc(y + mSeparatorHeight / 2) - 1.5;
	
			dev.SetLineWidth(1.0);
			dev.StrokeLine(mPaddingX / 3, sy, mWidth - mPaddingX / 3, sy);

			y += mSeparatorHeight;
		}
		else
		{
			if (ix == mActiveItem)
			{
				dev.SetThemeColor(kThemeColorMenuBackground, true, true);
				dev.FillRect(bounds.x, trunc(y - (mPaddingY / 2)), bounds.width, mItemHeight);
			}
			
			dev.SetThemeColor(kThemeColorMenuText, item.mEnabled, ix == mActiveItem);
			dev.MoveTo(mPaddingX, y + fontExtends.ascent);
			dev.ShowText(item.mLabel.c_str());
			
			if (item.mSubmenu)
			{
				float triangleHeight = 0.6 * fontExtends.height;
				float halfTriangleHeight = triangleHeight / 2;
				float ty = y + (mPaddingY + mItemHeight - triangleHeight) / 2;
				float tx = bounds.x + bounds.width - halfTriangleHeight - mPaddingX / 3;

				dev.FillPoly({
					{ tx, ty },
					{ tx - halfTriangleHeight, ty - halfTriangleHeight },
					{ tx - halfTriangleHeight, ty + halfTriangleHeight }
				});
			}
			
			y += mItemHeight;
		}
		
		++ix;
	}
}

void MXcbMenuImpl::AddToWindow(MWindowImpl* inWindowImpl)
{
}

void MXcbMenuImpl::MenuUpdated()
{
	mActiveItem = -1;
	
	// enable items that have sub menus with enabled items
	for (auto& mi: mItems)
	{
		if (mi.mSubmenu == nullptr)
			continue;
		
		auto impl = static_cast<MXcbMenuImpl*>(mi.mSubmenu->impl());
		impl->mParent = this;

		auto mii = find_if(impl->mItems.begin(), impl->mItems.end(),
			[](auto& mi2) { return mi2.mEnabled; });
	
		if (mii != impl->mItems.end())
			mi.mEnabled = true;
	}
}

MMenuImpl* MMenuImpl::CreateMenuImpl(MMenu* inMenu)
{
	return new MXcbMenuImpl(inMenu);
}

// --------------------------------------------------------------------



// --------------------------------------------------------------------

class MXcbMenubarImpl : public MXcbControlImpl<MMenubarControl>, public MParentMenu
{
  public:
	MXcbMenubarImpl(MMenubarControl* inMenubar, MMenu* inMenu)
		: MXcbControlImpl<MMenubarControl>(inMenubar, "menubar")
		, mMenu(inMenu), mMenuImpl(inMenu->impl())
	{
		sOffscreenDev.SetFont("Sans", 12);
		mPadding = sOffscreenDev.GetTextExtents("x").xAdvance;
	}

	virtual void DrawWidget(MGfxDevice& dev);

	virtual void ButtonPressEvent(xcb_button_press_event_t* inEvent);
	virtual void ButtonReleaseEvent(xcb_button_press_event_t* inEvent);
	virtual void MotionNotifyEvent(xcb_motion_notify_event_t* inEvent);

	virtual bool ReturnToParent(int32_t x, int32_t y, MMenu* inChild);
	
	int32_t MenuForMouse(float x, float y);
	int32_t LeftEdgeOfMenu(uint32_t inMenu);
	void ShowMenu(int32_t inMenu);
	
	bool mMouseDown = false;
	int32_t mActiveMenu = 1;
	uint32_t mCommand = 0;
	
	float mPadding;
	
	unique_ptr<MMenu> mMenu;
	MMenuImpl* mMenuImpl;
	MXcbMenuImpl* mVisibleMenu = nullptr;

	static MGfxDevice sOffscreenDev;
};

MGfxDevice MXcbMenubarImpl::sOffscreenDev(1000, 32);

int32_t MXcbMenubarImpl::MenuForMouse(float x, float y)
{
	int32_t result = -1;
	
	MRect bounds;
	mControl->GetBounds(bounds);

	if (bounds.ContainsPoint(x, y))
	{
		int32_t menuCount = mMenuImpl->CountItems();
		float left = 0;
	
		for (int32_t i = 0; i < menuCount; ++i)
		{
			auto menu = mMenuImpl->GetSubmenu(i);
			if (menu == nullptr) // ????
				continue;
			
			string label = mMenuImpl->GetItemLabel(i);
			auto textExtends = sOffscreenDev.GetTextExtents(label.c_str());

			auto width = 2 * mPadding + textExtends.xAdvance;
			
			if (x >= left and x <= left + width)
			{
				result = i;
				break;
			}
	
			left += width;
	
			if (x < left)
				break;
		}
	}
	
	return result;
}

int32_t MXcbMenubarImpl::LeftEdgeOfMenu(uint32_t inMenu)
{
	int32_t result = 0;
	
	for (uint32_t i = 0; i < inMenu; ++i)
	{
		auto menu = mMenuImpl->GetSubmenu(i);
		if (menu == nullptr) // ????
			continue;
			
		string label = mMenuImpl->GetItemLabel(i);
		result += 2 * mPadding + sOffscreenDev.GetTextExtents(label.c_str()).xAdvance;
	}
	
	return result;
}

void MXcbMenubarImpl::ShowMenu(int32_t inMenu)
{
	if (mVisibleMenu != nullptr)
	{
		mVisibleMenu->Hide();
		mVisibleMenu = nullptr;
	}
	
	if (inMenu >= 0 and inMenu < static_cast<int32_t>(mMenuImpl->CountItems()))
	{
		auto menu = mMenuImpl->GetSubmenu(mActiveMenu);
		mVisibleMenu = static_cast<MXcbMenuImpl*>(menu->impl());
	
		MRect bounds;
		mControl->GetBounds(bounds);
		
		int32_t x = bounds.x + LeftEdgeOfMenu(mActiveMenu);
		int32_t y = bounds.y + bounds.height;
		
		mControl->ConvertToScreen(x, y);
		
		mVisibleMenu->Popup(mControl->GetWindow(), true, x, y, x, y - bounds.height);
	}
}

void MXcbMenubarImpl::DrawWidget(MGfxDevice& dev)
{
	MRect bounds;
	mControl->GetBounds(bounds);

//	bounds.InsetBy(1, 1);
	
	dev.SetThemeColor(kThemeColorMenubarBackground);
	dev.FillRect(bounds);
	
	dev.SetFont("Sans", 12);

	auto fontExtends = dev.GetFontExtents();
	
	float x = 0;
	float y = bounds.y + (bounds.height - fontExtends.height) / 2 +
				fontExtends.ascent;

	int32_t menuCount = mMenuImpl->CountItems();
	for (int32_t i = 0; i < menuCount; ++i)
	{
		auto menu = mMenuImpl->GetSubmenu(i);
		if (menu == nullptr) // ????
			continue;
		
		string label = mMenuImpl->GetItemLabel(i);
		float w = dev.GetTextExtents(label.c_str()).xAdvance;
		
		MRect r = bounds;
		r.x = x;
		r.width = 2 * mPadding + w;
		
		if (mMouseDown and i == mActiveMenu)
		{
			dev.SetThemeColor(kThemeColorMenubarBackground, true, true);
			dev.FillRect(r);
		}

		dev.SetThemeColor(kThemeColorMenubarText, true, mMouseDown and i == mActiveMenu);
		dev.MoveTo(x + mPadding, y);
		dev.ShowText(label.c_str());
		x += r.width;
	}
}

void MXcbMenubarImpl::ButtonPressEvent(xcb_button_press_event_t* inEvent)
{
	mMenu->UpdateCommandStatus();
	
	mCommand = 0;
	mMouseDown = true;
	mActiveMenu = MenuForMouse(inEvent->event_x, inEvent->event_y);
	
	ShowMenu(mActiveMenu);
	
	Invalidate();
}

void MXcbMenubarImpl::ButtonReleaseEvent(xcb_button_press_event_t* inEvent)
{
	mMouseDown = false;
	mActiveMenu = -1;
	
	if (mCommand != 0)
	{
//		PRINT((MCommandToString(mCommand)));
		mVisibleMenu->SelectCommand(mCommand);
	}

	ShowMenu(-1);
	
	Invalidate();
}

void MXcbMenubarImpl::MotionNotifyEvent(xcb_motion_notify_event_t* inEvent)
{
	PRINT(("MXcbMenubarImpl::MotionNotifyEvent"));
	
	if (mMouseDown)
	{
		auto over = MenuForMouse(inEvent->event_x, inEvent->event_y);
		if (over != mActiveMenu and over != -1)
		{
			mActiveMenu = over;
			ShowMenu(mActiveMenu);
			Invalidate();
		}
//		
//		if (mVisibleMenu != nullptr)
//			mCommand = mVisibleMenu->TrackMouse(inEvent->root_x, inEvent->root_y);
//		else
//			mCommand = 0;
	}
}

bool MXcbMenubarImpl::ReturnToParent(int32_t x, int32_t y, MMenu* inChild)
{
	ConvertFromScreen(x, y);
	
	auto over = MenuForMouse(x, y);
	
	return over != -1 and mMenuImpl->GetSubmenu(over) != inChild;
}

// --------------------------------------------------------------------

MMenubarControlImpl* MMenubarControlImpl::Create(MMenubarControl* inMenubarControl, MMenu* inMenu)
{
	return new MXcbMenubarImpl(inMenubarControl, inMenu);	
}

// --------------------------------------------------------------------

int MMenubarControl::GetMenubarHeight()
{
	MXcbMenubarImpl::sOffscreenDev.SetFont("Sans", 12);
	auto fontExtends = MXcbMenubarImpl::sOffscreenDev.GetFontExtents();
	
	return static_cast<int>(1.8 * fontExtends.height);
}
