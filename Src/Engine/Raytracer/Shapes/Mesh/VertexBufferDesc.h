#pragma once

#include "../../../Common/Math/Float3.hpp"
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
    const Math::Float3* positions = nullptr;
    const Math::Float3* normals = nullptr;
    const Math::Float3* tangents = nullptr;
    const Math::Float2* texCoords = nullptr;
    const uint32* materialIndexBuffer = nullptr;
    const MaterialPtr* materials = nullptr;
};

} // namespace RT
} // namespace NFE
