/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common Windows helpers declarations.
 */

#pragma once

#include "../../nfCommon.hpp"


namespace NFE {
namespace Common {

// TODO implement custom "WString" class (Windows only)

/**
 * Convert a string from UTF-8 to Windows' UTF-16.
 */
NFCOMMON_API bool UTF8ToUTF16(const String& in, std::wstring& out);

/**
* Convert a string from Windows' UTF-16 to UTF-8.
*/
NFCOMMON_API bool UTF16ToUTF8(const std::wstring& in, String& out);

/**
 * Translates GetLastError() code to a string.
 */
String GetLastErrorString();

} // namespace Common
} // namespace NFE
