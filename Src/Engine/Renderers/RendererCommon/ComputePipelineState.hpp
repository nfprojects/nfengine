/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Shader.hpp"


namespace NFE {
namespace Renderer {

/**
 * Description of compute pipeline state object.
 */
struct ComputePipelineStateDesc
{
    ShaderPtr computeShader;
    const char* debugName;   //< optional debug name

    ComputePipelineStateDesc(const ShaderPtr& computeShader,
                             const char* debugName = nullptr)
        : computeShader(computeShader)
        , debugName(debugName)
    {
    }
};

/**
 * Compute Pipeline State interface.
 * @details Describes GPU resources used during compute shader execution.
 */
class IComputePipelineState
{
public:
    virtual ~IComputePipelineState() {}
};

} // namespace Renderer
} // namespace NFE
