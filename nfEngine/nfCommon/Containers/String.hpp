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
    String();
    String(char c);
    String(const String& other);
    String(const StringView& view);

    // assignment operators
    String& operator=(char c);
    String& operator=(const String& other);
    String& operator=(const StringView& view);

    // append
    String& operator+=(char c);
    String& operator+=(const String& other);
    String& operator+=(const StringView& view);

    // String-String concatenation operators
    friend String operator+(const String& lhs, const String& rhs);
    friend String operator+(String&& lhs, const String& rhs);
    friend String operator+(const String& lhs, String&& rhs);
    friend String operator+(String& lhs, String&& rhs);

    // String-StringView concatenation operators
    friend String operator+(const String& lhs, const StringView& rhs);
    friend String operator+(String&& lhs, const StringView& rhs);
    friend String operator+(const StringView& lhs, const String& rhs);
    friend String operator+(const StringView& lhs, String&& rhs);

    // String-char concatenation operators
    friend String operator+(const String& lhs, char rhs);
    friend String operator+(String&& lhs, char rhs);
    friend String operator+(char lhs, const String& rhs);
    friend String operator+(char lhs, String&& rhs);


    // insert character(s) at location
    String& Insert(uint32 index, const String& other);
    String& Insert(uint32 index, const StringView& other);
    String& Insert(uint32 index, char c);

    /**
     * Remove characters from string.
     * @param   index           Location of the first char to be removed.
     * @param   numCharacters   Number of characters to remove.
     */
    String& Erase(uint32 index, uint32 numCharacters);

    // replace characters (works like combination of Erase and Insert)
    String& Replace(uint32 index, uint32 numCharacters, const String& other);
    String& Replace(uint32 index, uint32 numCharacters, const StringView& other);
    String& Replace(uint32 index, uint32 numCharacters, char c);


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
    struct External
    {
        uint32 length : 31;     // string length (without null termination)
        uint32 isInternal : 1;  // string mode flag
        uint32 allocSize;       // allocated buffer size
        char* data;             // pointer to allocated buffer

        NFE_INLINE External()
            : length(0)
            , isInternal(0)
            , allocSize(0)
            , data(nullptr)
        { }
    };

    struct Internal
    {
        uint32 length : 31;                         // string length (without null termination)
        uint32 isInternal : 1;                      // string mode flag
        char data[NFE_INTERNAL_STRING_LENGTH + 1];  // internal data (+1 for null termination)

        NFE_INLINE Internal();
        NFE_INLINE Internal(char c);
    };

    union
    {
        External mExternalData;
        Internal mInternalData;
    };
};


static_assert(sizeof(String) == 16UL, "Invalid String type size");


// String-String concatenation operators
String operator+(const String& lhs, const String& rhs);
String operator+(String&& lhs, const String& rhs);
String operator+(const String& lhs, String&& rhs);
String operator+(String& lhs, String&& rhs);

// String-StringView concatenation operators
String operator+(const String& lhs, const StringView& rhs);
String operator+(String&& lhs, const StringView& rhs);
String operator+(const StringView& lhs, const String& rhs);
String operator+(const StringView& lhs, String&& rhs);

// String-char concatenation operators
String operator+(const String& lhs, char rhs);
String operator+(String&& lhs, char rhs);
String operator+(char lhs, const String& rhs);
String operator+(char lhs, String&& rhs);


} // namespace Common
} // namespace NFE


#include "StringImpl.hpp"
