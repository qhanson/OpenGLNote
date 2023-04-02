#pragma once

#ifdef _MSC_VER
#if _MSC_VER >= 1923
#define USE_STD_FILESYSTEM 1
#else
#include <experimental/filesystem>
#endif
#else
#include <experimental/filesystem>
#endif
#ifdef USE_STD_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else:
namespace fs = std::experimental::filesystem;
#endif