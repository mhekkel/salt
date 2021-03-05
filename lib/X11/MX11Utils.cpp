//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MX11Lib.hpp"

#include <sys/types.h>
#include <sys/time.h>
#include <pwd.h>

#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>
#include <sstream>
#include <string>
#include <stack>
#include <cmath>

#include "MError.hpp"
#include "MUtils.hpp"
#include "MApplication.hpp"

using namespace std;

double GetLocalTime()
{
	struct timeval tv;
	
	gettimeofday(&tv, nullptr);
	
	return tv.tv_sec + tv.tv_usec / 1e6;
}

double GetDoubleClickTime()
{
//	return ::GetDblTime() / 60.0;
	return 0.2;
}

string GetUserName(bool inShortName)
{
	string result;
	
	int uid = getuid();
	struct passwd* pw = getpwuid(uid);

	if (pw != nullptr)
	{
		if (inShortName or *pw->pw_gecos == 0)
			result = pw->pw_name;
		else
		{
			result = pw->pw_gecos;
			
			if (result.length() > 0)
			{
				string::size_type p = result.find(',');

				if (p != string::npos)
					result.erase(p, result.length() - p);

				p = result.find('&');

				if (p != string::npos)
					result.replace(p, 1, pw->pw_name);
			}
		}
	}

	return result;
}

string GetDateTime()
{
	// had to remove this, because it depends on wchar_t in libstdc++...
	using namespace boost::gregorian;

	date today = day_clock::local_day();

	date::ymd_type ymd = today.year_month_day();
	greg_weekday wd = today.day_of_week();
	
	stringstream s;
	
	s << wd.as_long_string() << " "
      << ymd.month.as_long_string() << " "
	  << ymd.day << ", " << ymd.year;
	
	return s.str();
//
//	string result;
//
//	GDate* date = g_date_new();
//	if (date != nullptr)
//	{
//		g_date_set_time_t(date, time(nullptr)); 
//	
//		char buffer[1024] = "";
//		uint32 size = g_date_strftime(buffer, sizeof(buffer),
//			"%A %d %B, %Y", date);
//		
//		result.assign(buffer, buffer + size);
//	}
//	
//	return result;
}

bool IsModifierDown(int inModifierMask)
{
	bool result = false;
	
//	GdkModifierType state;
//
//	if (gtk_get_current_event_state(&state))
//		result = (state & inModifierMask) != 0;
	
	return result;
}

//// --------------------------------------------------------------------
//// code to create a GdkPixbuf containing a single dot.
////
//// use cairo to create an alpha mask, then set the colour
//// into the pixbuf.
//
//GdkPixbuf* CreateDot(
//	MColor			inColor,
//	uint32			inSize)
//{
//	// first draw in a buffer with cairo
//	cairo_surface_t* cs = cairo_image_surface_create(
//		CAIRO_FORMAT_ARGB32, inSize, inSize);
//	
//	cairo_t* c = cairo_create(cs);
//
//	cairo_translate(c, inSize / 2., inSize / 2.);
//	cairo_scale(c, inSize / 2., inSize / 2.);
//	cairo_arc(c, 0., 0., 1., 0., 2 * M_PI);
//	cairo_fill(c);
//
//	cairo_destroy(c);
//	
//	// then copy the data over to a pixbuf;
//
//	GdkPixbuf* result = gdk_pixbuf_new(
//		GDK_COLORSPACE_RGB, true, 8, inSize, inSize);
//	THROW_IF_NIL(result);
//	
//	unsigned char* dst = gdk_pixbuf_get_pixels(result);
//	unsigned char* src = cairo_image_surface_get_data(cs);
//	
//	uint32 dst_rowstride = gdk_pixbuf_get_rowstride(result);
//	uint32 src_rowstride = cairo_image_surface_get_stride(cs);
//	uint32 n_channels = gdk_pixbuf_get_n_channels(result);
//
//	for (uint32 x = 0; x < inSize; ++x)
//	{
//		for (uint32 y = 0; y < inSize; ++y)
//		{
//			unsigned char* p = dst + y * dst_rowstride + x * n_channels;
//			uint32 cp = *reinterpret_cast<uint32*>(src + y * src_rowstride + x * 4);
//
//			p[0] = inColor.red;
//			p[1] = inColor.green;
//			p[2] = inColor.blue;
//
//			p[3] = (cp >> 24) & 0xFF;
//		}
//	}
//	
//	cairo_surface_destroy(cs);
//
//	return result;
//}

//#include <dlfcn.h>
//
//void OpenURI(const string& inURI)
//{
//	bool opened = false;
//	
//	void* libgnome = dlopen("libgnomevfs-2.so.0", RTLD_LAZY);
//	if (libgnome != nullptr)
//	{
//		typedef gboolean (*gnome_vfs_url_show_func)(const char*);
//		
//		gnome_vfs_url_show_func gnome_url_show =
//			(gnome_vfs_url_show_func)dlsym(libgnome, "gnome_vfs_url_show");
//
//		if (gnome_url_show != nullptr)
//		{
//			int r = (*gnome_url_show)(inURI.c_str());
//			opened = r == 0;
//		}
//	}
//	
//	if (not opened)
//		system((string("gnome-open ") + inURI).c_str());
//}

