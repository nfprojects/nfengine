/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  String declaration.
 */

#pragma once

#include "StringView.hpp"

namespace NFE {
namespace Common {

/**
 * Dynamic ASCII/UTF-8 string.
 */
class NFCOMMON_API String
{
public:
    // maximum length of a string (without null-terminator) that can be kept in internal storage
    static constexpr uint32 MaxInternalLength = 11;

    ~String();

    // move operators
    String(String&& other);
    String& operator=(String&& other);

    // constructors
    NFE_INLINE String();
    NFE_INLINE String(char c);
    explicit String(const StringView& view);
    explicit String(const char* string);
    String(const char* str, uint32 length);
    String(const String& string);

    // construct a string from a fixed-sized array
    // array size must be less than MaxInternalLength
    template<uint32 N>
    NFE_INLINE static String ConstructFromFixedArray(const char(& str)[N], const uint32 length = N);

    // produce printf-formated string
    static String Printf(const char* format, ...);

    // assignment operators
    String& operator=(char c);
    String& operator=(const StringView& view);
    String& operator=(const String& string);
    String& operator=(const char* string);

    // append
    String& operator+=(char c);
    String& operator+=(const StringView& view);
    String& operator+=(const String& string);
    String& operator+=(const char* string);

    // concatenation operators
    friend NFCOMMON_API String operator+(const StringView& lhs, const StringView& rhs);
    friend NFCOMMON_API String operator+(String&& lhs, const StringView& rhs);
    friend NFCOMMON_API String operator+(const StringView& lhs, String&& rhs);
    friend NFCOMMON_API String operator+(String&& lhs, String&& rhs);

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

    /**
     * Remove last element.
     * @remarks Using this function on an empty string is forbidden.
     */
    String& PopBack();

    // replace characters (works like combination of Erase and Insert)
    String& Replace(uint32 index, uint32 numCharacters, char c);
    String& Replace(uint32 index, uint32 numCharacters, const StringView& other);
    String& Replace(uint32 index, uint32 numCharacters, const String& string);
    String& Replace(uint32 index, uint32 numCharacters, const char* string);

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

    /**
     * Access first / last element.
     */
    NFE_INLINE char Front() const;
    NFE_INLINE char Back() const;

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
        uint32 length : 31;                 // string length (without null termination)
        uint32 isExternal : 1;              // string mode flag
        char data[MaxInternalLength + 1];   // internal data (+1 for null termination)

        NFE_INLINE InternalData();
        NFE_INLINE InternalData(char c);
    };

    static_assert(sizeof(InternalData) == 16UL, "Invalid InternalData type size");

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


// concatenation operators (const StringView& and String&&)
NFCOMMON_API String operator + (const StringView& lhs, const StringView& rhs);
NFCOMMON_API String operator + (String&& lhs, const StringView& rhs);
NFCOMMON_API String operator + (const StringView& lhs, String&& rhs);
NFCOMMON_API String operator + (String&& lhs, String&& rhs);

NFE_FORCE_INLINE bool operator == (const String& lhs, const StringView& rhs) { return lhs.ToView() == rhs;}
NFE_FORCE_INLINE bool operator != (const String& lhs, const StringView& rhs) { return lhs.ToView() != rhs;}
NFE_FORCE_INLINE bool operator < (const String& lhs, const StringView& rhs) { return lhs.ToView() < rhs;}
NFE_FORCE_INLINE bool operator > (const String& lhs, const StringView& rhs) { return lhs.ToView() > rhs;}
NFE_FORCE_INLINE bool operator <= (const String& lhs, const StringView& rhs) { return lhs.ToView() <= rhs;}
NFE_FORCE_INLINE bool operator >= (const String& lhs, const StringView& rhs) { return lhs.ToView() >= rhs;}

// hashing function for String class
NFCOMMON_API uint32 GetHash(const String& string);

} // namespace Common
} // namespace NFE


#include "StringImpl.hpp"
