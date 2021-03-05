//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "MApplication.hpp"

using namespace std;
namespace fs = boost::filesystem;

fs::path	gPrefsDir;
string		gPrefsFileName = string(kAppName) + "-settings";

namespace Preferences
{

class MPreferencesImpl
{
  public:
	
	static MPreferencesImpl&	Instance();
	
	void		SetString(const char* inName, const string& inValue);
	string		GetString(const char* inName, const string& inDefault);

	void		SetStrings(const char* inName, const vector<string>& inValues);
	void		GetStrings(const char* inName, vector<string>& outStrings);

  private:
				MPreferencesImpl();
				~MPreferencesImpl();

	HKEY		mRegKey;
};

MPreferencesImpl& MPreferencesImpl::Instance()
{
	static MPreferencesImpl sInstance;
	return sInstance;
}

MPreferencesImpl::MPreferencesImpl()
	: mRegKey(nullptr)
{
	string key = string("\\Software\\") + kAppName;
	
	if (FAILED(::RegOpenKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, KEY_ALL_ACCESS, &mRegKey))
	{
		DWORD disp;
		(void)::RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, "", 0, KEY_ALL_ACCESS,
			nil, &mRegKey, &disp);
	}
}

MPreferencesImpl::~MPreferencesImpl()
{
	if (mRegKey != nil)
		::RegCloseKey(mRegKey);
}

void MPreferencesImpl::SetString(const char* inName, const string& inValue)
{
}

string MPreferencesImpl::GetString(const char* inName, const string& inDefault)
{
	const char* result = nil;
	std::string key(inName);
	
	HPrefMap::iterator i = fEntries.find(key);
	if (i != fEntries.end())
		result = (*i).second.c_str();
	else if (fRegKey != nil)
	{
		unsigned long size = kMaxValueLength + 1, type;
		HAutoBuf<char> value(new char[size]);
		*value.get() = 0;
		
		long r = ::HRegQueryValueEx(fRegKey,
			inName, nil, &type, value.get(), &size);
		
		if (r == ERROR_SUCCESS)
		{
			assert(type == REG_SZ);
			assert(size < kMaxValueLength);
			fEntries[key] = std::string(value.get());
			result = fEntries[key].c_str();
		}
	}

	return result;
}

void MPreferencesImpl::SetStrings(const char* inName, const vector<string>& inValues)
{
}

void MPreferencesImpl::GetStrings(const char* inName, vector<string>& outStrings)
{
}





struct HPreferencesImp
{
  public:
				HPreferencesImp(const HUrl& inUrl);
				~HPreferencesImp();

	void		Open();
	void		Close();

	const char*	GetIxPrefString(const char* inName, int inIndex);
	const char*	GetPrefString(const char* inName);
	
	void		SetIxPrefString(const char* inName, int inIndex,
					const char* inValue);
	void		SetPrefString(const char* inName, const char* inValue);
	
	void		RemovePref(const char* inName);

	bool		IsNew() const			{ return fIsNew; }
	HUrl		GetURL() const			{ return fUrl; }

  private:
	
	typedef std::map<std::string,std::string>	HPrefMap;

	HKEY		fRegKey;
	HUrl		fUrl;
	bool		fIsNew;
	HPrefMap	fEntries;
};

const int kMaxValueLength = 1024;

HPreferencesImp::HPreferencesImp(const HUrl& inUrl)
	: fRegKey(nil)
	, fUrl(inUrl)
	, fIsNew(false)
{
}

HPreferencesImp::~HPreferencesImp()
{
	try
	{
		Close();
	}
	catch (...) {}
}

void HPreferencesImp::Open()
{
	std::string subKey = fUrl.GetPath();
	
	if (subKey.length() > 0 && subKey[0] == '/')
		subKey.erase(0, 1);
	
	std::string::size_type d = 1;
	while ((d = subKey.find('/', d)) != std::string::npos)
		subKey[d] = '\\';

	long err = ::HRegOpenKeyEx(HKEY_CURRENT_USER,
		subKey.c_str(), 0, KEY_ALL_ACCESS, &fRegKey);
	if (err != ERROR_SUCCESS)
	{
		fIsNew = true;
		unsigned long disp;

		err = ::HRegCreateKeyEx(HKEY_CURRENT_USER,
			subKey.c_str(), 0, "", 0, KEY_ALL_ACCESS,
			nil, &fRegKey, &disp);
	}
}

