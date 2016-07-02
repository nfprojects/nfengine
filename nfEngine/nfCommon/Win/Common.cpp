/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common Windows helpers definitions.
 */

#include "../PCH.hpp"
#include "Common.hpp"

#include <strsafe.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace NFE {
namespace Common {

bool UTF8ToUTF16(const std::string& in, std::wstring& out)
{
    // TODO: consider dynamic allocation
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];

    size_t inChars;
    HRESULT hr = ::StringCchLengthA(in.c_str(), INT_MAX - 1, &inChars);
    if (FAILED(hr))
        return false;

    ++inChars;

    int result = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.c_str(),
                                       static_cast<int>(inChars), buffer, bufferSize);
    if (result == 0)
        return false;

    out = buffer;
    return true;
}

bool UTF16ToUTF8(const std::wstring& in, std::string& out)
{
    // TODO: consider dynamic allocation
    const int bufferSize = 1024;
    char buffer[bufferSize];

    size_t inChars;
    HRESULT hr = ::StringCchLengthW(in.c_str(), INT_MAX - 1, &inChars);
    if (FAILED(hr))
        return false;

    ++inChars;

    int result = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), static_cast<int>(inChars), buffer,
                                       bufferSize, 0, 0);
    if (result == 0)
        return false;

    out = buffer;
    return true;
}

std::string GetLastErrorString()
{
    DWORD lastError = ::GetLastError();
    wchar_t buffer[256];

    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 255, NULL);

    std::string shortString = "?";
    UTF16ToUTF8(std::wstring(buffer), shortString);
    return shortString;
}

} // namespace Common
} // namespace NFE
