/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  StringView inline functions definitions.
 */

#pragma once

#include "String.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Common {


//////////////////////////////////////////////////////////////////////////

String::ExternalData::ExternalData()
    : length(0)
    , isExternal(1)
    , allocSize(0)
    , data(nullptr)
{ }

String::InternalData::InternalData()
    : length(0)
    , isExternal(0)
    , data{ '\0' }
{ }

String::InternalData::InternalData(char c)
    : length(1)
    , isExternal(0)
    , data{ c, '\0' }
{ }

//////////////////////////////////////////////////////////////////////////

String::String()
    : mInternalData()
{ }

String::String(char c)
    : mInternalData(c)
{ }

String::String(const String& string)
    : String(string.ToView())
{ }

String::String(const char* string)
    : String(StringView(string))
{ }

String::String(const char* string, uint32 length)
    : String(StringView(string, length))
{ }

String& String::operator=(const String& other)
{
    *this = other.ToView();
    return *this;
}

String& String::operator=(const char* string)
{
    *this = StringView(string);
    return *this;
}

String& String::operator+=(const String& string)
{
    *this += string.ToView();
    return *this;
}

String& String::operator+=(const char* string)
{
    *this += StringView(string);
    return *this;
}

String& String::Insert(uint32 index, const String& string)
{
    return Insert(index, string.ToView());
}

String& String::Insert(uint32 index, const char* string)
{
    return Insert(index, StringView(string));
}

String& String::Replace(uint32 index, uint32 numCharacters, const String& string)
{
    return Replace(index, numCharacters, string.ToView());
}

String& String::Replace(uint32 index, uint32 numCharacters, const char* string)
{
    return Replace(index, numCharacters, StringView(string));
}

uint32 String::Length() const
{
    return mInternalData.length;
}

uint32 String::Capacity() const
{
    if (IsInternal())
        return NFE_INTERNAL_STRING_LENGTH + 1;
    else
        return mExternalData.allocSize;
}

const char* String::Str() const
{
    if (IsInternal())
        return mInternalData.data;
    else
        return mExternalData.data;
}

char* String::GetBuffer()
{
    if (IsInternal())
        return mInternalData.data;
    else
        return mExternalData.data;
}

bool String::Empty() const
{
    return mInternalData.length == 0;
}

bool String::IsInternal() const
{
    return mInternalData.isExternal == 0;
}

char String::operator[](uint32 index) const
{
    NFE_ASSERT(index < Length(), "String index out of bounds");

    if (IsInternal())
        return mInternalData.data[index];
    else
        return mExternalData.data[index];
}

char& String::operator[](uint32 index)
{
    NFE_ASSERT(index < Length(), "String index out of bounds");

    if (IsInternal())
        return mInternalData.data[index];
    else
        return mExternalData.data[index];
}

void String::SetLength(uint32 length)
{
    mInternalData.length = length;
}

String::operator StringView() const
{
    return ToView();
}

//////////////////////////////////////////////////////////////////////////

NFE_INLINE String operator+(const String& lhs, const String& rhs)
{
    return lhs.ToView() + rhs.ToView();
}

NFE_INLINE String operator+(String&& lhs, const String& rhs)
{
    return std::move(lhs) + rhs.ToView();
}

NFE_INLINE String operator+(const String& lhs, String&& rhs)
{
    return lhs.ToView() + std::move(rhs);
}

NFE_INLINE String operator+(const String& lhs, char rhs)
{
    return lhs.ToView() + rhs;
}

NFE_INLINE String operator+(char lhs, const String& rhs)
{
    return lhs + rhs.ToView();
}

NFE_INLINE String operator+(const String& lhs, const char* rhs)
{
    return lhs.ToView() + StringView(rhs);
}

NFE_INLINE String operator+(const char* lhs, const String& rhs)
{
    return StringView(lhs) + rhs.ToView();
}

NFE_INLINE String operator+(String&& lhs, const char* rhs)
{
    return std::move(lhs) + StringView(rhs);
}

NFE_INLINE String operator+(const char* lhs, String&& rhs)
{
    return StringView(lhs) + std::move(rhs);
}

//////////////////////////////////////////////////////////////////////////

bool operator<(const String& lhs, const String& rhs)
{
    return lhs.ToView() < rhs.ToView();
}

bool operator<=(const String& lhs, const String& rhs)
{
    return lhs.ToView() <= rhs.ToView();
}

bool operator>(const String& lhs, const String& rhs)
{
    return lhs.ToView() > rhs.ToView();
}

bool operator>=(const String& lhs, const String& rhs)
{
    return lhs.ToView() >= rhs.ToView();
}

bool operator==(const String& lhs, const String& rhs)
{
    return lhs.ToView() == rhs.ToView();
}

bool operator!=(const String& lhs, const String& rhs)
{
    return lhs.ToView() != rhs.ToView();
}

//////////////////////////////////////////////////////////////////////////

bool operator<(const String& lhs, const StringView& rhs)
{
    return lhs.ToView() < rhs;
}

bool operator<=(const String& lhs, const StringView& rhs)
{
    return lhs.ToView() <= rhs;
}

bool operator>(const String& lhs, const StringView& rhs)
{
    return lhs.ToView() > rhs;
}

bool operator>=(const String& lhs, const StringView& rhs)
{
    return lhs.ToView() >= rhs;
}

bool operator==(const String& lhs, const StringView& rhs)
{
    return lhs.ToView() == rhs;
}

bool operator!=(const String& lhs, const StringView& rhs)
{
    return lhs.ToView() != rhs;
}

//////////////////////////////////////////////////////////////////////////

bool operator<(const StringView& lhs, const String& rhs)
{
    return lhs < rhs.ToView();
}

bool operator<=(const StringView& lhs, const String& rhs)
{
    return lhs <= rhs.ToView();
}

bool operator>(const StringView& lhs, const String& rhs)
{
    return lhs > rhs.ToView();
}

bool operator>=(const StringView& lhs, const String& rhs)
{
    return lhs >= rhs.ToView();
}

bool operator==(const StringView& lhs, const String& rhs)
{
    return lhs == rhs.ToView();
}

bool operator!=(const StringView& lhs, const String& rhs)
{
    return lhs != rhs.ToView();
}

//////////////////////////////////////////////////////////////////////////

uint32 GetHash(const String& string)
{
    return GetHash(string.ToView());
}

} // namespace Common
} // namespace NFE