void HPreferencesImp::Close()
{
	if (fRegKey != nil)
		::RegCloseKey(fRegKey);
	
	HPrefMap m;
	fEntries.swap(m);
}

const char* HPreferencesImp::GetPrefString(const char* inName)
{
	const char* result = nil;
	std::string key(inName);
	
	HPrefMap::iterator i = fEntries.find(key);
	if (i != fEntries.end())
		result = (*i).second.c_str();
	else if (fRegKey != nil)
	{
		unsigned long size = kMaxValueLength + 1, type;
		HAutoBuf<char> value(new char[size]);
		*value.get() = 0;
		
		long r = ::HRegQueryValueEx(fRegKey,
			inName, nil, &type, value.get(), &size);
		
		if (r == ERROR_SUCCESS)
		{
			assert(type == REG_SZ);
			assert(size < kMaxValueLength);
			fEntries[key] = std::string(value.get());
			result = fEntries[key].c_str();
		}
	}

	return result;
}

const char* HPreferencesImp::GetIxPrefString(const char* inName, int inIndex)
{
	const char* result = nil;

	char ext[32];
	std::snprintf(ext, 31, ";%d", inIndex);
	std::string key(inName);
	key += ext;

	HPrefMap::iterator i = fEntries.find(key);

	if (i != fEntries.end())
		result = (*i).second.c_str();
	else if (fRegKey != nil)
	{
		HKEY subKey;
		unsigned long disp;
		
		long r = ::HRegCreateKeyEx(fRegKey, inName,
			0, "", 0, KEY_ALL_ACCESS, nil, &subKey, &disp);
		if (r == ERROR_SUCCESS && subKey != nil)
		{
			unsigned long size = kMaxValueLength + 1, type;
			HAutoBuf<char> value(new char[size]);
			
			*value.get() = 0;
			
			r = ::HRegQueryValueEx(subKey,
				ext, nil, &type, value.get(), &size);
			
			if (r == ERROR_SUCCESS)
			{
				assert(type == REG_SZ);
				assert(size < kMaxValueLength);
				fEntries[key] = std::string(value.get());
				result = fEntries[key].c_str();
			}
			
			::RegCloseKey(subKey);
		}
	}

	return result;
}

void HPreferencesImp::SetPrefString(const char* inName, const char* inValue)
{
	fEntries[std::string(inName)] = std::string(inValue);
	
	if (fRegKey != nil)
		::HRegSetValueEx(fRegKey, inName, 0, REG_SZ, inValue);
}

void HPreferencesImp::SetIxPrefString(const char* inName, int inIndex,
	const char* inValue)
{
	char ext[32];
	std::snprintf(ext, 31, ";%d", inIndex);
	std::string key(inName);
	key += ext;

	fEntries[key] = std::string(inValue);

	if (fRegKey != nil)
	{
		HKEY subKey;
		unsigned long disp;
		
		long r = ::HRegCreateKeyEx(fRegKey, inName,
			0, "", 0, KEY_ALL_ACCESS, nil, &subKey, &disp);
		if (r == ERROR_SUCCESS && subKey != nil)
		{
			::HRegSetValueEx(subKey, ext, 0, REG_SZ, inValue);
			::RegCloseKey(subKey);
		}
	}
}

void HPreferencesImp::RemovePref(const char* inName)
{
	int ix = 0;
	
	for (;;)
	{
		char ext[32];
		std::snprintf(ext, 31, ";%d", ix);
		std::string key(inName);
		key += ext;
		
		HPrefMap::iterator i = fEntries.find(key);
		if (i == fEntries.end())
			break;

		fEntries.erase(i);
		++ix;
	}

	if (fRegKey != nil)
		::HRegDeleteKey(fRegKey, inName);
}

#pragma mark -

HPreferences::HPreferences(const HUrl& inUrl)
	: fImpl(nil)
{
	fImpl = new HPreferencesImp(inUrl);

	OpenPrefFile();
}

HPreferences::~HPreferences()
{
	try
	{
		ClosePrefFile();
	}
	catch (...) {}
	delete fImpl;
}

bool HPreferences::OpenPrefFile()
{
	fImpl->Open();
	return fImpl->IsNew();
}

void HPreferences::ClosePrefFile()
{
	WritePrefFile();
}

