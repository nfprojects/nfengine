/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  String definitions.
 */

#include "PCH.hpp"
#include "String.hpp"
#include "../Memory/DefaultAllocator.hpp"
#include "../Math/Math.hpp"


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

String::String(const StringView& view)
    : mInternalData()
{
    const uint32 length = view.Length();
    if (length <= NFE_INTERNAL_STRING_LENGTH)
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
    return view;
}

bool String::Reserve(uint32 length)
{
    if (length <= NFE_INTERNAL_STRING_LENGTH)
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
        LOG_ERROR("Memory allocation failed");
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
        return String(std::move(lhs));
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
        return String(std::move(rhs));
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
        return String(std::move(rhs));
    }

    if (rhs.Empty())
    {
        return String(std::move(lhs));
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
    else if (targetLength <= rhs.Capacity()) // reuse buffer from rhs (without reallocation)
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

String operator+(const StringView& lhs, char rhs)
{
    String result;
    const uint32 targetLength = lhs.Length() + 1;
    if (!result.Reserve(targetLength))
    {
        return result;
    }

    char* buffer = result.GetBuffer();
    memcpy(buffer, lhs.Data(), lhs.Length());
    buffer[lhs.Length()] = rhs;
    buffer[targetLength] = '\0';
    result.SetLength(targetLength);
    return result;
}

String operator+(String&& lhs, char rhs)
{
    String result(std::move(lhs));
    const uint32 targetLength = result.Length() + 1;
    if (!result.Reserve(targetLength))
    {
        return result;
    }

    char* buffer = result.GetBuffer();
    buffer[result.Length()] = rhs;
    buffer[targetLength] = '\0';
    result.SetLength(targetLength);
    return result;
}

String operator+(char lhs, const StringView& rhs)
{
    String result;
    const uint32 targetLength = 1 + rhs.Length();
    if (!result.Reserve(targetLength))
    {
        return result;
    }

    char* buffer = result.GetBuffer();
    buffer[0] = lhs;
    memcpy(buffer + 1, rhs.Data(), rhs.Length());
    buffer[targetLength] = '\0';
    result.SetLength(targetLength);
    return result;
}

String operator+(char lhs, String&& rhs)
{
    String result(std::move(rhs));
    const uint32 targetLength = result.Length() + 1;
    if (!result.Reserve(targetLength))
    {
        return result;
    }

    char* buffer = result.GetBuffer();
    memmove(buffer + 1, buffer, result.Length());
    buffer[0] = lhs;
    buffer[targetLength] = '\0';
    result.SetLength(targetLength);
    return result;
}

} // namespace Common
} // namespace NFE
