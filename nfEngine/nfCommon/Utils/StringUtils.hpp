/**
 * @file    Witek902
 * @brief   String utilities declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Containers/String.hpp"
#include "../Containers/DynArray.hpp"


namespace NFE {
namespace Common {


// Converting fundamental types to string
NFCOMMON_API String ToString(bool value);
NFCOMMON_API String ToString(uint32 value);
NFCOMMON_API String ToString(uint64 value);
NFCOMMON_API String ToString(int32 value);
NFCOMMON_API String ToString(int64 value);
NFCOMMON_API String ToString(float value);
NFCOMMON_API String ToString(double value);

/**
 * Split given string view into tokens, accoridng to a given delimiter
 * @param   a       Input string which is to be split
 * @param   delim   Character determining which element will be split.
 *                  Defaults to '\n', which will split given string into
 *                  consecutive lines. This char will NOT be included in any
 *                  of resulting string views.
 * @return Array of StringView elements, each containing resulting tokens.
 *         If there is no @p delim character in input, returns one-element
 *         array containing a view on entire input string.
 */
NFCOMMON_API DynArray<StringView> Split(const StringView& a, char delim = '\n');


} // namespace Common
} // namespace NFE
