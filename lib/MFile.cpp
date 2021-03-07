//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <cstring>

#include <regex>
#include <sys/stat.h>
#include <stack>
#include <fstream>
#include <cassert>
#include <cerrno>
#include <limits>
#include <filesystem>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include "MFile.hpp"
#include "MDocument.hpp"
#include "MError.hpp"
#include "MUnicode.hpp"
#include "MUtils.hpp"
#include "MStrings.hpp"
#include "MPreferences.hpp"
//#include "MSftpChannel.hpp"
#include "MFileImpl.hpp"

using namespace std;
namespace io = boost::iostreams;
namespace ba = boost::algorithm;

// These are declarations of file attribute I/O routines, very OS specific
// and so moved to the OS implemention part.
int32_t read_attribute(const fs::path& inPath, const char* inName, void* outData, size_t inDataSize);
int32_t write_attribute(const fs::path& inPath, const char* inName, const void* inData, size_t inDataSize);

bool FileIsReadOnly(const fs::path& inPath);

namespace {

// reserved characters in URL's

unsigned char kURLAcceptable[96] =
{/* 0 1 2 3 4 5 6 7 8 9 A B C D E F */
    0,0,0,0,0,0,0,0,0,0,7,6,0,7,7,4,		/* 2x   !"#$%&'()*+,-./	 */
    7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,0,		/* 3x  0123456789:;<=>?	 */
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,		/* 4x  @ABCDEFGHIJKLMNO  */
    7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,7,		/* 5X  PQRSTUVWXYZ[\]^_	 */
    0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,		/* 6x  `abcdefghijklmno	 */
    7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0			/* 7X  pqrstuvwxyz{\}~	DEL */
};

inline char ConvertHex(
	char		inChar)
{
	int	value;

	if (inChar >= 'A' and inChar <= 'F')
		value = inChar - 'A' + 10;
	else if (inChar >= 'a' and inChar <= 'f')
		value = inChar - 'a' + 10;
	else
		value = inChar - '0';

	return char(value);
}

}

// --------------------------------------------------------------------

void URLEncode(
	string&		ioPath)
{
	string path;
	
	swap(path, ioPath);
	
	for (unsigned int i = 0; i < path.length(); ++i)
	{
		unsigned char a = (unsigned char)path[i];
		if (not (a >= 32 and a < 128 and (kURLAcceptable[a - 32] & 4)))
		{
			ioPath += '%';
			ioPath += kHexChars[a >> 4];
			ioPath += kHexChars[a & 15];
		}
		else
			ioPath += path[i];
	}
}

void URLDecode(
	string&		ioPath)
{
	vector<char> buf(ioPath.length() + 1);
	char* r = &buf[0];
	
	for (string::iterator p = ioPath.begin(); p != ioPath.end(); ++p)
	{
		char q = *p;

		if (q == '%' and ++p != ioPath.end())
		{
			q = (char) (ConvertHex(*p) * 16);

			if (++p != ioPath.end())
				q = (char) (q + ConvertHex(*p));
		}

		*r++ = q;
	}
	
	ioPath.assign(&buf[0], r);
}

// --------------------------------------------------------------------
// MFileImpl, the base implementation class for local files

void MFileImpl::Load(MDocument* inDocument)
{
	assert(mIODocument == nullptr);
	if (mIODocument != nullptr)
		THROW(("IO already active for file"));
	try
	{
		mIODocument = inDocument;
		DoLoad();
	}
	catch (...)
	{
		mIODocument = nullptr;
		throw;
	}
}

void MFileImpl::Save(MDocument* inDocument)
{
	assert(mIODocument == nullptr);
	if (mIODocument != nullptr)
		THROW(("IO already active for file"));

	try
	{
		mIODocument = inDocument;
		DoSave();
	}
	catch (...)
	{
		mIODocument = nullptr;
		throw;
	}
}

string MFileImpl::GetURL() const
{
	string path = GetPath().string();
	ba::replace_all(path, "\\", "/");
	URLEncode(path);
	return GetScheme() + "://" + path;
}

bool MFileImpl::IsLocal() const
{
	string path = GetPath().string();
	return ba::starts_with(path, "//");
}

