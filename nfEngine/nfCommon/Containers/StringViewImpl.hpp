/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  StringView inline functions definitions.
 */

#pragma once

#include "StringView.hpp"
#include "../System/Assertion.hpp"

#include <string.h>


namespace NFE {
namespace Common {

StringView::StringView()
    : mData(nullptr)
    , mLength(0)
{

}

StringView::StringView(const char* string)
    : mData(string)
{
    mLength = static_cast<uint32>(::strlen(string));
    NFE_ASSERT(mLength <= MAX_LENGTH(), "String is too long");
}

StringView::StringView(const char* string, uint32 length)
    : mData(string)
    , mLength(length)
{
    NFE_ASSERT(mLength <= MAX_LENGTH(), "String is too long");

    for (uint32 i = 0; i < length; ++i)
    {
        NFE_ASSERT(string[i] != '\0', "Unexpected null termination");
    }
}

uint32 StringView::Length() const
{
    return mLength;
}

const char* StringView::Data() const
{
    return mData;
}

bool StringView::Empty() const
{
    return mLength == 0;
}

char StringView::operator[](uint32 index) const
{
    NFE_ASSERT(index < mLength, "StringView index out of bounds");
    return mData[index];
}

StringView StringView::Range(uint32 index, uint32 length) const
{
    // empty string view case
    if (Empty() && length == 0)
        return StringView();

    NFE_ASSERT((index < mLength), "StringView index out of bounds");
    NFE_ASSERT((length <= MAX_LENGTH()) && (index + length <= mLength), "StringView subrange is too big");

    StringView result;
    result.mData = mData + index;
    result.mLength = length;
    return result;
}


} // namespace Common
} // namespace NFE
