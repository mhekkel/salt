//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

struct MFileImpl
{
							MFileImpl(const fs::path& inPath)
								: mPath(inPath)
								, mReadOnly(false)
								, mIODocument(nullptr) {}

	virtual					~MFileImpl() {}

	void					Load(MDocument* inDocument);
	void					Save(MDocument* inDocument);
	bool					IsIOActive() const				{ return mIODocument != nullptr; }

	virtual fs::path		GetPath() const					{ return mPath; }

	virtual void			SetPath(fs::path inPath)		{ mPath = inPath; }
	virtual void			SetFileName(const std::string& inFileName)
															{ mPath = mPath.parent_path() / inFileName; }
	
	virtual std::string		GetScheme() const				{ return "file"; }
	virtual std::string		GetURL() const;
	virtual bool			IsLocal() const;
	virtual bool			IsReadOnly() const				{ return mReadOnly; }
	virtual fs::file_time_type		GetModDate() const				{ return mModDate; }
	virtual bool			IsModifiedOnDisk();

	// Override the following three for other scheme's
	virtual bool			Equivalent(const MFileImpl* rhs) const;
	virtual void			DoLoad();
	virtual void			DoSave();
	virtual void			CancelIO();

  protected:
	fs::path				mPath;
	bool					mReadOnly;
	fs::file_time_type		mModDate;
	MDocument*				mIODocument;
};
