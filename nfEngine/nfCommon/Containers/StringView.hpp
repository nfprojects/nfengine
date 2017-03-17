/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  StringView declaration.
 */

#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * A view of a const ASCII/UTF-8 string.
 */
class NFCOMMON_API StringView
{
    friend class String;

public:
    static const uint32 END = UINT32_MAX;

    // maximum string length in bytes - 1GB, due to String implementation
    static const uint32 MAX_LENGTH = (1 << 30) - 1;


    // create empty string view
    NFE_INLINE StringView();

    // create view of C-like, null-terminated string
    NFE_INLINE explicit StringView(const char* string);

    // create view of string with of given length
    // Null-termination is not expected, will cause assertion
    NFE_INLINE explicit StringView(const char* string, uint32 length);

    // copy/move constructor/assignment
    StringView(const StringView& other) = default;
    StringView& operator = (const StringView& other) = default;
    StringView(StringView&& other) = default;
    StringView& operator = (StringView&& other) = default;


    /**
     * Get length of the string (in bytes).
     */
    NFE_INLINE uint32 Length() const;

    /**
     * Get raw data pointed by the view.
     * @note    The string may not be null-terminated!
     */
    NFE_INLINE const char* Data() const;

    /**
     * Check if the array is empty.
     */
    NFE_INLINE bool Empty() const;

    /**
     * Character access operator.
     * @note The index must be valid. Otherwise it will cause an assertion.
     */
    NFE_INLINE char operator[](uint32 index) const;

    /**
     * Create view of a sub-string.
     * @param index Starting index.
     * @param size  Number of elements in range.
     */
    NFE_INLINE StringView Range(uint32 index, uint32 length) const;

    /**
     * Find a substring in the view. Start search from the left side.
     * @return  Index of the first substring character, of END if not found.
     */
    uint32 FindFirst(const StringView& subString) const;

    /**
     * Find a substring in the view. Start search from the right side.
     * @return  Index of the first substring character, of END if not found.
     */
    uint32 FindLast(const StringView& subString) const;

    /**
     * Find a single character in the view. Start search from the left side.
     * @return  Index of the character, of END if not found.
     */
    uint32 FindFirst(const char c) const;

    /**
     * Find a single character in the view. Start search from the right side.
     * @return  Index of the character, of END if not found.
     */
    uint32 FindLast(const char c) const;

    /**
     * Comparison operators.
     * Lexicographic order is preserved.
     */
    bool operator == (const StringView& other) const;
    bool operator != (const StringView& other) const;
    bool operator < (const StringView& other) const;
    bool operator > (const StringView& other) const;
    bool operator <= (const StringView& other) const;
    bool operator >= (const StringView& other) const;

protected:
    // pointer to the string data
    // don't need to be null-terminated
    const char* mData;

    // number of bytes
    uint32 mLength;
};


} // namespace Common
} // namespace NFE


#include "StringViewImpl.hpp"
