/**
 * @file   stdafx.hpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Precompiled header
 */

#pragma once

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <tchar.h>

#define NOMINMAX
#include <Windows.h>
#endif // WIN32

#include <stdlib.h>
#include <stdio.h>

#include "gtest/gtest.h"
