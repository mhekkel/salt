// Generated revision file

#pragma once

#include <ostream>

const char kProjectName[] = "salt";
const char kVersionNumber[] = "4.0.0";
const char kVersionGitTag[] = "";
const char kBuildInfo[] = "-128-NOTFOUND";
const char kBuildDate[] = "2022-12-01T14:35:03Z";

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