bool MFileImpl::Equivalent(const MFileImpl* rhs) const
{
	return GetScheme() == rhs->GetScheme() and fs::equivalent(mPath, rhs->GetPath());
}

void MFileImpl::DoLoad()
{
	if (not fs::exists(mPath))
		THROW(("File %s does not exist", mPath.string().c_str()));
		
	mModDate = fs::last_write_time(mPath);
	mReadOnly = FileIsReadOnly(mPath);
	
	std::ifstream file(mPath, ios::binary);
	mIODocument->IOReadFile(file);
	mIODocument->IOProgress(1.0f, "");

	mIODocument = nullptr;
}

void MFileImpl::DoSave()
{
	std::ofstream file(mPath, ios::trunc|ios::binary);
		
	if (not file.is_open())
		THROW(("Could not open file %s for writing", mPath.filename().c_str()));
		
	mIODocument->IOWriteFile(file);
	file.close();

	mModDate = fs::last_write_time(mPath);
	mReadOnly = false;

	mIODocument->IOProgress(1.0f, "");

	mIODocument = nullptr;
}

void MFileImpl::CancelIO()
{
	mIODocument = nullptr;
}

bool MFileImpl::IsModifiedOnDisk()
{
	bool result = false;
	// if (mModDate)
	// {
		auto modDate = fs::last_write_time(mPath);
		if (modDate > mModDate)
		{
			mModDate = modDate;
			result = true;
		}
	// }
	return result;
}

