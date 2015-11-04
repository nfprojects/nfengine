/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

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

    const char* debugName;   //< optional debug name

    RenderTargetBlendStateDesc()
        : enable(false)
        , srcColorFunc(BlendFunc::One)
        , destColorFunc(BlendFunc::One)
        , srcAlphaFunc(BlendFunc::One)
        , destAlphaFunc(BlendFunc::One)
        , colorOperator(BlendOp::Add)
        , alphaOperator(BlendOp::Add)
        , debugName(nullptr)
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
    const char* debugName;   //< optional debug name

    BlendStateDesc()
        : rtDescs()
        , independent(false)
        , alphaToCoverage(false)
        , debugName(nullptr)
    {}
};

/**
 * Blend State interface.
 *
 * @details Blend State object describes blending mode used in the rendring pipeline.
 */
class IBlendState
{
public:
    virtual ~IBlendState() {}
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
    const char* debugName;  //< optional debug name

    RasterizerStateDesc()
        : cullMode(CullMode::Disabled)
        , fillMode(FillMode::Solid)
        , scissorTest(false)
        , debugName(nullptr)
    {}
};

/**
 * Rasterizer State interface.
 *
 * @details Rasterizer State object describes polygon rasterization mode used in the rendring
 *          pipeline.
 */
class IRasterizerState
{
public:
    virtual ~IRasterizerState() {}
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

    const char* debugName;   //< optional debug name

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
        , debugName(nullptr)
    {}
};

/**
 * Depth State interface.
 *
 * @details Depth State object describes depth testing used in the rendring pipeline.
 */
class IDepthState
{
public:
    virtual ~IDepthState() {}
};

} // namespace Renderer
} // namespace NFE
