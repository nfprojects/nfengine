#pragma once

#include "../RendererApi.hpp"
#include "../../Renderers/RendererCommon/Buffer.hpp"
#include "../../Common/Containers/DynArray.hpp"
#include "../../Common/Memory/Buffer.hpp"
#include "../../Common/Math/Box.hpp"


namespace NFE {
namespace Renderer {

//
class MeshPart
{
    NFE_DECLARE_CLASS(MeshPart)

    //ResourcePath material;
    uint32 firstVertex;
    uint32 firstIndex;
    uint32 numTriangles;
};

class MeshLodLevel
{
    NFE_DECLARE_CLASS(MeshLodLevel)

    float maxDistance;
    Common::DynArray<MeshPart> parts;
};

class Mesh : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(Mesh)

private:
    Common::Buffer mVerticesData;
    Common::DynArray<uint16> mIndices;
    Common::DynArray<MeshLodLevel> mLodLevels;
    Math::Box mBoundingBox; // TODO per-chunk bounding box?

    // TODO single buffer for indices and vertices
    // TODO keeping small meshes in external buffer to avoid padding waste
    BufferPtr mVertexBuffer;
    BufferPtr mIndexBuffer;
};


} // namespace Renderer
} // namespace NFE
