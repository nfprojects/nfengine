#include "PCH.h"
#include "Renderer.h"
#include "../Common/Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_POLYMORPHIC_CLASS(NFE::RT::IRenderer)
NFE_END_DEFINE_CLASS()

namespace NFE {
namespace RT {

using namespace Common;

IRenderer::IRenderer() = default;

IRenderer::~IRenderer() = default;

RendererContextPtr IRenderer::CreateContext() const
{
    return nullptr;
}

void IRenderer::PreRender(Common::TaskBuilder& builder, const RenderParam& renderParams, Common::ArrayView<RenderingContext> contexts)
{
    NFE_UNUSED(builder);
    NFE_UNUSED(renderParams);
    NFE_UNUSED(contexts);
}

void IRenderer::Raytrace_Packet(RayPacket&, const RenderParam&, RenderingContext&) const
{
}

RendererPtr CreateRenderer(const StringView name, const Scene&)
{
    DynArray<const RTTI::ClassType*> types;
    RTTI::GetType<IRenderer>()->ListSubtypes(types);
    for (const RTTI::ClassType* type : types)
    {
        if (type->IsConstructible() && type->GetName() == name)
        {
            return RendererPtr(type->CreateObject<IRenderer>());
        }
    }

    return nullptr;
}

} // namespace RT
} // namespace NFE