string GetHomeDirectory()
{
	const char* home = getenv("HOME");
	return home ? string(home) : "~";
}

string GetPrefsDirectory()
{
	const char* user_config_dir = nullptr; //g_get_user_config_dir();
	return user_config_dir ?
		(fs::path(user_config_dir) / kAppName).string() :
		(GetHomeDirectory() + '/' + kAppName);
}

string GetApplicationVersion()
{
	return VERSION;
}

uint32 MapModifier(uint32 inModifier)
{
	uint32 result = 0;
	
//	if (inModifier & GDK_SHIFT_MASK)	 result |= kShiftKey;
//	if (inModifier & GDK_CONTROL_MASK)	 result |= kControlKey;
	
	return result;	
}

uint32 MapKeyCode(uint32 inKeyValue)
{
	uint32 result = 0;
		
//	switch (inKeyValue)
//	{
//		case GDK_KEY_Home:		result = kHomeKeyCode; break;
//		case GDK_KEY_Cancel:	result = kCancelKeyCode; break;
//		case GDK_KEY_End:		result = kEndKeyCode; break;
//		case GDK_KEY_Insert:	result = kInsertKeyCode; break;
////		case GDK_KEY_Be:		result = kBellKeyCode; break;
//		case GDK_KEY_BackSpace:	result = kBackspaceKeyCode; break;
//		case GDK_KEY_ISO_Left_Tab:
//								result = kTabKeyCode; break;
//		case GDK_KEY_Tab:		result = kTabKeyCode; break;
//		case GDK_KEY_Linefeed:	result = kLineFeedKeyCode; break;
////		case GDK_KEY_:		result = kVerticalTabKeyCode; break;
//		case GDK_KEY_Page_Up:	result = kPageUpKeyCode; break;
////		case GDK_KEY_:		result = kFormFeedKeyCode; break;
//		case GDK_KEY_Page_Down:	result = kPageDownKeyCode; break;
//		case GDK_KEY_Return:	result = kReturnKeyCode; break;
//		case GDK_KEY_KP_Enter:	result = kEnterKeyCode; break;
////		case GDK_KEY_:		result = kFunctionKeyKeyCode; break;
//		case GDK_KEY_Pause:		result = kPauseKeyCode; break;
//		case GDK_KEY_Escape:	result = kEscapeKeyCode; break;
//		case GDK_KEY_Clear:		result = kClearKeyCode; break;
//		case GDK_KEY_Left:		result = kLeftArrowKeyCode; break;
//		case GDK_KEY_Right:		result = kRightArrowKeyCode; break;
//		case GDK_KEY_Up:		result = kUpArrowKeyCode; break;
//		case GDK_KEY_Down:		result = kDownArrowKeyCode; break;
////		case GDK_KEY_:		result = kSpaceKeyCode; break;
//		case GDK_KEY_Delete:	result = kDeleteKeyCode; break;
////		case GDK_KEY_:		result = kDivideKeyCode; break;
////		case GDK_KEY_:		result = kMultiplyKeyCode; break;
////		case GDK_KEY_:		result = kSubtractKeyCode; break;
//		case GDK_KEY_Num_Lock:	result = kNumlockKeyCode; break;
//		case GDK_KEY_F1:		result = kF1KeyCode; break;
//		case GDK_KEY_F2:		result = kF2KeyCode; break;
//		case GDK_KEY_F3:		result = kF3KeyCode; break;
//		case GDK_KEY_F4:		result = kF4KeyCode; break;
//		case GDK_KEY_F5:		result = kF5KeyCode; break;
//		case GDK_KEY_F6:		result = kF6KeyCode; break;
//		case GDK_KEY_F7:		result = kF7KeyCode; break;
//		case GDK_KEY_F8:		result = kF8KeyCode; break;
//		case GDK_KEY_F9:		result = kF9KeyCode; break;
//		case GDK_KEY_F10:		result = kF10KeyCode; break;
//		case GDK_KEY_F11:		result = kF11KeyCode; break;
//		case GDK_KEY_F12:		result = kF12KeyCode; break;
//		case GDK_KEY_F13:		result = kF13KeyCode; break;
//		case GDK_KEY_F14:		result = kF14KeyCode; break;
//		case GDK_KEY_F15:		result = kF15KeyCode; break;
//		case GDK_KEY_F16:		result = kF16KeyCode; break;
//		case GDK_KEY_F17:		result = kF17KeyCode; break;
//		case GDK_KEY_F18:		result = kF18KeyCode; break;
//		case GDK_KEY_F19:		result = kF19KeyCode; break;
//		case GDK_KEY_F20:		result = kF20KeyCode; break;
//		
//		default:				result = inKeyValue; break;
//	}
	
	return result;	
}


