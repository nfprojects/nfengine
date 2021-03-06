/**
 * @file
 * @author Witek902
 * @brief  String definitions.
 */

#include "PCH.hpp"
#include "String.hpp"
#include "StringView.hpp"
#include "DynArray.hpp"



namespace NFE {
namespace Common {

String::~String()
{
    if (!IsInternal())
    {
        NFE_FREE(mExternalData.data);
        mExternalData.data = nullptr;
    }
}

String::String(const char* string)
    : String(StringView(string))
{
}

String::String(String&& other)
{
    mPackedData.a = other.mPackedData.a;
    mPackedData.b = other.mPackedData.b;
    other.mInternalData = InternalData();
}

String& String::operator=(String&& other)
{
    if (!IsInternal())
    {
        NFE_FREE(mExternalData.data);
        mExternalData.data = nullptr;
    }

    mPackedData.a = other.mPackedData.a;
    mPackedData.b = other.mPackedData.b;
    other.mInternalData = InternalData();

    return *this;
}

String::String(const String& string)
    : String(string.ToView())
{
}

String::String(const StringView& view)
    : mInternalData()
{
    const uint32 length = view.Length();
    if (length <= MaxInternalLength)
    {
        mInternalData.isExternal = 0;
    }
    else
    {
        if (!Reserve(length))
            return;
    }

    char* buffer = GetBuffer();
    memcpy(buffer, view.Data(), length);
    buffer[length] = '\0';

    SetLength(length);
}

String::String(const char* str, uint32 length)
    : mInternalData()
{
    if (length <= MaxInternalLength)
    {
        mInternalData.isExternal = 0;
    }
    else
    {
        if (!Reserve(length))
            return;
    }

    char* buffer = GetBuffer();
    memcpy(buffer, str, length);
    buffer[length] = '\0';

    SetLength(length);
}

String& String::operator=(char c)
{
    if (!Reserve(1))
    {
        return *this;
    }

    char* buffer = GetBuffer();
    buffer[0] = c;
    buffer[1] = '\0';

    SetLength(1);
    return *this;
}

String& String::operator=(const char* string)
{
    *this = StringView(string);
    return *this;
}

String& String::operator=(const String& other)
{
    *this = other.ToView();
    return *this;
}

String& String::operator=(const StringView& view)
{
    if (!Reserve(view.Length()))
    {
        return *this;
    }

    char* buffer = GetBuffer();
    memcpy(buffer, view.Data(), view.Length());
    buffer[view.Length()] = '\0';
    SetLength(view.Length());
    return *this;
}

void String::Clear()
{
    if (!IsInternal())
    {
        NFE_FREE(mExternalData.data);
    }

    mInternalData = InternalData();
}

String String::Printf(const char* format, ...)
{
    NFE_ASSERT(format, "Format string cannot be nullptr");

    const int shortStringLength = 1024;
    char stackBuffer[shortStringLength];

    DynArray<char> buffer;
    char* formattedStr = nullptr;
    va_list args, argsCopy;
    va_start(args, format);

    // we can't call vsnprintf with the same va_list more than once
    va_copy(argsCopy, args);

    int len = vsnprintf(stackBuffer, shortStringLength, format, args);
    NFE_ASSERT(len >= 0, "Invalid format string");

    if (len < 0)
    {
        va_end(argsCopy);
        va_end(args);
        return String();
    }

    if (len >= shortStringLength)  // buffer on the stack is too small
    {
        if (buffer.Resize(len + 1))
        {
            formattedStr = buffer.Data();
            vsnprintf(formattedStr, len + 1, format, argsCopy);
        }
    }
    else if (len > 0)  // buffer on the stack is sufficient
    {
        formattedStr = stackBuffer;
    }

    va_end(argsCopy);
    va_end(args);

    return String(formattedStr);
}

//////////////////////////////////////////////////////////////////////////

String& String::operator+=(char c)
{
    uint32 length = Length();
    if (!Reserve(length + 1))
    {
        return *this;
    }

    char* buffer = GetBuffer();
    buffer[length] = c;

    length++;
    buffer[length] = '\0';
    SetLength(length);
    return *this;
}

String& String::operator+=(const char* string)
{
    *this += StringView(string);
    return *this;
}

String& String::operator+=(const String& string)
{
    *this += string.ToView();
    return *this;
}

String& String::operator+=(const StringView& view)
{
    if (view.Empty())
    {
        return *this;
    }

    uint32 length = Length();
    if (!Reserve(length + view.Length()))
    {
        return *this;
    }

    char* buffer = GetBuffer();
    memcpy(buffer + length, view.Data(), view.Length());

    length += view.Length();
    buffer[length] = '\0';
    SetLength(length);
    return *this;
}

//////////////////////////////////////////////////////////////////////////

String& String::Insert(uint32 index, const char* string)
{
    return Insert(index, StringView(string));
}

String& String::Insert(uint32 index, const String& string)
{
    return Insert(index, string.ToView());
}

String& String::Insert(uint32 index, const StringView& other)
{
    NFE_ASSERT(index <= Length(), "String index out of bounds");

    const uint32 targetLength = Length() + other.Length();
    if (!Reserve(targetLength))
    {
        return *this;
    }

    char* buffer = GetBuffer();
    memmove(buffer + index + other.Length(), buffer + index, Length() - index);
    memcpy(buffer + index, other.Data(), other.Length());
    buffer[targetLength] = '\0';
    SetLength(targetLength);

    return *this;
}

String& String::Insert(uint32 index, char c)
{
    NFE_ASSERT(index <= Length(), "String index out of bounds");

    const uint32 targetLength = Length() + 1;
    if (!Reserve(targetLength))
    {
        return *this;
    }

    char* buffer = GetBuffer();
    memmove(buffer + index + 1, buffer + index, Length() - index);
    buffer[index] = c;
    buffer[targetLength] = '\0';
    SetLength(targetLength);

    return *this;
}

String& String::Erase(uint32 index, uint32 numCharacters)
{
    NFE_ASSERT(index <= Length(), "String index out of bounds");

    if (index + numCharacters > Length())
    {
        numCharacters = Length() - index;
    }

    if (numCharacters == 0)
    {
        // nothing to do
        return *this;
    }

    const uint32 targetLength = Length() - numCharacters;

    char* buffer = GetBuffer();
    memmove(buffer + index, buffer + index + numCharacters, Length() - (index + numCharacters));
    buffer[targetLength] = '\0';
    SetLength(targetLength);

    return *this;
}

String& String::PopBack()
{
    const uint32 len = Length();
    NFE_ASSERT(len > 0, "String is empty");

    const uint32 targetLength = len - 1;

    char* buffer = GetBuffer();
    buffer[targetLength] = '\0';
    SetLength(targetLength);

    return *this;
}

String& String::Replace(uint32 index, uint32 numCharacters, const char* string)
{
    return Replace(index, numCharacters, StringView(string));
}

String& String::Replace(uint32 index, uint32 numCharacters, const String& string)
{
    return Replace(index, numCharacters, string.ToView());
}

String& String::Replace(uint32 index, uint32 numCharacters, const StringView& other)
{
    NFE_ASSERT(index <= Length(), "String index out of bounds");

    if (index + numCharacters > Length())
    {
        numCharacters = Length() - index;
    }

    const uint32 targetLength = Length() - numCharacters + other.Length();

    char* buffer = GetBuffer();
    memmove(buffer + index + other.Length(), buffer + index + numCharacters, Length() - (index + numCharacters));
    memcpy(buffer + index, other.Data(), other.Length());
    buffer[targetLength] = '\0';
    SetLength(targetLength);

    return *this;
}

String& String::Replace(uint32 index, uint32 numCharacters, char c)
{
    NFE_ASSERT(index <= Length(), "String index out of bounds");

    if (index + numCharacters > Length())
    {
        numCharacters = Length() - index;
    }

    const uint32 targetLength = Length() - numCharacters + 1;

    char* buffer = GetBuffer();
    memmove(buffer + index + 1, buffer + index + numCharacters, Length() - (index + numCharacters));
    buffer[index] = c;
    buffer[targetLength] = '\0';
    SetLength(targetLength);

    return *this;
}

StringView String::ToView() const
{
    StringView view;
    view.mLength = Length();
    view.mData = IsInternal() ? mInternalData.data : mExternalData.data;
    view.mIsNullTerminated = true;
    return view;
}

bool String::Reserve(uint32 length)
{
    if (length <= MaxInternalLength)
    {
        // string will fit internal data - nothing to do
        return true;
    }

    // include null-termination
    length++;

    if (length <= Capacity())
    {
        // string will fit current buffer - nothing to do
        return true;
    }

    const uint32 MIN_EXTERNAL_BUFFER_SIZE = 16;

    // determine new buffer size
    uint32 size = MIN_EXTERNAL_BUFFER_SIZE;
    while (size < length)
        size <<= 1;

    char* newBuffer = static_cast<char*>(NFE_MALLOC(size, 1));
    if (!newBuffer)
    {
        return false;
    }

    // copy old string
    memcpy(newBuffer, Str(), Length() + 1);

    if (IsInternal())
    {
        // promote to external
        mExternalData.isExternal = 1;
    }
    else
    {
        // free old buffer
        NFE_FREE(mExternalData.data);
    }

    // update external buffer state
    mExternalData.data = newBuffer;
    mExternalData.allocSize = size;

    return true;
}

//////////////////////////////////////////////////////////////////////////

String operator+(const StringView& lhs, const StringView& rhs)
{
    String result;
    const uint32 targetLength = lhs.Length() + rhs.Length();
    if (!result.Reserve(targetLength))
    {
        return result;
    }

    char* buffer = result.GetBuffer();
    memcpy(buffer, lhs.Data(), lhs.Length());
    memcpy(buffer + lhs.Length(), rhs.Data(), rhs.Length());
    buffer[targetLength] = '\0';
    result.SetLength(targetLength);
    return result;
}

String operator+(String&& lhs, const StringView& rhs)
{
    if (rhs.Empty())
    {
        return std::move(lhs);
    }

    String result(std::move(lhs));
    const uint32 targetLength = result.Length() + rhs.Length();
    if (!result.Reserve(targetLength))
    {
        return result;
    }

    char* buffer = result.GetBuffer();
    memcpy(buffer + result.Length(), rhs.Data(), rhs.Length());
    buffer[targetLength] = '\0';
    result.SetLength(targetLength);
    return result;
}

String operator+(const StringView& lhs, String&& rhs)
{
    if (lhs.Empty())
    {
        return std::move(rhs);
    }

    String result(std::move(rhs));
    const uint32 targetLength = result.Length() + lhs.Length();
    if (!result.Reserve(targetLength))
    {
        return result;
    }

    char* buffer = result.GetBuffer();
    memmove(buffer + lhs.Length(), buffer, result.Length());
    memcpy(buffer, lhs.Data(), lhs.Length());
    buffer[targetLength] = '\0';
    result.SetLength(targetLength);
    return result;
}

String operator+(String&& lhs, String&& rhs)
{
    if (lhs.Empty())
    {
        return std::move(rhs);
    }

    if (rhs.Empty())
    {
        return std::move(lhs);
    }

    String result;
    const uint32 targetLength = lhs.Length() + rhs.Length();

    if (targetLength + 1 <= lhs.Capacity()) // reuse buffer from lhs (without reallocation)
    {
        result = std::move(lhs);

        // [lhs] => [lhs, rhs]
        char* buffer = result.GetBuffer();
        memcpy(buffer + result.Length(), rhs.Str(), rhs.Length());
        buffer[targetLength] = '\0';
    }
    else if (targetLength + 1 <= rhs.Capacity()) // reuse buffer from rhs (without reallocation)
    {
        result = std::move(rhs);

        // [rhs] => [lhs, rhs]
        char* buffer = result.GetBuffer();
        memmove(buffer + lhs.Length(), buffer, result.Length());
        memcpy(buffer, lhs.Str(), lhs.Length());
        buffer[targetLength] = '\0';
    }
    else // create new buffer (lhs and rhs buffers are too small)
    {
        result.Reserve(targetLength);

        char* buffer = result.GetBuffer();
        memcpy(buffer, lhs.Str(), lhs.Length());
        memcpy(buffer + lhs.Length(), rhs.Str(), rhs.Length());
        buffer[targetLength] = '\0';
    }

    result.SetLength(targetLength);
    return result;
}

//////////////////////////////////////////////////////////////////////////

uint32 GetHash(const String& string)
{
    return GetHash(string.ToView());
}

} // namespace Common
} // namespace NFE
