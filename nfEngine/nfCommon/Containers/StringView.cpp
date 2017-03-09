/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  StringView definitions.
 */

#include "PCH.hpp"
#include "StringView.hpp"


namespace NFE {
namespace Common {

const uint32 StringView::END = UINT32_MAX;

uint32 StringView::FindFirst(const StringView& subString) const
{
    if (subString.mLength > mLength || subString.mLength == 0)
        return END;

    // TODO: KMP algorithm

    // naive O(N * M) algorithm
    for (uint32 i = 0; i < mLength - subString.mLength + 1; ++i)
    {
        bool found = true;
        for (uint32 j = 0; j < subString.mLength; ++j)
        {
            if (mData[i + j] != subString[j])
            {
                found = false;
                break;
            }
        }

        if (found)
            return i;
    }

    // not found
    return END;
}

uint32 StringView::FindLast(const StringView& subString) const
{
    if (subString.mLength > mLength || subString.mLength == 0)
        return END;

    // TODO: KMP algorithm

    // naive O(N * M) algorithm
    for (uint32 i = mLength - subString.mLength + 1; i-- > 0; )
    {
        bool found = true;
        for (uint32 j = 0; j < subString.mLength; ++j)
        {
            if (mData[i + j] != subString[j])
            {
                found = false;
                break;
            }
        }

        if (found)
            return i;
    }

    // not found
    return END;
}

uint32 StringView::FindFirst(const char c) const
{
    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] == c)
            return i;
    }

    // not found
    return END;
}

uint32 StringView::FindLast(const char c) const
{
    for (uint32 i = mLength; i-- > 0; )
    {
        if (mData[i] == c)
            return i;
    }

    // not found
    return END;
}


//////////////////////////////////////////////////////////////////////////


bool StringView::operator == (const StringView& other) const
{
    if (mLength != other.mLength)
        return false;

    if (mData == other.mData)
        return true;

    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] != other.mData[i])
            return false;
    }

    return true;
}

bool StringView::operator != (const StringView& other) const
{
    if (mLength != other.mLength)
        return true;

    if (mData == other.mData)
        return false;

    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] != other.mData[i])
            return true;
    }

    return false;
}

bool StringView::operator < (const StringView& other) const
{
    if (mLength < other.mLength)
        return true;

    if (mLength > other.mLength)
        return false;

    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] < other.mData[i])
            return true;
        else if (mData[i] > other.mData[i])
            return false;
    }

    // string are equal
    return false;
}

bool StringView::operator > (const StringView& other) const
{
    if (mLength > other.mLength)
        return true;

    if (mLength < other.mLength)
        return false;

    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] > other.mData[i])
            return true;
        else if (mData[i] < other.mData[i])
            return false;
    }

    // string are equal
    return false;
}

bool StringView::operator <= (const StringView& other) const
{
    if (mLength < other.mLength)
        return true;

    if (mLength > other.mLength)
        return false;

    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] < other.mData[i])
            return true;
        else if (mData[i] > other.mData[i])
            return false;
    }

    // string are equal
    return true;
}

bool StringView::operator >= (const StringView& other) const
{
    if (mLength > other.mLength)
        return true;

    if (mLength < other.mLength)
        return false;

    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] > other.mData[i])
            return true;
        else if (mData[i] < other.mData[i])
            return false;
    }

    // string are equal
    return true;
}


} // namespace Common
} // namespace NFE
