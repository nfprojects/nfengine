#include "PCH.h"

#include "PathTracer.h"
#include "PathTracerMIS.h"
#include "LightTracer.h"
#include "VertexConnectionAndMerging.h"
#include "DebugRenderer.h"

namespace NFE {
namespace RT {

using namespace Common;

IRenderer::IRenderer(const Scene& scene)
    : mScene(scene)
{
}

IRenderer::~IRenderer() = default;

RendererContextPtr IRenderer::CreateContext() const
{
    return nullptr;
}

void IRenderer::PreRender(uint32, const Film&)
{
}

void IRenderer::PreRender(uint32, RenderingContext&)
{
}

void IRenderer::PreRenderGlobal(RenderingContext&)
{
}

void IRenderer::PreRenderGlobal()
{
}

void IRenderer::Raytrace_Packet(RayPacket&, const Camera&, Film&, RenderingContext&) const
{
}


// TODO use reflection
RendererPtr CreateRenderer(const StringView name, const Scene& scene)
{
    if (name == StringView("Path Tracer"))
    {
        return MakeSharedPtr<PathTracer>(scene);
    }
    else if (name == StringView("Path Tracer MIS"))
    {
        return MakeSharedPtr<PathTracerMIS>(scene);
    }
    else if (name == StringView("Light Tracer"))
    {
        return MakeSharedPtr<LightTracer>(scene);
    }
    else if (name == StringView("Debug"))
    {
        return MakeSharedPtr<DebugRenderer>(scene);
    }
    else if (name == StringView("VCM"))
    {
        return MakeSharedPtr<VertexConnectionAndMerging>(scene);
    }

    return nullptr;
}

} // namespace RT
} // namespace NFE
