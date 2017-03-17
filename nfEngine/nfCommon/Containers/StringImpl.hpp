/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  StringView inline functions definitions.
 */

#pragma once

#include "String.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Common {


//////////////////////////////////////////////////////////////////////////

String::Internal::Internal()
    : length(0)
    , isInternal(1)
    , data{ '\0' }
{ }

String::Internal::Internal(char c)
    : length(1)
    , isInternal(1)
    , data{ c, '\0' }
{ }

//////////////////////////////////////////////////////////////////////////

uint32 String::Length() const
{
    return mInternalData.length;
}

uint32 String::Capacity() const
{
    if (IsInternal())
        return NFE_INTERNAL_STRING_LENGTH + 1;
    else
        return mExternalData.allocSize;
}

const char* String::Str() const
{
    if (IsInternal())
        return mInternalData.data;
    else
        return mExternalData.data;
}

char* String::GetBuffer()
{
    if (IsInternal())
        return mInternalData.data;
    else
        return mExternalData.data;
}

bool String::Empty() const
{
    return mInternalData.length == 0;
}

bool String::IsInternal() const
{
    return mInternalData.isInternal != 0;
}

char String::operator[](uint32 index) const
{
    NFE_ASSERT(index < Length(), "String index out of bounds");
    
    if (IsInternal())
        return mInternalData.data[index];
    else
        return mExternalData.data[index];
}

char& String::operator[](uint32 index)
{
    NFE_ASSERT(index < Length(), "String index out of bounds");

    if (IsInternal())
        return mInternalData.data[index];
    else
        return mExternalData.data[index];
}

} // namespace Common
} // namespace NFE
