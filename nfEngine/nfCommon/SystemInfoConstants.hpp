/**
 * @file   SystemInfoConstants.hpp
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Constants for SystemInfo class
 */

#pragma once

const int EXTENDED_ID_MAX_VALUE = 0x80000000;
const int CPU_BRAND_STRING_LENGTH = 0x40;
const int CPU_BRAND_STRING_1 = 0x80000002;
const int CPU_BRAND_STRING_2 = 0x80000003;
const int CPU_BRAND_STRING_3 = 0x80000004;
const int CPU_CACHE_LINE_SIZE = 0x80000006;

const int CPUID_FEATURES_1_2 = 0x1;
const int CPUID_FEATURES_3 = 0x7;
const int CPUID_FEATURES_4_5 = 0x80000001;