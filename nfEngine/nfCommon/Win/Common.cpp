/**
 * @file   Common.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common Windows helpers definitions.
 */

#include "../stdafx.hpp"
#include "Common.hpp"

#include <strsafe.h>

namespace NFE {
namespace Common {
namespace Windows {

bool UTF8ToUTF16(const std::string& in, std::wstring& out)
{
    const size_t bufferSize = 1024; // TODO
    wchar_t buffer[bufferSize];

    size_t inChars;
    HRESULT hr = ::StringCchLengthA(in.c_str(), INT_MAX - 1, &inChars);
    if (FAILED(hr))
        return false;

    ++inChars;

    // TODO: allocate buffer
    /*
    int outChars = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.c_str(), static_cast<int>(inChars), NULL, 0);
    if (outChars == 0)
    return false;
    */

    int result = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.c_str(), inChars, buffer, bufferSize);
    if (result == 0)
        return false;

    out = buffer;
    return true;
}

bool UTF16ToUTF8(const std::wstring& in, std::string& out)
{
    const size_t bufferSize = 1024; // TODO
    char buffer[bufferSize];

    size_t inChars;
    HRESULT hr = ::StringCchLengthW(in.c_str(), INT_MAX - 1, &inChars);
    if (FAILED(hr))
        return false;

    ++inChars;

    // TODO: allocate buffer
    /*
    int outChars = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), static_cast<int>(inChars), NULL, 0);
    if (outChars == 0)
    return false;
    */

    int result = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), inChars, buffer, bufferSize, 0, 0);
    if (result == 0)
        return false;

    out = buffer;
    return true;
}

std::string GetLastErrorString()
{
    DWORD lastError = ::GetLastError();
    char buffer[256] = "?";

    // TODO: use FormatMessageW and convert UTF-16 > UTF-8
    if (lastError != 0)
        ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 255, NULL);
    return buffer;
}

} // namespace Windows
} // namespace Common
} // namespace NFE
