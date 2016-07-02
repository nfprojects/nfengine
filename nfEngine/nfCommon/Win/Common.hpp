/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Common Windows helpers declarations.
 */

#include "../PCH.hpp"
#include "../nfCommon.hpp"

#include <string>


namespace NFE {
namespace Common {

/**
 * Convert a string from UTF-8 to Windows' UTF-16.
 */
NFCOMMON_API bool UTF8ToUTF16(const std::string& in, std::wstring& out);

/**
* Convert a string from Windows' UTF-16 to UTF-8.
*/
NFCOMMON_API bool UTF16ToUTF8(const std::wstring& in, std::string& out);

/**
 * Translates GetLastError() code to a string.
 */
std::string GetLastErrorString();

} // namespace Common
} // namespace NFE
