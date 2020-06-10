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
    // denotes end of the string
    static constexpr uint32 END() { return UINT32_MAX; }

    // maximum string length in bytes - 1GB, due to String implementation
    static constexpr uint32 MAX_LENGTH() { return (1 << 30) - 1; }

    // create empty string view
    NFE_INLINE StringView();

    // create view of C-like, null-terminated string
    NFE_INLINE StringView(const char* string);

    // create view of string with of given length
    // Null-termination is not expected, will cause assertion
    NFE_INLINE StringView(const char* string, uint32 length);

    // copy constructor/assignment
    StringView(const StringView& other) = default;
    StringView& operator = (const StringView& other) = default;

    // create view of String
    StringView(const String& string);
    StringView& operator = (const String& other);

    /**
     * Get length of the string (in bytes).
     */
    NFE_FORCE_INLINE uint32 Length() const
    {
        return mLength;
    }

    /**
     * Get raw data pointed by the view.
     * @note    The string may not be null-terminated!
     */
    NFE_FORCE_INLINE const char* Data() const { return mData; }

    NFE_FORCE_INLINE bool IsNullTerminated() const
    {
        return mIsNullTerminated;
    }

    /**
     * Check if the array is empty.
     */
    NFE_FORCE_INLINE bool Empty() const
    {
        return mLength == 0;
    }

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
    NFE_INLINE const StringView Range(uint32 index, uint32 length) const;

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
     * Find a first character that is not part of another string.
     */
    uint32 FindFirstNotOf(const char other) const;
    uint32 FindFirstNotOf(const StringView& other) const;

    /**
     * Check if the string ends with another string
     */
    bool EndsWith(const StringView& subString) const;

    /**
     * Comparison operators.
     * Lexicographic order is preserved.
     */
    friend NFCOMMON_API bool operator == (const StringView& lhs, const StringView& rhs);
    friend NFCOMMON_API bool operator != (const StringView& lhs, const StringView& rhs);
    friend NFCOMMON_API bool operator < (const StringView& lhs, const StringView& rhs);
    friend NFCOMMON_API bool operator > (const StringView& lhs, const StringView& rhs);
    friend NFCOMMON_API bool operator <= (const StringView& lhs, const StringView& rhs);
    friend NFCOMMON_API bool operator >= (const StringView& lhs, const StringView& rhs);

private:
    // pointer to the string data
    // don't need to be null-terminated
    const char* mData;

    // number of bytes
    uint32 mLength;

    // indicates whether byte after the string is null-terminator
    // if so, it can be safely casted to 'const char*'
    bool mIsNullTerminated;
};


/**
 * A helper class that allows for easy and fast StringView -> const char* (C-style) string conversion
 */
class StringViewToCStringHelper
{
public:
    static constexpr uint32 stackBufferSize = 1024;

    NFCOMMON_API StringViewToCStringHelper(const StringView stringView);

        NFE_FORCE_INLINE const char* Str() const
    {
        return ptr;
    }

    NFE_FORCE_INLINE operator const char* () const
    {
        return ptr;
    }

private:
    const char* ptr = nullptr;
    char buffer[stackBufferSize];
};


NFE_FORCE_INLINE bool operator == (const char* lhs, const StringView& rhs) { return StringView(lhs) == rhs;}
NFE_FORCE_INLINE bool operator != (const char* lhs, const StringView& rhs) { return StringView(lhs) != rhs;}
NFE_FORCE_INLINE bool operator < (const char* lhs, const StringView& rhs) { return StringView(lhs) < rhs;}
NFE_FORCE_INLINE bool operator > (const char* lhs, const StringView& rhs) { return StringView(lhs) > rhs;}
NFE_FORCE_INLINE bool operator <= (const char* lhs, const StringView& rhs) { return StringView(lhs) <= rhs;}
NFE_FORCE_INLINE bool operator >= (const char* lhs, const StringView& rhs) { return StringView(lhs) >= rhs;}


/**
 * Calculate hash of a string view.
 * @note This function is meant to be fast (it's used in hash tables), not to be cryptographically secure.
 */
NFCOMMON_API uint32 GetHash(const StringView& stringView);


} // namespace Common
} // namespace NFE


#include "StringViewImpl.hpp"
