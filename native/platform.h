#pragma once

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER)
#define CM5_PLATFORM_WINDOWS
#elif defined(__linux__)
#define CM5_PLATFORM_LINUX
#elif defined(__APPLE__)
#define CM5_PLATFORM_MACOS
#endif

#ifndef NDEBUG
#define CM5_DEBUG
#else
#define CM5_RELEASE
#endif