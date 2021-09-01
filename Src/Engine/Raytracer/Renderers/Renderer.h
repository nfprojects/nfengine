#pragma once

#include "RendererContext.h"

#include "../Color/RayColor.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/Ray.hpp"
#include "../../Common/Containers/SharedPtr.hpp"
#include "../../Common/Containers/StringView.hpp"
#include "../../Common/Containers/ArrayView.hpp"
#include "../../Common/Memory/Aligned.hpp"
#include "../../Common/Reflection/ReflectionClassDeclare.hpp"
#include "../../Common/Reflection/Object.hpp"


namespace NFE {
namespace RT {

// abstract scene rendering interface
class NFE_ALIGN(16) IRenderer : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IRenderer)

public:
    NFE_ALIGNED_CLASS(16)

    struct RenderParam
    {
        const Scene& scene;
        const Camera& camera;
        uint32 iteration;
        Film& film;
    };

    IRenderer();

    NFE_RAYTRACER_API virtual ~IRenderer();

    // TODO batch & multisample rendering
    // TODO cancelation of ongoing rendering

    // create per-thread context
    virtual RendererContextPtr CreateContext() const;

    // optional rendering pre-pass, called once before all RenderPixel
    // can build custom tasks graph for internal state update
    virtual void PreRender(Common::TaskBuilder& builder, const RenderParam& renderParams, Common::ArrayView<RenderingContext> contexts);

    // called for every pixel on screen during rendering
    // Note: this will be called from multiple threads, each thread provides own RenderingContext
    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const = 0;

    virtual void Raytrace_Packet(RayPacket& packet, const RenderParam& param, RenderingContext& context) const;

protected:

    static constexpr float SecondaryRayOffset = 0.001f;
    static constexpr float SecondaryRayLengthScale = 0.999f;

private:
    IRenderer(const IRenderer&) = delete;
    IRenderer& operator = (const IRenderer&) = delete;
    IRenderer(IRenderer&&) = delete;
    IRenderer& operator = (IRenderer&&) = delete;
};

using RendererPtr = Common::UniquePtr<IRenderer>;

NFE_RAYTRACER_API RendererPtr CreateRenderer(const Common::StringView name, const Scene& scene);

} // namespace RT
} // namespace NFE
