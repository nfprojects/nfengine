/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common Windows helpers declarations.
 */

#pragma once

#include "../../nfCommon.hpp"

#include "../../Containers/StringView.hpp"
#include "../../Containers/String.hpp"
#include "../../Containers/ArrayView.hpp"
#include "../../Containers/DynArray.hpp"


namespace NFE {
namespace Common {

// TODO get rid of std::wstring
using Utf16String = std::wstring;

/**
 * Convert a string from UTF-8 to Windows' UTF-16.
 */
NFCOMMON_API bool UTF8ToUTF16(const StringView in, Utf16String& out);

/**
* Convert a string from Windows' UTF-16 to UTF-8.
*/
NFCOMMON_API bool UTF16ToUTF8(const Utf16String& in, String& out);

/**
 * Translates GetLastError() code to a string.
 */
String GetLastErrorString();

} // namespace Common
} // namespace NFE
