/**
 * @file
 * @brief  String utilities definitions.
 */

#include "PCH.hpp"
#include "StringUtils.hpp"
#include "BitUtils.hpp"
#include "../Containers/String.hpp"


namespace NFE {
namespace Common {


namespace {

// Lookup table for numbers 0-99
const char gDigitsLut[201] =
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899";

const uint32 gPowersOf10_32[] =
{
    0u,
    10u,
    100u,
    1000u,
    10000u,
    100000u,
    1000000u,
    10000000u,
    100000000u,
    1000000000u,
};

const uint64 gPowersOf10_64[] =
{
    0ull,
    10ull,
    100ull,
    1000ull,
    10000ull,
    100000ull,
    1000000ull,
    10000000ull,
    100000000ull,
    1000000000ull,
    10000000000ull,
    100000000000ull,
    1000000000000ull,
    10000000000000ull,
    100000000000000ull,
    1000000000000000ull,
    10000000000000000ull,
    100000000000000000ull,
    1000000000000000000ull,
    10000000000000000000ull,
};

// Calculate number of decimal digits required to represent given value
NFE_INLINE uint32 CountDecimalDigits32(uint32 n)
{
    const uint32 t = (32u - (uint32)BitUtils<uint32>::CountLeadingZeros(n | 1u)) * 1233u >> 12u;
    return 1u + t - (n < gPowersOf10_32[t] ? 1u : 0u);
}

NFE_INLINE uint32 CountDecimalDigits64(uint64 n)
{
    const uint32_t t = (64u - (uint32)BitUtils<uint64>::CountLeadingZeros(n | 1u)) * 1233u >> 12u;
    return 1u + t - (n < gPowersOf10_64[t] ? 1u : 0u);
}

template<typename T>
NFE_INLINE void NumToDecimal(T value, char* buffer, uint32 offset)
{
    // implementation based on "countlut" algorithm used here: https://github.com/miloyip/itoa-benchmark

    while (value >= T(100))
    {
        const uint32 i = (value % T(100)) << 1u;
        value /= T(100);
        buffer[--offset] = gDigitsLut[i + 1];
        buffer[--offset] = gDigitsLut[i];
    }

    if (value < 10ull)
    {
        buffer[--offset] = char(value) + '0';
    }
    else
    {
        const uint32 i = static_cast<uint32>(value << 1);
        buffer[--offset] = gDigitsLut[i + 1];
        buffer[--offset] = gDigitsLut[i];
    }
}

} // namespace


String ToString(bool value)
{
    const StringView trueString("true");
    const StringView falseString("false");

    return String(value ? trueString : falseString);
}

String ToString(uint32 value)
{
    char buffer[10]; // max uint32 requires 10 characters

    const uint32 len = CountDecimalDigits32(value);
    NumToDecimal(value, buffer, len);
    return String::ConstructFromFixedArray(buffer, len);
}

String ToString(uint64 value)
{
    char buffer[21]; // max uint64 requires 21 characters

    const uint32 len = CountDecimalDigits64(value);
    NumToDecimal(value, buffer, len);
    return String(buffer, len);
}

String ToString(int32 value)
{
    uint32 len = 0;
    char buffer[11]; // min int32 requires 11 characters

    // convert to unsigned
    uint32 unsignedValue = static_cast<uint32>(value);
    if (value < 0)
    {
        len++;
        buffer[0] = '-';
        unsignedValue = ~unsignedValue + 1;
    }

    len += CountDecimalDigits32(unsignedValue);
    NumToDecimal(unsignedValue, buffer, len);
    return String::ConstructFromFixedArray(buffer, len);
}

String ToString(int64 value)
{
    uint32 len = 0;
    char buffer[21]; // min int64 requires 11 characters

    // convert to unsigned
    uint64 unsignedValue = static_cast<uint64>(value);
    if (value < 0)
    {
        len++;
        buffer[0] = '-';
        unsignedValue = ~unsignedValue + 1;
    }

    len += CountDecimalDigits64(unsignedValue);
    NumToDecimal(unsignedValue, buffer, len);
    return String(buffer, len);
}

String ToString(float value)
{
    char buffer[64] = { '\0' };
    snprintf(buffer, 64, "%g", value);
    return String(buffer);
}

String ToString(double value)
{
    char buffer[64] = { '\0' };
    snprintf(buffer, 64, "%g", value);
    return String(buffer);
}

bool FromString(const StringView& string, int32& ret)
{
    int32 r = 0;
    int8 num = 0;

    uint32 i = 0;
    if (string[0] == '-')
        ++i;

    for (; i < string.Length(); ++i)
    {
        num = string[i] - '0';
        if (num < 0 || num > 9)
            return false;
        r = r * 10 + num;
    }

    if (string[0] == '-')
        r *= -1;

    ret = r;
    return true;
}

bool FromString(const StringView& string, uint32& ret)
{
    int32 r = 0;
    int8 num = 0;

    for (uint32 i = 0; i < string.Length(); ++i)
    {
        num = string[i] - '0';
        if (num < 0 || num > 9)
            return false;
        r = r * 10 + num;
    }

    ret = r;
    return true;
}

String BytesToString(uint32 size, bool withSpace)
{
    String str;

    if (size < 1024u)
    {
        str = ToString(size);
        if (withSpace) str += ' ';
        str += "bytes";
    }
    else if (size < (1024u << 10u))
    {
        str = String::Printf("%.3f", size / static_cast<float>(1024u));
        if (withSpace) str += ' ';
        str += "KB";
    }
    else if (size < (1024u << 20u))
    {
        str = String::Printf("%.3f", size / static_cast<float>(1024u << 10u));
        if (withSpace) str += ' ';
        str += "MB";
    }
    else
    {
        str = String::Printf("%.3f", size / static_cast<float>(1024u << 20u));
        if (withSpace) str += ' ';
        str += "GB";
    }

    return str;
}

String BytesToString(uint64 size, bool withSpace)
{
    String str;

    if (size <= UINT32_MAX)
    {
        str = BytesToString(static_cast<uint32>(size), withSpace);
    }
    else if (size < (1024ull << 40u))
    {
        str = String::Printf("%.3f", size / static_cast<float>(1024ull << 30u));
        if (withSpace) str += ' ';
        str += "TB";
    }
    else
    {
        str = String::Printf("%.3f", size / static_cast<float>(1024ull << 40u));
        if (withSpace) str += ' ';
        str += "PB";
    }

    return str;
}

DynArray<StringView> Split(const StringView& a, char delim)
{
    DynArray<StringView> result;

    const char* buffer = a.Data();
    uint32 lastDelimPos = 0;
    uint32 i;
    for (i = 0; i < a.Length(); ++i)
    {
        if (buffer[i] == delim)
        {
            if (i != lastDelimPos)
            {
                result.EmplaceBack(buffer + lastDelimPos, i - lastDelimPos);
            }

            lastDelimPos = i + 1;
        }
    }

    if (i != lastDelimPos)
    {
        result.EmplaceBack(buffer + lastDelimPos, i - lastDelimPos);
    }

    return result;
}

} // namespace Common
} // namespace NFE
