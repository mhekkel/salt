//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MCommands.hpp"

extern const uint32_t kValidModifiersMask;

class MAcceleratorTable
{
  public:
					~MAcceleratorTable();

	static MAcceleratorTable&
					Instance();

	static MAcceleratorTable&
					EditKeysInstance();

	void			RegisterAcceleratorKey(uint32_t inCommand, uint32_t inKeyValue, uint32_t inModifiers);
	bool			GetAcceleratorKeyForCommand(uint32_t inCommand, uint32_t& outKeyValue, uint32_t& outModifiers);
	bool			IsAcceleratorKey(uint32_t inKeyCode, uint32_t inModifiers, uint32_t& outCommand);
	bool			IsNavigationKey(uint32_t inKeyValue, uint32_t inModifiers, MKeyCommand& outCommand);
  private:

					MAcceleratorTable();
					MAcceleratorTable(const MAcceleratorTable&);
	MAcceleratorTable&
					operator=(const MAcceleratorTable&);
	
	struct MAcceleratorTableImp*	mImpl;
};
