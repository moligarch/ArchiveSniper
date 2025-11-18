// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here

#include <fstream>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <future>
#include <atomic>

#include "CppUnitTest.h"
#include "ArcSnp/ArchiveSniper.h"
#include "ArcSnp/Shared7z.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#endif //PCH_H
