/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common Windows helpers definitions.
 */

#include "PCH.hpp"
#include "Common.hpp"

#include <strsafe.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace NFE {
namespace Common {

bool UTF8ToUTF16(const StringView in, Utf16String& out)
{
    if (in.Empty())
    {
        out.Clear();
        return true;
    }

    const int numWideChars = ::MultiByteToWideChar(CP_UTF8, 0, in.Data(), in.Length(), nullptr, 0);
    if (numWideChars <= 0)
    {
        // conversion failed
        return false;
    }

    out.Resize(numWideChars);
    const int ret = ::MultiByteToWideChar(CP_UTF8, 0, in.Data(), in.Length(), out.Data(), numWideChars);
    if (ret <= 0)
    {
        // conversion failed
        return false;
    }

    return true;
}

bool UTF16ToUTF8(const Utf16StringView in, String& out)
{
    if (in.Empty())
    {
        out.Clear();
        return true;
    }

    int numChars = ::WideCharToMultiByte(CP_UTF8, 0, in.Data(), in.Size(), nullptr, 0, 0, 0);
    if (numChars <= 0)
    {
        // conversion failed
        return false;
    }

    DynArray<char> buffer;
    buffer.Resize(numChars);

    int wideChars = ::WideCharToMultiByte(CP_UTF8, 0, in.Data(), in.Size(), buffer.Data(), numChars, 0, 0);
    if (wideChars <= 0)
    {
        // conversion failed
        return false;
    }

    buffer.PushBack(0);
    out = buffer.Data();

    return true;
}

String GetLastErrorString()
{
    const int bufferLength = 1024;

    DWORD lastError = ::GetLastError();
    wchar_t buffer[bufferLength];

    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, bufferLength - 1, NULL);

    String shortString = "?";
    UTF16ToUTF8(Utf16StringView(buffer, wcslen(buffer)), shortString);
    return shortString;
}

} // namespace Common
} // namespace NFE
