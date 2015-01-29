/**
 * @file   Common.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common Windows helpers declarations.
 */

#include "../stdafx.hpp"

namespace NFE {
namespace Common {
namespace Windows {

/**
 * Convert a string from UTF-8 to Windows' UTF-16.
 */
bool UTF8ToUTF16(const std::string& in, std::wstring& out);

/**
* Convert a string from Windows' UTF-16 to UTF-8.
*/
bool UTF16ToUTF8(const std::wstring& in, std::string& out);

/**
 * Translates GetLastError() code to a string.
 */
std::string GetLastErrorString();

} // namespace Windows
} // namespace Common
} // namespace NFE
