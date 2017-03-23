#pragma once

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif // WIN32

#include <stdlib.h>
#include <stdio.h>

#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>
#include <mutex>
#include <atomic>
#include <iostream>
#include <type_traits>
#include <thread>
#include <limits>
#include <fstream>
#include <functional>
#include <chrono>
#include <condition_variable>

#include "gtest/gtest.h"

// workaround for Windows/Linux mkdir difference
#if defined(__LINUX__) | defined(__linux__)
#define _mkdir(x) mkdir(x, 0777)
#endif // defined(__LINUX__) | defined(__linux__)
