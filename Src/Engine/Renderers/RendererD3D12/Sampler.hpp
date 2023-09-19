/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's sampler
 */

#pragma once

#include "../RendererCommon/Texture.hpp"
#include "Common.hpp"
#include "Descriptors.hpp"

namespace NFE {
namespace Renderer {

class Sampler : public ISampler
{
public:
    Sampler();
    ~Sampler();

    bool Init(const SamplerDesc& desc);

    DescriptorID GetDescriptor() const
    {
        return mDescriptor;
    }

private:
    DescriptorID mDescriptor;
};

} // namespace Renderer
} // namespace NFE
