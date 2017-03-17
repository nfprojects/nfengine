/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  String definitions.
 */

#include "PCH.hpp"
#include "String.hpp"
#include "../Memory/DefaultAllocator.hpp"


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
    // TODO
    UNUSED(other);
}

String& String::operator=(String&& other)
{
    // TODO
    UNUSED(other);

    return *this;
}

String::String()
    : mInternalData()
{
}

String::String(char c)
    : mInternalData(c)
{
}

String::String(const String& other)
    : String(other.ToView())
{
}

String::String(const StringView& view)
    : mInternalData()
{
    const uint32 length = view.Length();
    if (length <= NFE_INTERNAL_STRING_LENGTH)
    {
        mInternalData.isInternal = 1;
    }
    else
    {
        if (!Reserve(length))
            return;
    }

    mInternalData.length = length;

    char* buffer = GetBuffer();
    memcpy(buffer, view.Data(), length);
    buffer[length] = '\0';
}

String& String::operator=(char c)
{
    // TODO
    UNUSED(c);

    return *this;
}

String& String::operator=(const String& other)
{
    *this = other.ToView();
    return *this;
}

String& String::operator=(const StringView& view)
{
    // TODO
    UNUSED(view);

    return *this;
}


//////////////////////////////////////////////////////////////////////////

String& String::operator+=(char c)
{
    UNUSED(c);

    return *this;
}

String& String::operator+=(const String& other)
{
    // TODO
    UNUSED(other);

    return *this;
}

String& String::operator+=(const StringView& view)
{
    // TODO
    UNUSED(view);

    return *this;
}

//////////////////////////////////////////////////////////////////////////

String& String::Insert(uint32 index, const String& other)
{
    // TODO
    UNUSED(index);
    UNUSED(other);

    return *this;
}

String& String::Insert(uint32 index, const StringView& other)
{
    // TODO
    UNUSED(index);
    UNUSED(other);

    return *this;
}

String& String::Insert(uint32 index, char c)
{
    // TODO
    UNUSED(index);
    UNUSED(c);

    return *this;
}


//////////////////////////////////////////////////////////////////////////

String& String::Erase(uint32 index, uint32 numCharacters)
{
    // TODO
    UNUSED(index);
    UNUSED(numCharacters);

    return *this;
}

String& String::Replace(uint32 index, uint32 numCharacters, const String& other)
{
    // TODO
    UNUSED(index);
    UNUSED(numCharacters);
    UNUSED(other);

    return *this;
}

String& String::Replace(uint32 index, uint32 numCharacters, const StringView& other)
{
    // TODO
    UNUSED(index);
    UNUSED(numCharacters);
    UNUSED(other);

    return *this;
}

String& String::Replace(uint32 index, uint32 numCharacters, char c)
{
    // TODO
    UNUSED(index);
    UNUSED(numCharacters);
    UNUSED(c);

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
        mExternalData.isInternal = 0;
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


String operator+(const String& lhs, const String& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(String&& lhs, const String& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(const String& lhs, String&& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(String& lhs, String&& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(const String& lhs, const StringView& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(String&& lhs, const StringView& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(const StringView& lhs, const String& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(const StringView& lhs, String&& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(const String& lhs, char rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(String&& lhs, char rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(char lhs, const String& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}

String operator+(char lhs, String&& rhs)
{
    // TODO
    UNUSED(lhs);
    UNUSED(rhs);
    return String();
}


} // namespace Common
} // namespace NFE
