/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"
#include "../../Common/Containers/FixedArray.hpp"
#include "../../Common/Containers/StaticArray.hpp"

namespace NFE {
namespace Renderer {

/**
 * Blending mode description of a single render target.
 */
struct RenderTargetBlendStateDesc
{
    bool enable             : 1;

    BlendOp colorOperator   : 3;
    BlendOp alphaOperator   : 3;

    BlendFunc srcColorFunc  : 4;
    BlendFunc destColorFunc : 4;
    BlendFunc srcAlphaFunc  : 4;
    BlendFunc destAlphaFunc : 4;

    RenderTargetBlendStateDesc()
        : enable(false)
        , srcColorFunc(BlendFunc::One)
        , destColorFunc(BlendFunc::One)
        , srcAlphaFunc(BlendFunc::One)
        , destAlphaFunc(BlendFunc::One)
        , colorOperator(BlendOp::Add)
        , alphaOperator(BlendOp::Add)
    {}
};

/**
 * Description of Blend State object.
 */
struct BlendStateDesc
{
    Common::FixedArray<RenderTargetBlendStateDesc, MAX_RENDER_TARGETS> rtDescs;
    bool independent : 1; //< if set to false structure at index 0 is applied to all rendertargets
    bool alphaToCoverage : 1;

    BlendStateDesc()
        : independent(false)
        , alphaToCoverage(false)
    {}
};


/**
 * Description of Rasterizer State object.
 */
struct RasterizerStateDesc
{
    CullMode cullMode : 2;
    FillMode fillMode : 1;

    RasterizerStateDesc()
        : cullMode(CullMode::Disabled)
        , fillMode(FillMode::Solid)
    {}
};


/**
 * Description of Depth State object.
 */
struct DepthStateDesc
{
    // enable testing pixels against depth buffer
    bool depthTestEnable : 1;

    // enable writing to depth buffer
    bool depthWriteEnable : 1;

    // enable testing pixels against stencil buffer
    bool stencilEnable : 1;

    // comparison function for depth tesing
    CompareFunc depthCompareFunc : 3;

    uint8 stencilMask;

    // stencil buffer operation taken when stencil test fails
    StencilOp stencilOpFail : 3;

    // stencil buffer operation taken when stencil test passes, but depth test fails
    StencilOp stencilOpDepthFail : 3;

    // stencil buffer operation taken when both stencil and depth tests pass
    StencilOp stencilOpPass : 3;

    CompareFunc stencilFunc : 3;

    DepthStateDesc()
        : depthTestEnable(false)
        , depthWriteEnable(false)
        , stencilEnable(false)
        , depthCompareFunc(CompareFunc::Pass)
        , stencilMask(0xFF)
        , stencilOpFail(StencilOp::Keep)
        , stencilOpDepthFail(StencilOp::Keep)
        , stencilOpPass(StencilOp::Keep)
        , stencilFunc(CompareFunc::Pass)
    {}
};

/**
 * Description of Pipeline State object.
 */
struct PipelineStateDesc
{
    ShaderPtr vertexShader;
    ShaderPtr hullShader;
    ShaderPtr domainShader;
    ShaderPtr geometryShader;
    ShaderPtr pixelShader;

    RasterizerStateDesc raterizerState;
    BlendStateDesc blendState;
    DepthStateDesc depthState;

    PrimitiveType primitiveType = PrimitiveType::Unknown;
    uint8 numControlPoints = 0; //< for tessellation
    VertexLayoutPtr vertexLayout;

    // list of render targets, empty means no render targets (depth buffer only)
    Common::StaticArray<Format, MAX_RENDER_TARGETS> renderTargetFormats;
    Format depthFormat = Format::Unknown;

    // for multisampling
    uint8 numSamples = 1u;

    ResourceBindingLayoutPtr resBindingLayout;

    // optional debug name
    const char* debugName = nullptr;
};

/**
 * Pipeline State interface.
 * @details Pipeline State object aggregates rasterizer, blend and depth states.
 */
class IPipelineState
{
public:
    virtual ~IPipelineState() {}
};

} // namespace Renderer
} // namespace NFE
