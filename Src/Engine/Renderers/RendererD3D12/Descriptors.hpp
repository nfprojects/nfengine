/**
 * @file
 * @author  Witek902
 * @brief   Descriptor types definitions.
 */

#pragma once

#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/Containers/DynArray.hpp"

namespace NFE {
namespace Renderer {

// descriptor ID is an offset in staging descriptor heap
using DescriptorID = uint32;

enum class DescriptorType : uint8
{
    CBV,
    SRV,
    UAV,
    Sampler,
};

struct DescriptorRange
{
    DescriptorID baseDescriptor = UINT32_MAX;
    uint32 numDescriptors = 0;
};

using ReferencedDescriptorsRanges = Common::DynArray<DescriptorRange>;

} // namespace Renderer
} // namespace NFE