//// --------------------------------------------------------------------
//// MSFTPFileImpl, the implementation for accessing files over SSH/SFTP
//
//struct MSFTPFileImpl : public MFileImpl
//{
//					MSFTPFileImpl(const string& inHost, const string& inUser, uint16_t inPort, const fs::path& inPath);
//	virtual			~MSFTPFileImpl();
//
//	virtual string	GetURL() const;
//	virtual string	GetScheme() const				{ return "sftp"; }
//	virtual bool	IsLocal() const					{ return false; }
//	virtual bool	IsModifiedOnDisk();
//	virtual bool	Equivalent(const MFileImpl* rhs) const;
//	virtual void	DoLoad();
//	virtual void	DoSave();
//	
//	virtual void	CancelIO();
//
//  protected:
//	void			SFTPInitialised();
//
//	void			ChannelMessage(const string& inMessage);
//	void			ChannelError(const string& inError);
//	void			SendData(int64_t inOffset, uint32_t inMaxSize, string& outData);
//	void			ReceiveData(const string& inData, int64_t inOffset, int64_t inFileSize);
//	void			FileClosed();
//
//	MSftpChannel*	mChannel;
//	string			mHost, mUser;
//	uint16_t			mPort;
//	string			mData;
//};
//
//MSFTPFileImpl::MSFTPFileImpl(const string& inHost, const string& inUser, uint16_t inPort, const fs::path& inPath)
//	: MFileImpl(inPath)
//	, mHost(inHost)
//	, mUser(inUser)
//	, mPort(inPort)
//	, mChannel(nullptr)
//{
//}
//
//MSFTPFileImpl::~MSFTPFileImpl()
//{
//	CancelIO();
//}
//
//string MSFTPFileImpl::GetURL() const
//{
//	string result = "sftp://";
//	if (not mUser.empty())
//		result += mUser + '@';
//	result += mHost + "/";
//
//	string path = mPath.string();
//	ba::replace_all(path, "\\", "/");
//	URLEncode(path);
//
//	result += path;
//
//	return result;
//}
//
//void MSFTPFileImpl::CancelIO()
//{
//	if (mChannel != nullptr)
//	{
//		mChannel->eSendData.clear();
//		mChannel->eReceiveData.clear();
//		//mChannel->eChannelMessage.clear();
//		//mChannel->eChannelError.clear();
//		mChannel->eSFTPInitialised.clear();
//		mChannel->eFileClosed.clear();
//		
//		//if (mChannel->IsOpen())
//		//	mChannel->Close();
//
//		mChannel->Release();
//		mChannel = nullptr;
//	}
//
//	mData = string();
//	
//	MFileImpl::CancelIO();
//}
//
//bool MSFTPFileImpl::IsModifiedOnDisk()
//{
//	return false;
//}
//
//bool MSFTPFileImpl::Equivalent(const MFileImpl* rhs) const
//{
//	bool result = false;
//
//	const MSFTPFileImpl* ri = dynamic_cast<const MSFTPFileImpl*>(rhs);
//	if (ri != nullptr)
//	{
//		fs::path a(mPath);			NormalizePath(a);
//		fs::path b(ri->mPath);		NormalizePath(b);
//
//		result = mHost == ri->mHost and mUser == ri->mUser and mPort == ri->mPort and a == b;
//	}
//
//	return result;
//}
//
//void MSFTPFileImpl::DoLoad()
//{
//	mData.clear();
//	
//	assert(mChannel == nullptr);
//	mChannel = new MSftpChannel(mHost, mUser, mPort);
//
//	mChannel->eReceiveData = std::bind(&MSFTPFileImpl::ReceiveData, this, _1, _2, _3);
//	//mChannel->eChannelMessage = std::bind(&MSFTPFileImpl::ChannelMessage, this, _1);
//	//mChannel->eChannelError = std::bind(&MSFTPFileImpl::ChannelError, this, _1);
//	mChannel->eSFTPInitialised = std::bind(&MSFTPFileImpl::SFTPInitialised, this);
//	mChannel->eFileClosed = std::bind(&MSFTPFileImpl::FileClosed, this);
//
//	mChannel->Open();
//}
//
//void MSFTPFileImpl::DoSave()
//{
//	mData.clear();
//	
//	io::filtering_ostream out(io::back_inserter(mData));
//	mIODocument->IOWriteFile(out);
//
//	assert(mChannel == nullptr);
//	mChannel = new MSftpChannel(mHost, mUser, mPort);
//
//	mChannel->eSendData = std::bind(&MSFTPFileImpl::SendData, this, _1, _2, _3);
//	//mChannel->eChannelMessage = std::bind(&MSFTPFileImpl::ChannelMessage, this, _1);
//	//mChannel->eChannelError = std::bind(&MSFTPFileImpl::ChannelError, this, _1);
//	mChannel->eSFTPInitialised = std::bind(&MSFTPFileImpl::SFTPInitialised, this);
//	mChannel->eFileClosed = std::bind(&MSFTPFileImpl::FileClosed, this);
//
//	mChannel->Open();
//}
//
//void MSFTPFileImpl::SFTPInitialised()
//{
//	if (mChannel->eReceiveData)
//		mChannel->ReadFile(mPath.string());
//	else
//		mChannel->WriteFile(mPath.string());
//}
//
//void MSFTPFileImpl::ReceiveData(const string& inData, int64_t inOffset, int64_t inFileSize)
//{
//	mData += inData;
//
//	if (inOffset + inData.length() == inFileSize)
//	{
//		stringstream in(mData);
//		mIODocument->IOReadFile(in);
//	}
//	else
//		mIODocument->IOProgress(float(inOffset + inData.length()) / inFileSize, _("Receiving data"));
//}
//
//void MSFTPFileImpl::SendData(int64_t inOffset, uint32_t inMaxSize, string& outData)
//{
//	int64_t n = inMaxSize;
//
//	if (n > mData.length() - inOffset)
//		n = mData.length() - inOffset;
//
//	if (n > 0)
//	{
//		mIODocument->IOProgress(float(inOffset) / mData.length(), _("Sending data"));
//		outData = mData.substr(static_cast<string::size_type>(inOffset), static_cast<string::size_type>(n));
//	}
//	//vector<char> buffer(inMaxSize);
//	//streamsize n = mData.readsome(&buffer[0], inMaxSize);
//
//	//if (n > 0)
//	//{
//	//	mIODocument->IOProgress(float(inOffset) / mDataLength, _("Sending data"));
//	//	outData.assign(&buffer[0], n);
//	//}
//}
//
//void MSFTPFileImpl::FileClosed()
//{
//	mIODocument->IOProgress(1.0f, _("File closed"));
//	
//	CancelIO();
//}
//
//void MSFTPFileImpl::ChannelError(const string& inError)
//{
//	mIODocument->IOError(inError);
//
//	CancelIO();
//}
//
//void MSFTPFileImpl::ChannelMessage(const string& inMessage)
//{
//	float fraction = -1.0f;
//	mIODocument->IOProgress(fraction, inMessage);
//}

