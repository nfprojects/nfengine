/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  Precompiled header
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <fstream>

#if defined(NFE_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(NFE_PLATFORM_WINDOWS)

#include "gtest/gtest.h"
