//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*	$Id: MClipboard.h 158 2007-05-28 10:08:18Z maarten $
	Copyright Maarten L. Hekkelman
	Created Wednesday July 21 2004 18:18:19
*/

#ifndef MCLIPBOARD_H
#define MCLIPBOARD_H

#include <string>
#include "MTypes.hpp"

const uint32
	kClipboardRingSize = 7;

class MClipboard
{
  public:

	static MClipboard&	Instance();

	bool				HasData();
	bool				IsBlock();
	void				NextInRing();
	void				PreviousInRing();
	void				GetData(std::string& outText, bool& outIsBlock);
	void				SetData(const std::string& inText, bool inBlock);
	void				AddData(const std::string& inText);

	struct Data
	{
						Data(const std::string& inText, bool inBlock);
		void			SetData(const std::string& inText, bool inBlock);
		void			AddData(const std::string& inText);
		
		std::string		mText;
		bool			mBlock;
	};

  private:

						MClipboard();
	virtual				~MClipboard();

	class MClipboardImpl*
						mImpl;

	Data*				mRing[kClipboardRingSize];
	uint32				mCount;
};

#endif // MCLIPBOARD_H
