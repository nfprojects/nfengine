#pragma once

#include "RendererContext.h"

#include "../Color/RayColor.h"
#include "../Utils/Memory.h"
#include "../../nfCommon/Math/Ray.hpp"
#include "../../nfCommon/Containers/SharedPtr.hpp"
#include "../../nfCommon/Containers/StringView.hpp"
#include "../../nfCommon/Memory/Aligned.hpp"

namespace NFE {
namespace RT {

// abstract scene rendering interface
class NFE_ALIGN(16) IRenderer : public Common::Aligned<16>
{
public:
    struct RenderParam
    {
        uint32 iteration;
        uint32 pixelIndex;
        const Camera& camera;
        Film& film;
    };

    IRenderer(const Scene& scene);

    NFE_RAYTRACER_API virtual ~IRenderer();

    // TODO batch & multisample rendering
    // TODO cancelation of ongoing rendering

    virtual const char* GetName() const = 0;

    // create per-thread context
    virtual RendererContextPtr CreateContext() const;

    // TODO clean this up...
    // idea: each renderer should report what passes it requires, etc.

    // optional rendering pre-pass, called once per frame
    virtual void PreRender(uint32 passNumber, const Film& film);

    // optional rendering pre-pass, called once per frame for every thread
    virtual void PreRender(uint32 passNumber, RenderingContext& ctx);

    // optional rendering pre-pass, called once (single threaded)
    virtual void PreRenderGlobal(RenderingContext& ctx);
    virtual void PreRenderGlobal();

    // called for every pixel on screen during rendering
    // Note: this will be called from multiple threads, each thread provides own RenderingContext
    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const = 0;


    virtual void Raytrace_Packet(RayPacket& packet, const Camera& camera, Film& film, RenderingContext& context) const;

protected:
    const Scene& mScene;

private:
    IRenderer(const IRenderer&) = delete;
    IRenderer& operator = (const IRenderer&) = delete;
    IRenderer(IRenderer&&) = delete;
    IRenderer& operator = (IRenderer&&) = delete;
};

using RendererPtr = Common::SharedPtr<IRenderer>;

NFE_RAYTRACER_API RendererPtr CreateRenderer(const Common::StringView name, const Scene& scene);

} // namespace RT
} // namespace NFE
