/**
 * @file
 * @author Witek902
 * @brief  BufferView class definition.
 */

#include "PCH.hpp"
#include "BufferView.hpp"

namespace NFE {
namespace Common {

bool operator == (const BufferView& lhs, const BufferView& rhs)
{
    if (lhs.mSize != rhs.mSize)
    {
        return false;
    }

    if (lhs.mData == rhs.mData)
    {
        return true;
    }

    return 0 == memcmp(lhs.mData, rhs.mData, lhs.mSize);
}

bool operator != (const BufferView& lhs, const BufferView& rhs)
{
    if (lhs.mSize != rhs.mSize)
    {
        return true;
    }

    if (lhs.mData == rhs.mData)
    {
        return false;
    }

    return 0 != memcmp(lhs.mData, rhs.mData, lhs.mSize);
}

} // namespace Common
} // namespace NFE
