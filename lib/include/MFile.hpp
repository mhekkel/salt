//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "MWindow.hpp"

#include <filesystem>
#include <functional>

namespace MFileDialogs
{

// bool ChooseDirectory(MWindow *inParent, fs::path &outDirectory);
// bool ChooseOneFile(MWindow *inParent, fs::path &ioFile);
// bool ChooseFiles(MWindow *inParent, bool inLocalOnly, std::vector<fs::path> &outFiles);

void ChooseOneFile(MWindow *inParent, std::function<void(std::filesystem::path)> &&callback);
void SaveFileAs(MWindow *inParent, std::filesystem::path filename, std::function<void(std::filesystem::path)> &&callback);

} // namespace MFileDialogs
