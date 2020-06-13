#pragma once

#include "../../nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * File/stream seeking mode.
 */
enum class SeekMode : uint8
{
    Begin,   ///< relative to the stream beginning
    Current, ///< relative to current cursor position
    End,     ///< relative to the stream end
};

} // namespace Common
} // namespace NFE
