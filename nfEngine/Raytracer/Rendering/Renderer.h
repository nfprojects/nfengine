#pragma once

#include "RendererContext.h"

#include "../Color/RayColor.h"
#include "../Utils/Memory.h"
#include "../../nfCommon/Math/Ray.hpp"
#include "../../nfCommon/Containers/SharedPtr.hpp"
#include "../../nfCommon/Containers/StringView.hpp"
#include "../../nfCommon/Containers/ArrayView.hpp"
#include "../../nfCommon/Memory/Aligned.hpp"
#include "../../nfCommon/Reflection/ReflectionClassDeclare.hpp"
#include "../../nfCommon/Reflection/Object.hpp"


namespace NFE {
namespace RT {

// abstract scene rendering interface
class NFE_ALIGN(16) IRenderer
    : public Common::Aligned<16>
    , public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(IRenderer);

public:
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