// --------------------------------------------------------------------
// The MFile class

MFile::MFile()
	: mImpl(nullptr)
{
}

MFile::~MFile()
{
	delete mImpl;
}

MFile::MFile(const fs::path& inPath)
	: mImpl(new MFileImpl(inPath))
{
}

MFile::MFile(const string& inURL, bool isAbsoluteURL)
	: mImpl(CreateImpl(inURL, isAbsoluteURL))
{
}

MFileImpl* MFile::CreateImpl(const string& inURL, bool isAbsoluteURL)
{
	MFileImpl* result = nullptr;

	std::regex re("^([-+a-zA-Z]+)://(.+)");
	std::smatch m;
	if (std::regex_match(inURL, m, re))
	{
		string scheme = ba::to_lower_copy(m[1].str());
		string path = m[2];
		
		URLDecode(path);
		
		if (scheme == "file")
			result = new MFileImpl(fs::canonical(path));
		//else if (scheme == "sftp" or scheme == "ssh")
		//{
		//	std::regex re2("^(?:([-$_.+!*'(),[:alnum:];?&=]+)(?::([-$_.+!*'(),[:alnum:];?&=]+))?@)?([-[:alnum:].]+)(?::(\\d+))?/(.+)");
		//	
		//	if (std::regex_match(path, m, re2))
		//	{
		//		string username = m[1];
		//		string password = m[2];
		//		string host = m[3];
		//		string port = m[4];
		//		string file = m[5];

		//		if (port.empty())
		//			port = "22";

		//		if (isAbsoluteURL)
		//			file.insert(file.begin(), '/');

		//		result = new MSFTPFileImpl(host, username, std::to_string(port), file);
		//	}
		//	else
		//		THROW(("Malformed URL: '%s'", inURL.c_str()));
		//}
		else
			THROW(("Unsupported URL scheme '%s'", scheme.c_str()));
	}
	else // assume it is a simple path
		result = new MFileImpl(fs::canonical(inURL));
	
	return result;
}

bool MFile::operator==(const MFile& rhs) const
{
	return mImpl != nullptr and rhs.mImpl != nullptr and mImpl->Equivalent(rhs.mImpl);
}

fs::path MFile::GetPath() const
{
	fs::path result;
	if (mImpl != nullptr)
		result = mImpl->GetPath();
	return result;
}

void MFile::SetPath(fs::path inPath)
{
	if (mImpl != nullptr)
		mImpl->SetPath(inPath);
	else
		mImpl = CreateImpl(inPath.string(), inPath.is_absolute());
}

void MFile::SetFileName(const std::string& inFileName)
{
	if (mImpl != nullptr)
		mImpl->SetFileName(inFileName);
}

string MFile::GetURL() const
{
	string result;
	if (mImpl != nullptr)
		result = mImpl->GetURL();
	return result;
}

void MFile::SetURL(const string& inURL, bool isAbsoluteURL)
{
	MFileImpl* impl = CreateImpl(inURL, isAbsoluteURL);
	delete mImpl;
	mImpl = impl;
}

string MFile::GetScheme() const
{
	string result;
	if (mImpl != nullptr)
		result = mImpl->GetScheme();
	return result;
}

bool MFile::IsIOActive() const
{
	return mImpl != nullptr and mImpl->IsIOActive();
}

void MFile::CancelIO()
{
	if (mImpl != nullptr)
		mImpl->CancelIO();
}

void MFile::Load(MDocument* inDocument)
{
	if (mImpl == nullptr)
		THROW(("Cannot load unspecified file"));
	return mImpl->Load(inDocument);
}

void MFile::Save(MDocument* inDocument)
{
	if (mImpl == nullptr)
		THROW(("Cannot load unspecified file"));
	return mImpl->Save(inDocument);
}

bool MFile::IsValid() const
{
	return mImpl != nullptr and not mImpl->GetPath().empty();
}

bool MFile::IsLocal() const
{
	return mImpl != nullptr and mImpl->GetScheme() == "file";
}

