/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  String declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "StringView.hpp"


#define NFE_INTERNAL_STRING_LENGTH 11

namespace NFE {
namespace Common {

/**
 * Dynamic ASCII/UTF-8 string.
 */
class NFCOMMON_API String
{
public:
    ~String();

    // move operators
    String(String&& other);
    String& operator=(String&& other);

    // constructors
    NFE_INLINE String();
    NFE_INLINE String(char c);
    String(const StringView& view);
    NFE_INLINE String(const String& string);
    NFE_INLINE String(const char* string);

    // assignment operators
    String& operator=(char c);
    String& operator=(const StringView& view);
    NFE_INLINE String& operator=(const String& string);
    NFE_INLINE String& operator=(const char* string);

    // append
    String& operator+=(char c);
    String& operator+=(const StringView& view);
    NFE_INLINE String& operator+=(const String& string);
    NFE_INLINE String& operator+=(const char* string);

    // base concatenation operators
    friend NFCOMMON_API String operator+(const StringView& lhs, const StringView& rhs);
    friend NFCOMMON_API String operator+(String&& lhs, const StringView& rhs);
    friend NFCOMMON_API String operator+(const StringView& lhs, String&& rhs);
    friend NFCOMMON_API String operator+(String&& lhs, String&& rhs);
    friend NFCOMMON_API String operator+(const StringView& lhs, char rhs);
    friend NFCOMMON_API String operator+(String&& lhs, char rhs);
    friend NFCOMMON_API String operator+(char lhs, const StringView& rhs);
    friend NFCOMMON_API String operator+(char lhs, String&& rhs);

    /**
     * Clear the content (will free memory if allocated).
     */
    void Clear();

    // insert character(s) at location
    String& Insert(uint32 index, const StringView& other);
    String& Insert(uint32 index, char c);
    NFE_INLINE String& Insert(uint32 index, const String& string);
    NFE_INLINE String& Insert(uint32 index, const char* string);

    /**
     * Remove characters from string.
     * @param   index           Location of the first char to be removed.
     *                          If it's out of range, assertion will be called.
     * @param   numCharacters   Number of characters to remove.
     *                          If (index + numCharacters) exceeds string length, as many characters as possible
     *                          will be removed.
     */
    String& Erase(uint32 index, uint32 numCharacters);

    // replace characters (works like combination of Erase and Insert)
    String& Replace(uint32 index, uint32 numCharacters, char c);
    String& Replace(uint32 index, uint32 numCharacters, const StringView& other);
    NFE_INLINE String& Replace(uint32 index, uint32 numCharacters, const String& string);
    NFE_INLINE String& Replace(uint32 index, uint32 numCharacters, const char* string);

    /**
     * Reserve memory for given string length (in characters, without null-termination).
     */
    bool Reserve(uint32 length);

    /**
     * Get a view of this string.
     */
    StringView ToView() const;

    /**
     * Get length of the string (in bytes).
     */
    NFE_INLINE uint32 Length() const;

    /**
     * Get current string buffer capacity (in bytes, so includes null-termination).
     */
    NFE_INLINE uint32 Capacity() const;

    /**
     * Get C-style string.
     * @note    The string WILL BE null-terminated.
     */
    NFE_INLINE const char* Str() const;

    /**
     * Get read-write buffer.
     * @note    Use with care.
     */
    NFE_INLINE char* GetBuffer();

    /**
     * Check if the array is empty.
     */
    NFE_INLINE bool Empty() const;

    /**
     * Is the string data allocated directly in the String object?
     */
    NFE_INLINE bool IsInternal() const;

    /**
     * Character access operator.
     * @note The index must be valid. Otherwise it will cause an assertion.
     */
    NFE_INLINE char operator[](uint32 index) const;

    /**
     * Character access operator.
     * @note The index must be valid. Otherwise it will cause an assertion.
     */
    NFE_INLINE char& operator[](uint32 index);

private:

    // string buffer is stored in external memory block
    struct ExternalData
    {
        uint32 length : 31;     // string length (without null termination)
        uint32 isExternal : 1;  // string mode flag
        uint32 allocSize;       // allocated buffer size
        char* data;             // pointer to allocated buffer

        NFE_INLINE ExternalData();
    };

    // string buffer is hold inside the String object itself (up to 12 characters, including null-termination)
    struct InternalData
    {
        uint32 length : 31;                         // string length (without null termination)
        uint32 isExternal : 1;                      // string mode flag
        char data[NFE_INTERNAL_STRING_LENGTH + 1];  // internal data (+1 for null termination)

        NFE_INLINE InternalData();
        NFE_INLINE InternalData(char c);
    };

    // for easy move
    struct PackedData
    {
        uint64 a;
        uint64 b;
    };

    union
    {
        ExternalData mExternalData;
        InternalData mInternalData;
        PackedData   mPackedData;
    };

    NFE_INLINE void SetLength(uint32 length);
};


static_assert(sizeof(String) == 16UL, "Invalid String type size");


// base concatenation operators (const StringView& and String&&)
NFCOMMON_API String operator+(const StringView& lhs, const StringView& rhs);
NFCOMMON_API String operator+(String&& lhs, const StringView& rhs);
NFCOMMON_API String operator+(const StringView& lhs, String&& rhs);
NFCOMMON_API String operator+(String&& lhs, String&& rhs);
NFCOMMON_API String operator+(const StringView& lhs, char rhs);
NFCOMMON_API String operator+(String&& lhs, char rhs);
NFCOMMON_API String operator+(char lhs, const StringView& rhs);
NFCOMMON_API String operator+(char lhs, String&& rhs);

// other concatenation operators (const String&)
NFE_INLINE String operator+(const String& lhs, const String& rhs);
NFE_INLINE String operator+(String&& lhs, const String& rhs);
NFE_INLINE String operator+(const String& lhs, String&& rhs);
NFE_INLINE String operator+(const String& lhs, char rhs);
NFE_INLINE String operator+(char lhs, const String& rhs);

// other concatenation operators (const char*)
NFE_INLINE String operator+(const String& lhs, const char* rhs);
NFE_INLINE String operator+(const char* lhs, const String& rhs);
NFE_INLINE String operator+(String&& lhs, const char* rhs);
NFE_INLINE String operator+(const char* lhs, String&& rhs);


} // namespace Common
} // namespace NFE


#include "StringImpl.hpp"
