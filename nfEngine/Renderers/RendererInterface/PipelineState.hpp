/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"
#include "Shader.hpp"
#include "VertexLayout.hpp"

namespace NFE {
namespace Renderer {

/**
 * Blending mode description of a single render target.
 */
struct RenderTargetBlendStateDesc
{
    bool enable;

    BlendFunc srcColorFunc;
    BlendFunc destColorFunc;
    BlendFunc srcAlphaFunc;
    BlendFunc destAlphaFunc;

    BlendOp colorOperator;
    BlendOp alphaOperator;

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
    RenderTargetBlendStateDesc rtDescs[MAX_RENDER_TARGETS];
    bool independent; //< if set to false structure at index 0 is applied to all rendertargets
    bool alphaToCoverage;

    BlendStateDesc()
        : rtDescs()
        , independent(false)
        , alphaToCoverage(false)
    {}
};


/**
 * Description of Rasterizer State object.
 */
struct RasterizerStateDesc
{
    CullMode cullMode;
    FillMode fillMode;
    bool scissorTest;
    // TODO: more options

    RasterizerStateDesc()
        : cullMode(CullMode::Disabled)
        , fillMode(FillMode::Solid)
        , scissorTest(false)
    {}
};


/**
 * Description of Depth State object.
 */
struct DepthStateDesc
{
    // enable testing pixels against depth buffer
    bool depthTestEnable;

    // enable writing to depth buffer
    bool depthWriteEnable;

    // enable testing pixels against stencil buffer
    bool stencilEnable;

    // comparison function for depth tesing
    CompareFunc depthCompareFunc;

    unsigned char stencilMask;

    // stencil buffer operation taken when stencil test fails
    StencilOp stencilOpFail;

    // stencil buffer operation taken when stencil test passes, but depth test fails
    StencilOp stencilOpDepthFail;

    // stencil buffer operation taken when both stencil and depth tests pass
    StencilOp stencilOpPass;

    CompareFunc stencilFunc;

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
    RasterizerStateDesc raterizerState;
    BlendStateDesc blendState;
    DepthStateDesc depthState;
    PrimitiveType primitiveType;
    IVertexLayout* vertexLayout;
    IResourceBindingLayout* resBindingLayout;
    const char* debugName;   //< optional debug name

    PipelineStateDesc()
        : raterizerState()
        , blendState()
        , depthState()
        , primitiveType(PrimitiveType::Unknown)
        , vertexLayout(nullptr)
        , resBindingLayout(nullptr)
        , debugName(nullptr)
    { }
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
