#pragma once

#include "../../Raytracer.h"

#include "VertexBufferDesc.h"

#include "../../../Common/Math/Vec4f.hpp"
#include "../../../Common/Math/Triangle.hpp"
#include "../../../Common/Math/Vec3f.hpp"
#include "../../../Common/Containers/DynArray.hpp"

namespace NFE {
namespace RT {

class Material;

struct NFE_ALIGN(16) VertexIndices
{
    uint32 i0;
    uint32 i1;
    uint32 i2;
    uint32 materialIndex;
};

struct NFE_ALIGN(32) VertexShadingData
{
    Math::Vec3f normal;
    Math::Vec3f tangent;
    Math::Vec2f texCoord;
};


// Structure containing packed mesh data (vertices, vertex indices and material indices).
class VertexBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    // Clear buffers
    void Clear();

    // Initialize the vertex buffer with a new content
    bool Initialize(const VertexBufferDesc& desc);

    // get vertex indices for given triangle
    void GetVertexIndices(const uint32 triangleIndex, VertexIndices& indices) const;

    // get material for given a triangle
    const Material* GetMaterial(const uint32 materialIndex) const;

    // extract preprocessed triangle data (for one triangle)
    NFE_FORCE_INLINE const Math::ProcessedTriangle& GetTriangle(const uint32 triangleIndex) const
    {
        return mPreprocessedTriangles[triangleIndex];
    }

    void GetTriangle(const uint32 triangleIndex, Math::Triangle_Simd8& outTriangle) const;

    void GetShadingData(const VertexIndices& indices, VertexShadingData& a, VertexShadingData& b, VertexShadingData& c) const;

    NFE_FORCE_INLINE uint32 GetNumVertices() const { return mNumVertices; }
    NFE_FORCE_INLINE uint32 GetNumTriangles() const { return mNumTriangles; }

private:

    char* mBuffer;
    Math::ProcessedTriangle* mPreprocessedTriangles;

    size_t mVertexIndexBufferOffset;
    size_t mShadingDataBufferOffset;
    size_t mMaterialBufferOffset;

    uint32 mNumVertices;
    uint32 mNumTriangles;

    Common::DynArray<MaterialPtr> mMaterials;
};


} // namespace RT
} // namespace NFE
