/**
 * @file    Texture.hpp
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

class TextureDesc
{

};

class ITexture
{
public:
    virtual ~ITexture() {}
};

struct SamplerDesc
{

};

class ISampler
{
public:
    virtual ~ISampler()
    {
    }
};

} // namespace Renderer
} // namespace NFE
