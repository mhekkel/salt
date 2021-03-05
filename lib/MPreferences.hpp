//          Copyright Maarten L. Hekkelman 2006-2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>

#include "MColor.hpp"
#include "MTypes.hpp"

extern boost::filesystem::path	gPrefsDir;
extern std::string				gPrefsFileName;

namespace Preferences
{

bool	GetBoolean(const char* inName, bool inDefaultValue);
void	SetBoolean(const char* inName, bool inValue);

int32	GetInteger(const char* inName, int32 inDefaultValue);
void	SetInteger(const char* inName, int32 inValue);

std::string	GetString(const char* inName, std::string inDefaultValue);
void	SetString(const char* inName, std::string inValue);

void	GetArray(const char* inName, std::vector<std::string>& outArray);
void	SetArray(const char* inName, const std::vector<std::string>& inArray);

MColor	GetColor(const char* inName, MColor inDefaultValue);
void	SetColor(const char* inName, MColor inValue);

MRect	GetRect(const char* inName, MRect inDefault);
void	SetRect(const char* inName, MRect inValue);

std::time_t
		GetPrefsFileCreationTime();

void	Save();
void	SaveIfDirty();

}
