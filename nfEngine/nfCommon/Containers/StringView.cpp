/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  StringView definitions.
 */

#include "PCH.hpp"
#include "StringView.hpp"


namespace NFE {
namespace Common {



uint32 StringView::FindFirst(const StringView& subString) const
{
    UNUSED(subString);
    return END;
}

uint32 StringView::FindLast(const StringView& subString) const
{
    UNUSED(subString);
    return END;
}

uint32 StringView::FindFirst(const char c) const
{
    UNUSED(c);
    return END;
}

uint32 StringView::FindLast(const char c) const
{
    UNUSED(c);
    return END;
}

bool StringView::operator == (const StringView& other) const
{
    if (mLength != other.mLength)
        return false;

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

    for (uint32 i = 0; i < mLength; ++i)
    {
        if (mData[i] != other.mData[i])
            return true;
    }

    return false;
}

bool StringView::operator < (const StringView& other) const
{
    UNUSED(other);
    return false;
}

bool StringView::operator > (const StringView& other) const
{
    UNUSED(other);
    return false;
}

bool StringView::operator <= (const StringView& other) const
{
    UNUSED(other);
    return false;
}

bool StringView::operator >= (const StringView& other) const
{
    UNUSED(other);
    return false;
}

} // namespace Common
} // namespace NFE
