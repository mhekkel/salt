//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id: MPreferences.cpp 85 2006-09-14 08:23:20Z maarten $
	Copyright Maarten L. Hekkelman
	Created Sunday August 01 2004 13:33:22
*/

#include "MLib.hpp"

#include <sstream>
#include <cerrno>
#include <cstring>
#include <map>

#include <boost/bind.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

#include <cstring>

#include "MTypes.hpp"
#include "MPreferences.hpp"
#include "MError.hpp"
#include "MApplication.hpp"
#include "MUtils.hpp"
#include "MFile.hpp"

#include <zeep/xml/document.hpp>
#include <zeep/xml/node.hpp>
#include <zeep/xml/serialize.hpp>

using namespace std;
namespace xml = zeep::xml;
namespace fs = boost::filesystem;

fs::path	gPrefsDir;
string		gPrefsFileName = string(kAppName) + ".cfg";

namespace Preferences
{

struct preference
{
	string				name;
	vector<string>		value;
	
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(name)
		   & BOOST_SERIALIZATION_NVP(value);
	}
};

struct preferences
{
	vector<preference>	pref;
	
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & zeep::xml::make_element_nvp("pref", pref);
	}
};

class IniFile
{
  public:

	static IniFile&	Instance();
	
	void		SetString(const char* inName, const string& inValue);
	string		GetString(const char* inName, const string& inDefault);

	void		SetStrings(const char* inName, const vector<string>& inValues);
	void		GetStrings(const char* inName, vector<string>& outStrings);
	
	time_t		GetCreationTime() const		{ return GetFileCreationTime(mPrefsFile); }
	bool		IsDirty() const				{ return mDirty;  }

	void		Save();
	
  private:
				IniFile();
				~IniFile();

	typedef map<string,vector<string>> PrefsMap;
	
	fs::path	mPrefsFile;
	PrefsMap	mPrefs;
	bool		mDirty;
};

IniFile::IniFile()
	: mDirty(false)
{
	try
	{
		// we use the gPrefsFileName that is located in the directory containing the exe, if it exists...
		if (fs::exists(gExecutablePath))
			mPrefsFile = gExecutablePath.parent_path() / gPrefsFileName;

		if (fs::exists(mPrefsFile))
			gPrefsDir = gExecutablePath.parent_path();
		else
		{
			gPrefsDir = GetPrefsDirectory();

			if (not fs::exists(gPrefsDir))
				fs::create_directories(gPrefsDir);

			mPrefsFile = gPrefsDir / gPrefsFileName;
		}
		
		if (fs::exists(mPrefsFile))
		{
			fs::ifstream data(mPrefsFile, ios::binary);
			
			if (data.is_open())
			{
				xml::document doc(data);
				
				preferences prefs;
				doc.deserialize("preferences", prefs);

				for (const preference& p: prefs.pref)
					mPrefs[p.name] = p.value;
			}
		}
	}
	catch (exception& e)
	{
		cerr << "Exception reading preferences: " << e.what() << endl;		
	}
}

IniFile::~IniFile()
{
	if (mDirty)
		Save();
}

IniFile& IniFile::Instance()
{
	static IniFile sInstance;
	return sInstance;
}

void IniFile::Save()
{
	try
	{
		if (not fs::exists(mPrefsFile))
		{
			if (not fs::exists(gPrefsDir))
				fs::create_directories(gPrefsDir);
		
			mPrefsFile = gPrefsDir / gPrefsFileName;
		}
		
		fs::path tmpPrefs = mPrefsFile.parent_path() / (mPrefsFile.filename().string() + "-new");
		
		fs::ofstream data(tmpPrefs);
	
		if (data.is_open())
		{
			preferences prefs;

			transform(mPrefs.begin(), mPrefs.end(), back_inserter(prefs.pref),
				[](const pair<string,vector<string>>& p) -> preference
				{
					preference pp = { p.first, p.second };
					return pp;
				});

			xml::document doc;
			doc.serialize("preferences", prefs);

			data << doc;
			
			data.close();

			fs::path oldPrefs = mPrefsFile.parent_path() / (mPrefsFile.filename().string() + "-old");
			
			if (fs::exists(mPrefsFile))
				fs::rename(mPrefsFile, oldPrefs);
		
			fs::rename(tmpPrefs, mPrefsFile);
		
			if (fs::exists(oldPrefs))
				fs::remove(oldPrefs);
			
			mDirty = false;
		}
	}
	catch (exception& e)
	{
		PRINT(("Exception writing prefs file: %s", e.what()));
	}
	catch (...) {}
}

