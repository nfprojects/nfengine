/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  BitMask class definition.
 */

#pragma once

#include "../nfCommon.hpp"
#include "UniquePtr.hpp"


namespace NFE {
namespace Common {


/**
 * BitMask container - like std::bitset, but dynamic.
 */
class NFCOMMON_API BitMask
{
public:
    BitMask() : mSize(0) { }
    BitMask(const BitMask& other);
    BitMask(BitMask&& other);
    BitMask& operator = (const BitMask& other);
    BitMask& operator = (BitMask&& other);

    /**
     * Resize the mask.
     * @param size Bit mask size in bits.
     */
    bool Resize(size_t size, bool preserveValues = true);

    /**
     * Get mask size in bits.
     */
    NFE_INLINE size_t Size() const { return mSize; }

    /**
     * Access bit value.
     * @note    'index' must be valid (in bit range).
     */
    bool operator[](size_t index) const;

    /**
     * Returns true if all bits are set.
     */
    bool All() const;

    /**
     * Returns true if any bit is set.
     */
    bool Any() const;

    /**
     * Returns true if none of the bits is set.
     */
    bool None() const;

    /**
     * Count set bits.
     */
    size_t Count() const;


    /**
     * Set a single bit state.
     * @note    'index' must be valid (in bit range).
     */
    BitMask& Set(size_t index, bool value);

    /**
     * Set bits state.
     * @param   mask    Bits to set.
     * @note    'index' must be valid (in bit range).
     */
    BitMask& Set(const BitMask& mask, bool value);

    /**
     * Set all bits to a specific value.
     */
    BitMask& Set(bool value);

    /**
     * Toggle a single bit state (perform logical negation).
     * @note    'index' must be valid (in bit range).
     */
    BitMask& Toggle(size_t index);

    /**
     * Toggle all bits state (perform logical negation).
     */
    BitMask& Toggle();

    /**
     * Perform bitwise operation with another mask.
     * @note    If bitmasks sizes do not match, the operation is performed on
     *          Min(Size(), other.Size()) number of bits.
     */
    BitMask& operator &= (const BitMask& other);
    BitMask& operator |= (const BitMask& other);
    BitMask& operator ^= (const BitMask& other);

    BitMask operator~() const;

    /**
     * Compare two bit masks.
     */
    bool operator == (const BitMask& other);
    bool operator != (const BitMask& other);

private:

    NFE_INLINE size_t GetNumOfWords() const
    {
        return (mSize + (8 * sizeof(size_t) - 1)) / sizeof(size_t);
    }

    NFE_INLINE size_t GetNumBitsInLastWord() const
    {
        return mSize % (8 * sizeof(size_t));
    }

    NFE_INLINE size_t GetLastWordMask() const
    {
        return (static_cast<size_t>(1) << GetNumBitsInLastWord()) - 1;
    }

    UniquePtr<size_t[]> mWords;

    // size in BITS
    size_t mSize;
};

} // namespace Common
} // namespace NFE