bool MFile::Exists() const
{
	bool result = false;

	try
	{
//		assert(IsLocal());
		result = IsLocal();
		if (result)
			result = fs::exists(GetPath());
	}
	catch (...) {}

	return result;
}

fs::file_time_type MFile::GetModDate() const
{
	return mImpl ? mImpl->GetModDate() : fs::file_time_type{};
}

bool MFile::IsModifiedOnDisk() const
{
	return mImpl != nullptr and mImpl->IsModifiedOnDisk();
}

bool MFile::IsReadOnly() const
{
	return mImpl != nullptr and mImpl->IsReadOnly();
}

int32_t MFile::ReadAttribute(const char* inName, void* outData, uint32_t inDataSize) const
{
	int32_t result = 0;
	
	if (IsLocal())
		result = read_attribute(GetPath(), inName, outData, inDataSize);
	
	return result;
}

int32_t MFile::WriteAttribute(const char* inName, const void* inData, uint32_t inDataSize) const
{
	uint32_t result = 0;
	
	if (IsLocal())
	{
		write_attribute(GetPath(), inName, inData, inDataSize);
		result = inDataSize;
	}
	
	return result;
}

// ----------------------------------------------------------------------------
// Glob like routines

namespace {

bool Match(const char* inPattern, const char* inName)
{
	for (;;)
	{
		char op = *inPattern;

		switch (op)
		{
			case 0:
				return *inName == 0;
			case '*':
			{
				if (inPattern[1] == 0)	// last '*' matches all 
					return true;

				const char* n = inName;
				while (*n)
				{
					if (Match(inPattern + 1, n))
						return true;
					++n;
				}
				return false;
			}
			case '?':
				if (*inName)
					return Match(inPattern + 1, inName + 1);
				else
					return false;
			default:
				if (tolower(*inName) == tolower(op))
				{
					++inName;
					++inPattern;
				}
				else
					return false;
				break;
		}
	}
}

}

bool FileNameMatches(
	const char*		inPattern,
	const fs::path&	inFile)
{
	return FileNameMatches(inPattern, inFile.filename().string());
}

bool FileNameMatches(
	const char*		inPattern,
	const string&	inFile)
{
	bool result = false;
	
	if (inFile.length() > 0)
	{
		string p(inPattern);
	
		while (not result and p.length())
		{
			string::size_type s = p.find(';');
			string pat;
			
			if (s == string::npos)
			{
				pat = p;
				p.clear();
			}
			else
			{
				pat = p.substr(0, s);
				p.erase(0, s + 1);
			}
			
			result = Match(pat.c_str(), inFile.c_str());
		}
	}
	
	return result;	
}

// ------------------------------------------------------------

struct MFileIteratorImp
{
						MFileIteratorImp()
							: mReturnDirs(false) {}
	virtual				~MFileIteratorImp() {}
	
	virtual	bool		Next(fs::path& outFile) = 0;
	
	string				mFilter;
	bool				mReturnDirs;
};

struct MSingleFileIteratorImp : public MFileIteratorImp
{
						MSingleFileIteratorImp(
							const fs::path&	inDirectory);

	virtual	bool		Next(
							fs::path&			outFile);
	
	fs::directory_iterator
						mIter;
};

MSingleFileIteratorImp::MSingleFileIteratorImp(
	const fs::path&	inDirectory)
	: mIter(inDirectory)
{
}

bool MSingleFileIteratorImp::Next(
	fs::path&			outFile)
{
	bool result = false;
	
	for (; result == false and mIter != fs::directory_iterator(); ++mIter)
	{
		if (fs::is_directory(*mIter) and not mReturnDirs)
			continue;
		
		if (mFilter.empty() or
			FileNameMatches(mFilter.c_str(), *mIter))
		{
			outFile = *mIter;
			result = true;
		}
	}
	
	return result;
}

struct MDeepFileIteratorImp : public MFileIteratorImp
{
						MDeepFileIteratorImp(
							const fs::path&	inDirectory);

	virtual	bool		Next(fs::path& outFile);

	fs::recursive_directory_iterator
						mIter;
};

MDeepFileIteratorImp::MDeepFileIteratorImp(const fs::path& inDirectory)
	: mIter(inDirectory)
{
}