const char *HPreferences::GetPrefString(const char *name, const char *def)
{
	const char* result = fImpl->GetPrefString(name);

	if (result == nil && def != nil)
	{
		fImpl->SetPrefString(name, def);
		result = def;
	}

	return result;
}

const char *HPreferences::GetIxPrefString(const char *name, int ix)
{
	return fImpl->GetIxPrefString(name, ix);
}

int HPreferences::GetPrefInt(const char *name, int def)
{
	char val[32];
	std::snprintf(val, 32, "%d", def);
	return std::atoi(GetPrefString(name, val));
}

double HPreferences::GetPrefDouble(const char *name, double def)
{
	char val[32];
	std::snprintf(val, 32, "%g", def);
	return std::atof(GetPrefString(name, val));
}

HColor HPreferences::GetPrefColor(const char *name, HColor def)
{
	char cs[12];
	std::snprintf(cs, 12, "#%2.2x%2.2x%2.2x", def.red, def.green, def.blue);
	const char* v = GetPrefString(name, cs);

	HColor c;
	char s[4], *p;
		
	s[2] = 0;
		
	std::strncpy(s, v + 1, 2);
	assert (std::strtoul(s, &p, 16) < 256);
	c.red = static_cast<unsigned char> (std::strtoul(s, &p, 16));
		
	std::strncpy(s, v + 3, 2);
	assert (std::strtoul(s, &p, 16) < 256);
	c.green = static_cast<unsigned char> (std::strtoul(s, &p, 16));
		
	std::strncpy(s, v + 5, 2);
	assert (std::strtoul(s, &p, 16) < 256);
	c.blue = static_cast<unsigned char> (std::strtoul(s, &p, 16));
		
	return c;
}

HRect HPreferences::GetPrefRect(const char *name, const HRect& def)
{
	char rs[64];
	std::snprintf(rs, 64, "%ld,%ld,%ld,%ld", def.left, def.top, def.right, def.bottom);

	const char* v = GetPrefString(name, rs);
	char *p;

	HRect r;

	r.left = std::strtol(v, &p, 10);
	p++;
	r.top = std::strtol(p, &p, 10);
	p++;
	r.right = std::strtol(p, &p, 10);
	p++;
	r.bottom = std::strtol(p, &p, 10);
		
	return r.IsValid() ? r : def;
}

//void HPreferences::GetPrefBlob(const char* /*inName*/, void* /*outBlob*/, int /*inMaxSize*/,
//	int& /*outSize*/)
//{
//	assert(false);
//}

void HPreferences::SetPrefString(const char *name, const char *value)
{
	fImpl->SetPrefString(name, value);
}

void HPreferences::SetIxPrefString(const char *name, int ix, const char *value)
{
	fImpl->SetIxPrefString(name, ix, value);
}

void HPreferences::SetPrefInt(const char *name, int value)
{
	char s[10];
	std::snprintf(s, 10, "%d", value);
	SetPrefString(name, s);
}

void HPreferences::SetPrefDouble(const char *name, double value)
{
	char s[20];
	std::snprintf(s, 20, "%g", value);
	SetPrefString(name, s);
}

void HPreferences::SetPrefColor(const char *name, HColor value)
{
	char c[12];
	std::snprintf(c, 12, "#%2.2x%2.2x%2.2x", value.red, value.green, value.blue);
	SetPrefString(name, c);
}

void HPreferences::SetPrefRect(const char *name, const HRect& value)
{
	char s[64];
	std::snprintf(s, 64, "%ld,%ld,%ld,%ld", value.left, value.top, value.right, value.bottom);
	SetPrefString(name, s);
}

//void HPreferences::SetPrefBlob(const char* /*inName*/, const void* /*inBlob*/, int /*inBlobSize*/)
//{
//	assert(false);
//}

void HPreferences::RemovePref(const char *name)
{
	fImpl->RemovePref(name);
}

void HPreferences::ResetAll()
{
	HAutoPtr<HPreferencesImp>
		obj(new HPreferencesImp(fImpl->GetURL()));
	delete fImpl;
	fImpl = obj.release();
}

void HPreferences::ReadPrefFile()
{
}

void HPreferences::WritePrefFile()
{
}

HUrl HPreferences::GetPrefFileURL() const
{
	return fImpl->GetURL();
}

bool HPreferences::IsNewPrefFile() const
{
	return fImpl->IsNew();
}

}
