#pragma once

#include "../RendererApi.hpp"
#include "../../Renderers/RendererCommon/Texture.hpp"


namespace NFE {
namespace Renderer {

class IRenderGraphNode
{
public:
    virtual ~IRenderGraphNode();
    virtual void Execute(const RenderGraphContext& context) const = 0;
    virtual const char* GetName() const = 0;
};

class RenderGraphNode_RenderScene : public IRenderGraphNode
{
public:
    RenderGraphNode_RenderScene(const Scene* scene) : mScene(scene) { }
    virtual void Execute(const RenderGraphContext& context) const override;
    virtual const char* GetName() const { return "RenderScene"; }
private:
    const Scene* mScene;
};

class RenderGraphNode_RenderImGui : public IRenderGraphNode
{
public:
    virtual void Execute(const RenderGraphContext& context) const override;
    virtual const char* GetName() const { return "RenderImGui"; }
};

class RenderGraphNode_Present : public IRenderGraphNode
{
public:
    virtual void Execute(const RenderGraphContext& context) const override;
    virtual const char* GetName() const { return "Present"; }
};

} // namespace Renderer
} // namespace NFE
