/**
 * @file    Witek902
 * @brief   String utilities declarations.
 */

#pragma once

#include "../Containers/StringView.hpp"
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

// Convert bytes into human-readable form, e.g. "1.23 GB"
NFCOMMON_API String BytesToString(uint32 size, bool withSpace = true);
NFCOMMON_API String BytesToString(uint64 size, bool withSpace = true);

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
