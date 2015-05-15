/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "Types.hpp"

namespace NFE {
namespace Renderer {

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

class IBlendState
{
public:
    virtual ~IBlendState() {}
};

struct RasterizerStateDesc
{
    CullMode cullMode;
    FillMode fillMode;
    // TODO: more options
};

class IRasterizerState
{
public:
    virtual ~IRasterizerState() {}
};

struct DepthStateDesc
{
    // enable testing pixels against depth buffer
    bool depthTestEnable;

    // enable writing to depth buffer
    bool depthWriteEnable;

    // comparison function for depth tesing
    CompareFunc depthCompareFunc;

    // TODO: stencil buffer support
};

class IDepthState
{
public:
    virtual ~IDepthState() {}
};

} // namespace Renderer
} // namespace NFE
