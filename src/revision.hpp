// Generated revision file

#pragma once

#include <ostream>

const char kProjectName[] = "salt";
const char kVersionNumber[] = "4.0.0";
const char kVersionGitTag[] = "415428e";
const char kBuildInfo[] = "62*";
const char kBuildDate[] = "2022-12-02T09:46:03Z";

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
