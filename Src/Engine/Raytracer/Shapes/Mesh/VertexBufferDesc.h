#pragma once

#include "../../../Common/Math/Vec3f.hpp"
#include "../../../Common/Containers/SharedPtr.hpp"

namespace NFE {
namespace RT {

using MaterialPtr = Common::SharedPtr<Material>;

/**
 * Structure describing a vertex buffer.
 */
struct VertexBufferDesc
{
    uint32 numVertices = 0;
    uint32 numTriangles = 0;
    uint32 numMaterials = 0;

    const uint32* vertexIndexBuffer = nullptr;
    const Math::Vec3f* positions = nullptr;
    const Math::Vec3f* normals = nullptr;
    const Math::Vec3f* tangents = nullptr;
    const Math::Vec2f* texCoords = nullptr;
    const uint32* materialIndexBuffer = nullptr;
    const MaterialPtr* materials = nullptr;
};

} // namespace RT
} // namespace NFE
