/**
 * @file    Witek902
 * @brief   String utilities declarations.
 */

#pragma once

#include "../Containers/String.hpp"
#include "../Containers/StringView.hpp"
#include "../Containers/DynArray.hpp"


namespace NFE {
namespace Common {


/**
 * Fundamental types to string converter.
 *
 * Accept fundamental types as input param.
 *
 * Returns string representation of provided value.
 *
 * @{
 */
NFCOMMON_API String ToString(bool value);
NFCOMMON_API String ToString(uint32 value);
NFCOMMON_API String ToString(uint64 value);
NFCOMMON_API String ToString(int32 value);
NFCOMMON_API String ToString(int64 value);
NFCOMMON_API String ToString(float value);
NFCOMMON_API String ToString(double value);
/**
 * @}
 */

/**
 * String to fundamental types converter.
 *
 * Accept String or StringView as input param.
 * Value is returned in output param "ret". In case of error, it is left unchanged.
 *
 * Returns true when successful, false on parsing error.
 *
 * @todo These functions don't check for overflows yet - make sure they do.
 *
 * @{
 */
NFCOMMON_API bool FromString(const StringView& string, int32& ret);
NFCOMMON_API bool FromString(const StringView& string, uint32& ret);

NFE_FORCE_INLINE bool FromString(const String& string, int32& ret)
{
    return FromString(string.ToView(), ret);
}

NFE_FORCE_INLINE bool FromString(const String& string, uint32& ret)
{
    return FromString(string.ToView(), ret);
}
/**
 * @}
 */

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
