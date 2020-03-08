/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Precompiled header
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <fstream>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // defined(WIN32)

#include "gtest/gtest.h"