void IniFile::SetString(
	const char*		inName,
	const string&	inValue)
{
	vector<string> values = { inValue };
	if (mPrefs[inName] != values)
	{
		mDirty = true;
		mPrefs[inName] = values;
	}
}

string IniFile::GetString(
	const char*		inName,
	const string&	inDefault)
{
	if (mPrefs.find(inName) == mPrefs.end() or mPrefs[inName].size() == 0)
		SetString(inName, inDefault);
	vector<string>& values = mPrefs[inName];
	assert(values.size() == 1);
	if (values.size() != 1)
		cerr << "Inconsistent use of preference array/value" << endl;
	return mPrefs[inName].front();
}

void IniFile::SetStrings(
	const char*				inName,
	const vector<string>&	inValues)
{
	if (mPrefs[inName] != inValues)
	{
		mDirty = true;
		mPrefs[inName] = inValues;
	}
}

void IniFile::GetStrings(
	const char*				inName,
	vector<string>&			outStrings)
{
	outStrings = mPrefs[inName];
}

bool GetBoolean(
	const char*	inName,
	bool		inDefaultValue)
{
	bool result = inDefaultValue;

	try
	{
		string s = GetString(inName, inDefaultValue ? "true" : "false");
		result = (s == "true" or s == "1");
	}
	catch (...)
	{
	}

	return result;
}

void SetBoolean(
	const char*	inName,
	bool		inValue)
{
	SetString(inName, inValue ? "true" : "false");
}

int32_t GetInteger(
	const char*	inName,
	int32_t		inDefaultValue)
{
	int32_t result = inDefaultValue;

	try
	{
		result = boost::lexical_cast<int32_t>(GetString(
			inName, boost::lexical_cast<string>(inDefaultValue)));
	}
	catch (...)
	{
	}

	return result;
}

void SetInteger(
	const char*	inName,
	int32_t		inValue)
{
	SetString(inName, boost::lexical_cast<string>(inValue));
}

string GetString(
	const char*	inName,
	string		inDefaultValue)
{
	return IniFile::Instance().GetString(inName, inDefaultValue);
}

void SetString(
	const char*	inName,
	string		inValue)
{
	IniFile::Instance().SetString(inName, inValue);
}

void
GetArray(
	const char*		inName,
	vector<string>&	outArray)
{
	IniFile::Instance().GetStrings(inName, outArray);
}

void
SetArray(
	const char*				inName,
	const vector<string>&	inArray)
{
	IniFile::Instance().SetStrings(inName, inArray);
}

MColor GetColor(const char* inName, MColor inDefaultValue)
{
	inDefaultValue.hex(GetString(inName, inDefaultValue.hex()));
	return inDefaultValue;
}

void SetColor(const char* inName, MColor inValue)
{
	SetString(inName, inValue.hex());
}

MRect GetRect(const char* inName, MRect inDefault)
{
	stringstream s;
	s << inDefault.x << ' ' << inDefault.y << ' ' << inDefault.width << ' ' << inDefault.height;
	s.str(GetString(inName, s.str()));
	
	s >> inDefault.x >> inDefault.y >> inDefault.width >> inDefault.height;
	return inDefault;
}

void SetRect(const char* inName, MRect inValue)
{
	ostringstream s;
	s << inValue.x << ' ' << inValue.y << ' ' << inValue.width << ' ' << inValue.height;
	SetString(inName, s.str());
}

time_t GetPrefsFileCreationTime()
{
	return IniFile::Instance().GetCreationTime();
}

void Save()
{
	IniFile::Instance().Save();
}

void SaveIfDirty()
{
	auto& instance = IniFile::Instance();
	if (instance.IsDirty())
		instance.Save();
}

}
