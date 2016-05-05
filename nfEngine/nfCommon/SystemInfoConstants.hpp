/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Constants for SystemInfo class
 */

#pragma once

const unsigned int EXTENDED_ID_MAX_VALUE = 0x80000000;
const unsigned int CPU_BRAND_STRING_LENGTH = 0x40;
const unsigned int CPU_BRAND_STRING_1 = 0x80000002;
const unsigned int CPU_BRAND_STRING_2 = 0x80000003;
const unsigned int CPU_BRAND_STRING_3 = 0x80000004;
const unsigned int CPU_CACHE_LINE_SIZE = 0x80000006;

const unsigned int CPUID_FEATURES_1_2 = 0x1;
const unsigned int CPUID_FEATURES_3 = 0x7;
const unsigned int CPUID_FEATURES_4_5 = 0x80000001;