bool MDeepFileIteratorImp::Next(
	fs::path&		outFile)
{
	bool result = false;
	
	for (; result == false and mIter != fs::recursive_directory_iterator(); ++mIter)
	{
		if (fs::is_directory(*mIter) and not mReturnDirs)
			continue;

		if (mFilter.empty() or
			FileNameMatches(mFilter.c_str(), *mIter))
		{
			outFile = *mIter;
			result = true;
		}
	}
	
	return result;
}

MFileIterator::MFileIterator(
	const fs::path&	inDirectory,
	uint32_t			inFlags)
	: mImpl(nullptr)
{
	if (inFlags & kFileIter_Deep)
		mImpl = new MDeepFileIteratorImp(inDirectory);
	else
		mImpl = new MSingleFileIteratorImp(inDirectory);
	
	mImpl->mReturnDirs = (inFlags & kFileIter_ReturnDirectories) != 0;
}

MFileIterator::~MFileIterator()
{
	delete mImpl;
}

bool MFileIterator::Next(
	fs::path&			outFile)
{
	return mImpl->Next(outFile);
}

void MFileIterator::SetFilter(
	const string&	inFilter)
{
	mImpl->mFilter = inFilter;
}

// ----------------------------------------------------------------------------
//	relative_path

fs::path relative_path(const fs::path& inFromDir, const fs::path& inFile)
{
	fs::path::iterator d = inFromDir.begin();
	fs::path::iterator f = inFile.begin();
	
	while (d != inFromDir.end() and f != inFile.end() and *d == *f)
	{
		++d;
		++f;
	}
	
	fs::path result;
	
	if (d == inFromDir.end() and f == inFile.end())
		result = ".";
	else
	{
		while (d != inFromDir.end())
		{
			result /= "..";
			++d;
		}
		
		while (f != inFile.end())
		{
			result /= *f;
			++f;
		}
	}

	return result;
}

void NormalizePath(
	fs::path&	ioPath)
{
	string p = ioPath.string();
	NormalizePath(p);
	ioPath = p;
}

void NormalizePath(string& ioPath)
{
	string path(ioPath);	
	stack<unsigned long> dirs;
	int r = 0;
	unsigned long i = 0;
	
	dirs.push(0);

	bool unc = false;

	if (path.length() > 2 and path[0] == '/' and path[1] == '/')
	{
		unc = true;
		++i;
	}
	
	while (i < path.length())
	{
		while (i < path.length() and path[i] == '/')
		{
			++i;
			if (not dirs.empty())
				dirs.top() = i;
			else
				dirs.push(i);
		}
		
		if (path[i] == '.' and path[i + 1] == '.' and path[i + 2] == '/')
		{
			if (not dirs.empty())
				dirs.pop();
			if (dirs.empty())
				--r;
			i += 2;
			continue;
		}
		else if (path[i] == '.' and path[i + 1] == '/')
		{
			i += 1;
			if (not dirs.empty())
				dirs.top() = i;
			else
				dirs.push(i);
			continue;
		}

		unsigned long d = path.find('/', i + 1);

		if (d == string::npos)
			break;
		
		i = d + 1;
		dirs.push(i);
	}
	
	if (not dirs.empty() and dirs.top() == path.length())
		ioPath.assign("/");
	else
		ioPath.erase(ioPath.begin(), ioPath.end());
	
	bool dir = false;
	while (not dirs.empty())
	{
		unsigned long l, n;
		n = path.find('/', dirs.top());
		if (n == string::npos)
			l = path.length() - dirs.top();
		else
			l = n - dirs.top();
		
		if (l > 0)
		{
			if (dir)
				ioPath.insert(0, "/");
			ioPath.insert(0, path.c_str() + dirs.top(), l);
			dir = true;
		}
		
		dirs.pop();
	}
	
	if (r < 0)
	{
		ioPath.insert(0, "../");
		while (++r < 0)
			ioPath.insert(0, "../");
	}
	else if (path.length() > 0 and path[0] == '/' and ioPath[0] != '/')
		ioPath.insert(0, "/");

	if (unc and ioPath[0] == '/')
		ioPath.insert(ioPath.begin(), '/');
}
