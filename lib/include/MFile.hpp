//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <vector>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

class MDocument;
class MWindow;

// --------------------------------------------------------------------
// MFile, a link to something on a disk. Used for I/O. Works with MDocument.

class MFile
{
  public:
				MFile();
	explicit	MFile(const fs::path& inPath);
				// GTK has the nasty habit of turning URL's into absolute URL's...
				MFile(const std::string& inURL, bool isAbsoluteURL = false);
				~MFile();

	bool		operator==(const MFile& rhs) const;

	std::string	GetURL() const;
	void		SetURL(const std::string& inURL, bool isAbsoluteURL = false);

	std::string	GetScheme() const;
	fs::path	GetPath() const;
	
	void		SetPath(fs::path inPath);
	void		SetFileName(const std::string& inName);

	bool		IsValid() const;
	bool		IsLocal() const;
	bool		IsReadOnly() const;
	bool		Exists() const;
	fs::file_time_type	GetModDate() const;
	bool		IsModifiedOnDisk() const;

	int32_t		ReadAttribute(const char* inName, void* outData, uint32_t inDataSize) const;
	int32_t		WriteAttribute(const char* inName, const void* inData, uint32_t inDataSize) const;

	// IO to/from MDocument.
	void		Load(MDocument* inDocument);
	void		Save(MDocument* inDocument);
	bool		IsIOActive() const;
	void		CancelIO();

	typedef std::function<struct MFileImpl*(const std::string&,bool)> ImplCreatorFunc;
	static void	RegisterImplCreator(const char* inScheme, ImplCreatorFunc inFunc);

  private:
	
	static struct MFileImpl*
				CreateImpl(const std::string& inURL, bool inIsAbsolute);

				MFile(const MFile& rhs);
	MFile&		operator=(const MFile& rhs);
	
	struct MFileImpl*	mImpl;
};

// --------------------------------------------------------------------
//  

enum {
	kFileIter_Deep				= 1 << 0,
	kFileIter_ReturnDirectories	= 1 << 1
};

class MFileIterator
{
  public:
					MFileIterator(const fs::path& inDirectory, uint32_t inFlags);

					~MFileIterator();

	void			SetFilter(const std::string& inFilter);
	
	bool			Next(fs::path& outFile);

  private:

					MFileIterator(const MFileIterator&);
	MFileIterator&	operator=(const MFileIterator&);

	struct MFileIteratorImp*
					mImpl;
};

namespace MFileDialogs
{
	bool ChooseDirectory(MWindow* inParent, fs::path& outDirectory);
	bool ChooseOneFile(MWindow* inParent, fs::path& ioFile);
	bool ChooseFiles(MWindow* inParent, bool inLocalOnly, std::vector<fs::path>& outFiles);
	bool SaveFileAs(MWindow* inParent, fs::path& ioFile);
}

bool FileNameMatches(const char* inPattern, const fs::path& inFile);
bool FileNameMatches(const char* inPattern, const std::string& inFile);

fs::path relative_path(const fs::path& inFromDir, const fs::path& inFile);

void NormalizePath(std::string& ioPath);
void NormalizePath(fs::path& ioPath);

void URLDecode(std::string& ioURL);
void URLEncode(std::string& ioURL);

std::time_t GetFileCreationTime(const fs::path& inFile);
