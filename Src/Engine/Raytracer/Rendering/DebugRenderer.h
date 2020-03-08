#pragma once

#include "Renderer.h"
#include "../../Common/Reflection/ReflectionEnumMacros.hpp"

namespace NFE {
namespace RT {

enum class DebugRenderingMode : uint8
{
    CameraLight = 0,            // simple shading based on normal vector orientation

    // geometry
    TriangleID,                 // draw every triangle with random color
    Depth,                      // visualize depth
    Position,                   // visualize world-space position
    Normals,                    // visualize normal vectors (in world space)
    Tangents,
    Bitangents,
    TexCoords,   

    // material
    BaseColor,                  // visualize base color of the first intersection
    Emission,                   // visualize emission color
    Roughness,                  // visualize "rougness" parameter
    Metalness,                  // visualize "metalness" parameter
    IoR,                        // visualize "index of refraction" parameter

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
    // stats
    RayBoxIntersection,         // visualize number of performed ray-box intersections
    RayBoxIntersectionPassed,   // visualize number of passed ray-box intersections
    RayTriIntersection,         // visualize number of performed ray-triangle intersections
    RayTriIntersectionPassed,   // visualize number of passed ray-triangle intersections
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

    MAX
};


// Debug renderer for visualizing normals, tangents, base color, etc.
class DebugRenderer : public IRenderer
{
    NFE_DECLARE_POLYMORPHIC_CLASS(DebugRenderer);

public:
    DebugRenderer();

    virtual const char* GetName() const;
    virtual const RayColor RenderPixel(const Math::Ray& ray, const RenderParam& param, RenderingContext& ctx) const override;
    virtual void Raytrace_Packet(RayPacket& packet, const RenderParam& param, RenderingContext& context) const override;

private:
    DebugRenderingMode renderingMode;
};

} // namespace RT
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::RT::DebugRenderingMode);
