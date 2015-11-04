/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OGL4 Renderer's pipeline states declarations
 */

#pragma once

#include "../RendererInterface/PipelineState.hpp"

namespace NFE {
namespace Renderer {

class BlendState : public IBlendState
{
    friend class CommandBuffer;
    BlendStateDesc mDesc;

public:
    BlendState();
    ~BlendState();
    bool Init(const BlendStateDesc& desc);
};

class DepthState : public IDepthState
{
    friend class CommandBuffer;
    DepthStateDesc mDesc;

public:
    DepthState();
    ~DepthState();
    bool Init(const DepthStateDesc& desc);
};

class RasterizerState : public IRasterizerState
{
    friend class CommandBuffer;
    RasterizerStateDesc mDesc;

public:
    RasterizerState();
    ~RasterizerState();
    bool Init(const RasterizerStateDesc& desc);
};

} // namespace Renderer
} // namepsace NFE
