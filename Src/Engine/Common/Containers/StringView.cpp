/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  StringView definitions.
 */

#include "PCH.hpp"
#include "StringView.hpp"
#include "String.hpp"


namespace NFE {
namespace Common {

StringView::StringView(const String& string)
    : mData(string.Str())
    , mLength(string.Length())
    , mIsNullTerminated(true)
{
}

StringView& StringView::operator = (const String& other)
{
    mData = other.Str();
    mLength = other.Length();
    mIsNullTerminated = true;
    return *this;
}

uint32 StringView::FindFirst(const StringView& subString) const
{
    if (subString.mLength > mLength || subString.mLength == 0)
    {
        return END();
    }

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
    return END();
}

uint32 StringView::FindLast(const StringView& subString) const
{
    if (subString.mLength > mLength || subString.mLength == 0)
    {
        return END();
    }

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
    return END();
}

uint32 StringView::FindFirst(const char c) const
{
    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] == c)
        {
            return i;
        }
    }

    // not found
    return END();
}

uint32 StringView::FindLast(const char c) const
{
    for (uint32 i = mLength; i-- > 0; )
    {
        if (mData[i] == c)
        {
            return i;
        }
    }

    // not found
    return END();
}

uint32 StringView::FindFirstNotOf(const char other) const
{
    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] != other)
        {
            return i;
        }
    }

    return END();
}

uint32 StringView::FindFirstNotOf(const StringView& other) const
{
    for (uint32 i = 0; i < mLength; ++i)
    {
        bool found = false;
        for (uint32 j = 0; j < other.Length(); ++j)
        {
            if (mData[i] == other.mData[j])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return i;
        }
    }

    return END();
}

bool StringView::EndsWith(const StringView& subString) const
{
    if (subString.mLength > mLength || subString.mLength == 0)
    {
        return false;
    }

    for (uint32 j = 0; j < subString.mLength; ++j)
    {
        if (mData[mLength - subString.mLength + j] != subString[j])
        {
            return false;
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////

bool operator == (const StringView& lhs, const StringView& rhs)
{
    if (lhs.mLength != rhs.mLength)
    {
        return false;
    }

    if (lhs.mData == rhs.mData)
    {
        return true;
    }

    return 0 == memcmp(lhs.mData, rhs.mData, lhs.mLength);
}

bool operator != (const StringView& lhs, const StringView& rhs)
{
    if (lhs.mLength != rhs.mLength)
    {
        return true;
    }

    if (lhs.mData == rhs.mData)
    {
        return false;
    }

    return 0 != memcmp(lhs.mData, rhs.mData, lhs.mLength);
}

bool operator < (const StringView& lhs, const StringView& rhs)
{
    if (lhs.mLength < rhs.mLength)
    {
        return true;
    }

    if (lhs.mLength > rhs.mLength)
    {
        return false;
    }

    for (uint32 i = 0; i < lhs.mLength; ++i)
    {
        if (lhs.mData[i] < rhs.mData[i])
        {
            return true;
        }
        else if (lhs.mData[i] > rhs.mData[i])
        {
            return false;
        }
    }

    // string are equal
    return false;
}

bool operator > (const StringView& lhs, const StringView& rhs)
{
    return rhs < lhs;
}

bool operator <= (const StringView& lhs, const StringView& rhs)
{
    if (lhs.mLength < rhs.mLength)
    {
        return true;
    }

    if (lhs.mLength > rhs.mLength)
    {
        return false;
    }

    for (uint32 i = 0; i < lhs.mLength; ++i)
    {
        if (lhs.mData[i] < rhs.mData[i])
        {
            return true;
        }
        else if (lhs.mData[i] > rhs.mData[i])
        {
            return false;
        }
    }

    // string are equal
    return true;
}

bool operator >= (const StringView& lhs, const StringView& rhs)
{
    return rhs <= lhs;
}

StringViewToCStringHelper::StringViewToCStringHelper(const StringView stringView)
{
    if (stringView.IsNullTerminated())
    {
        ptr = stringView.Data();
    }
    else if (stringView.Length() < stackBufferSize)
    {
        memcpy(buffer, stringView.Data(), stringView.Length());
        buffer[stringView.Length()] = '\0';
        ptr = buffer;
    }
    else
    {
        // not implemented yet...
        NFE_FATAL("String is too long");
    }
}

uint32 GetHash(const StringView& stringView)
{
    const uint32 length = stringView.Length();

    // djb2 hash function
    // http://www.cse.yorku.ca/~oz/hash.html

    uint32 hash = 5381u;
    for (uint32 i = 0; i < length; ++i)
    {
        hash = hash * 33u + static_cast<uint32>(stringView[i]);
    }

    return hash;
}


} // namespace Common
} // namespace NFE
