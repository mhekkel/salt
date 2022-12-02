// Generated revision file

#pragma once

#include <ostream>

const char kProjectName[] = "salt";
const char kVersionNumber[] = "4.0.0";
const char kVersionGitTag[] = "09904ef";
const char kBuildInfo[] = "63*";
const char kBuildDate[] = "2022-12-02T13:16:18Z";

inline void write_version_string(std::ostream &os, bool verbose)
{
	os << kProjectName << " version " << kVersionNumber << std::endl;
	if (verbose)
	{
		os << "build: " << kBuildInfo << ' ' << kBuildDate << std::endl;
		if (kVersionGitTag[0] != 0)
			os << "git tag: " << kVersionGitTag << std::endl;
	}
}
