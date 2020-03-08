/**
 * @file
 * @author mkkulagowski (mkulagowski(at)gmail.com)
 * @brief Implementation of test library for LibraryTest
 */

#pragma once

#ifdef WIN32
#ifdef TESTCALCDLL_EXPORTS
#define TESTCALCDLL_API __declspec(dllexport)
#else
#define TESTCALCDLL_API __declspec(dllimport)
#endif // TESTCALCDLL_EXPORTS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__LINUX__) | defined(__linux__)
#define TESTCALCDLL_API
#endif // WIN32

extern "C" TESTCALCDLL_API int Add(int num1, int num2);
extern "C" TESTCALCDLL_API int Subtract(int num1, int num2);
