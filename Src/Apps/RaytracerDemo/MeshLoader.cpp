#include "PCH.h"
#include "MeshLoader.h"

#include "Engine/Raytracer/Utils/Bitmap.h"
#include "Engine/Raytracer/Textures/BitmapTexture.h"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/Math/Geometry.hpp"
#include "Engine/Common/Math/HdrColor.hpp"
#include "Engine/Common/FileSystem/FileSystem.hpp"

#include <tinyobjloader/tiny_obj_loader.h>


namespace NFE {
namespace helpers {

using namespace RT;
using namespace Math;
using namespace Common;

struct TriangleIndicesComparator
{
    NFE_FORCE_INLINE bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const
    {
        return (a.vertex_index == b.vertex_index) && (a.normal_index == b.normal_index) && (a.texcoord_index == b.texcoord_index);
    }
};

struct TriangleIndicesHash
{
    NFE_FORCE_INLINE uint32 operator()(const tinyobj::index_t& k) const
    {
        return k.vertex_index ^ k.normal_index ^ k.texcoord_index;
    }
};

BitmapPtr LoadBitmapObject(const StringView& baseDir, const StringView& path)
{
    if (path.Empty())
    {
        return nullptr;
    }

    String fullPath = baseDir + path;
    if (fullPath.ToView().EndsWith(StringView(".png")) || fullPath.ToView().EndsWith(StringView(".jpg")))
    {
        fullPath.Replace(fullPath.Length() - 4, 4, ".bmp");
    }

    // cache bitmaps so they are loaded only once
    static HashMap<String, BitmapPtr> bitmapsList;
    BitmapPtr& bitmapPtr = bitmapsList.Insert(fullPath, BitmapPtr()).iterator->second;

    if (!bitmapPtr)
    {
        bitmapPtr = MakeSharedPtr<Bitmap>(fullPath.Str());
        if (!bitmapPtr->Load(fullPath.Str()))
        {
            return nullptr;
        }
    }

    return bitmapPtr;
}

TexturePtr LoadTexture(const StringView& baseDir, const StringView& path)
{
    BitmapPtr bitmap = LoadBitmapObject(baseDir, path);

    if (!bitmap)
    {
        return nullptr;
    }

    if (bitmap->GetWidth() > 0u && bitmap->GetHeight() > 0u)
    {
        return MakeSharedPtr<BitmapTexture>(bitmap);
    }

    return nullptr;
}

MaterialPtr LoadMaterial(const StringView& baseDir, const tinyobj::material_t& sourceMaterial)
{
    auto material = MakeSharedPtr<Material>();

    material->SetBsdf("diffuse"); // TODO
    material->debugName = sourceMaterial.name.c_str();
    material->baseColor = HdrColorRGB(sourceMaterial.diffuse[0], sourceMaterial.diffuse[1], sourceMaterial.diffuse[2]);
    material->emission = HdrColorRGB(sourceMaterial.emission[0], sourceMaterial.emission[1], sourceMaterial.emission[2]);
    material->baseColor.SetTexture(LoadTexture(baseDir, sourceMaterial.diffuse_texname.c_str()));
    material->normalMap = LoadTexture(baseDir, sourceMaterial.normal_texname.c_str());
    material->maskMap = LoadTexture(baseDir, sourceMaterial.alpha_texname.c_str());
    material->roughness = 0.075f;

    material->Compile();

    return material;
}

MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials)
{
    auto material = MakeSharedPtr<Material>();
    material->debugName = "default";
    material->baseColor = HdrColorRGB(0.8f, 0.8f, 0.8f);
    material->emission = HdrColorRGB(0.0f, 0.0f, 0.0f);
    material->roughness = 0.75f;
    material->SetBsdf(Material::DefaultBsdfName);
    material->Compile();

    outMaterials.Insert(material->debugName, material);
    return material;
}

class MeshLoader
{
public:
    static constexpr float MinEdgeLength = 0.001f;
    static constexpr float MinEdgeLengthSqr = Sqr(MinEdgeLength);

    MeshLoader()
    {
    }

    bool LoadMesh(const String& filePath, MaterialsMap& outMaterials, const float scale)
    {
        NFE_LOG_DEBUG("Loading mesh file: '%s'...", filePath.Str());

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        const String meshBaseDir = String(FileSystem::GetParentDir(filePath)) + "/";

        {
            Timer timer;
            std::string warning, err;
            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &err, filePath.Str(), meshBaseDir.Str(), true, false);
            if (!warning.empty())
            {
                NFE_LOG_WARNING("Mesh '%s' loading message:\n%s", filePath.Str(), err.c_str());
            }
            if (!err.empty())
            {
                NFE_LOG_ERROR("Mesh '%s' loading message:\n%s", filePath.Str(), err.c_str());
            }
            if (!ret)
            {
                NFE_LOG_ERROR("Failed to load mesh '%s'", filePath.Str());
                return false;
            }

            NFE_LOG_INFO("Mesh file '%s' parsed in %.3f seconds", filePath.Str(), timer.Stop());
        }

        // Loop over shapes
        for (size_t shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++)
        {
            // Loop over faces
            const tinyobj::mesh_t& mesh = shapes[shapeIndex].mesh;

            for (size_t faceIndex = 0; faceIndex < mesh.num_face_vertices.size(); faceIndex++)
            {
                const int numFaceVertices = mesh.num_face_vertices[faceIndex];
                if (numFaceVertices != 3)
                {
                    NFE_LOG_ERROR("Expected only triangles (shape index = %zu, face index = %zu)", shapeIndex, faceIndex);
                    return false;
                }

                const tinyobj::index_t idx[3] =
                {
                    mesh.indices[3u * faceIndex + 0],
                    mesh.indices[3u * faceIndex + 1],
                    mesh.indices[3u * faceIndex + 2],
                };

                const bool hasNormals = idx[0].normal_index >= 0 && idx[1].normal_index >= 0 && idx[2].normal_index >= 0;
                const bool hasTexCoords = idx[0].texcoord_index >= 0 && idx[1].texcoord_index >= 0 && idx[2].texcoord_index >= 0;

                Vec4f verts[3];
                for (size_t i = 0; i < 3; i++)
                {
                    verts[i] = scale * Vec4f(
                        attrib.vertices[3 * idx[i].vertex_index + 0],
                        attrib.vertices[3 * idx[i].vertex_index + 1],
                        attrib.vertices[3 * idx[i].vertex_index + 2]);
                }

                // discard degenerate triangles
                const Vec4f edge1 = verts[1] - verts[0];
                const Vec4f edge2 = verts[2] - verts[0];
                const Vec4f edge3 = verts[2] - verts[1];
                if (edge1.SqrLength3() < MinEdgeLengthSqr ||
                    edge2.SqrLength3() < MinEdgeLengthSqr ||
                    edge3.SqrLength3() < MinEdgeLengthSqr ||
                    TriangleSurfaceArea(edge1, edge2) < MinEdgeLengthSqr)
                {
                    NFE_LOG_WARNING("Mesh has degenerate triangle (shape index = %zu, face index = %zu)", shapeIndex, faceIndex);
                    continue;
                }

                // compute per-face normal
                const Vec4f faceNormal = Vec4f::Cross3(verts[1] - verts[0], verts[2] - verts[0]).Normalized3();
                NFE_ASSERT(faceNormal.IsValid(), "");

                for (size_t i = 0; i < 3; i++)
                {
                    const tinyobj::index_t indices = idx[i];
                    const auto iter = mUniqueIndices.Find(indices);

                    uint32 uniqueIndex = 0;
                    if (iter != mUniqueIndices.end())
                    {
                        uniqueIndex = (*iter).second;
                    }
                    else
                    {
                        uniqueIndex = mUniqueIndices.Size();
                        mUniqueIndices.Insert(indices, uniqueIndex);

                        mVertexPositions.PushBack(verts[i].ToVec3f());

                        if (hasNormals)
                        {
                            const Vec4f normal(
                                attrib.normals[3 * indices.normal_index + 0],
                                attrib.normals[3 * indices.normal_index + 1],
                                attrib.normals[3 * indices.normal_index + 2]);
                            mVertexNormals.PushBack(normal.Normalized3().ToVec3f());
                        }
                        else
                        {
                            // fallback to face normal
                            // TODO smooth shading
                            mVertexNormals.PushBack(faceNormal.ToVec3f());
                        }

                        if (hasTexCoords)
                        {
                            mVertexTexCoords.PushBack(Vec2f(attrib.texcoords.data() + 2 * idx[i].texcoord_index));
                        }
                        else
                        {
                            mVertexTexCoords.PushBack(Vec2f());
                        }
                    }

                    mVertexIndices.PushBack(uniqueIndex);
                }

                mMaterialIndices.PushBack(mesh.material_ids[faceIndex]);
            }
        }

        NFE_ASSERT(mVertexPositions.Size() == mUniqueIndices.Size(), "");
        NFE_ASSERT(mVertexPositions.Size() == mVertexNormals.Size(), "");
        NFE_ASSERT(mVertexPositions.Size() == mVertexTexCoords.Size(), "");

        ComputeTangentVectors();

        // load materials
        mMaterialPointers.Reserve((uint32)materials.size());
        for (size_t i = 0; i < materials.size(); i++)
        {
            auto material = LoadMaterial(meshBaseDir, materials[i]);
            mMaterialPointers.PushBack(material);
            outMaterials.Insert(material->debugName, material);
        }

        // fallback to default material
        if (materials.empty())
        {
            NFE_LOG_WARNING("No materials found in mesh '%s'. Falling back to the default material.", filePath.Str());

            mMaterialPointers.PushBack(CreateDefaultMaterial(outMaterials));

            for (uint32& index : mMaterialIndices)
            {
                index = 0;
            }
        }

        NFE_LOG_DEBUG("Mesh file '%s' loaded, vertices = %zu, indices = %zu, materials = %zu", filePath.Str(), mVertexPositions.Size(), mVertexIndices.Size(), mMaterialPointers.Size());
        mFilePath = filePath;

        return true;
    }

    void ComputeTangentVectors()
    {
        mVertexTangents.Resize(mVertexNormals.Size());

        DynArray<Vec4f> bitangents;
        bitangents.Resize(mVertexNormals.Size(), Vec4f::Zero());

        uint32 numTriangles = mVertexIndices.Size() / 3;
        for (uint32 i = 0; i < numTriangles; ++i)
        {
            // algorithm based on: http://www.terathon.com/code/tangent.html
            // (Lengyel�s Method)

            const uint32 i0 = mVertexIndices[3 * i + 0];
            const uint32 i1 = mVertexIndices[3 * i + 1];
            const uint32 i2 = mVertexIndices[3 * i + 2];

            const Vec4f p0(mVertexPositions[i0]);
            const Vec4f p1(mVertexPositions[i1]);
            const Vec4f p2(mVertexPositions[i2]);
            const Vec4f e1 = p1 - p0;
            const Vec4f e2 = p2 - p0;

            const Vec2f& w0 = mVertexTexCoords[i0];
            const Vec2f& w1 = mVertexTexCoords[i1];
            const Vec2f& w2 = mVertexTexCoords[i2];
            const float s1 = w1.x - w0.x;
            const float t1 = w1.y - w0.y;
            const float s2 = w2.x - w0.x;
            const float t2 = w2.y - w0.y;

            const float det = s1 * t2 - s2 * t1;
            if (Abs(det) < 1.0e-10f)
            {
                continue;
            }

            const float r = 1.0f / det;
            const Vec4f sdir = (t2 * e1 - t1 * e2) * r;
            const Vec4f tdir = (s1 * e2 - s2 * e1) * r;

            NFE_ASSERT(sdir.IsValid(), "");
            NFE_ASSERT(tdir.IsValid(), "");

            mVertexTangents[i0] += sdir.ToVec3f();
            mVertexTangents[i1] += sdir.ToVec3f();
            mVertexTangents[i2] += sdir.ToVec3f();

            bitangents[i0] += tdir;
            bitangents[i1] += tdir;
            bitangents[i2] += tdir;
        }

        uint32 numVertices = static_cast<uint32>(mVertexPositions.Size());
        for (uint32 i = 0; i < numVertices; ++i)
        {
            Vec4f tangent(mVertexTangents[i]);
            Vec4f normal(mVertexNormals[i]);
            Vec4f bitangent(bitangents[i]);

            NFE_ASSERT(tangent.IsValid(), "");
            NFE_ASSERT(normal.IsValid(), "");
            NFE_ASSERT(bitangent.IsValid(), "");

            bool tangentIsValid = false;
            if (tangent.SqrLength3() > 0.1f)
            {
                tangent.Normalize3();
                if (Vec4f::Cross3(tangent, normal).SqrLength3() > 0.01f)
                {
                    tangent = Vec4f::Orthogonalize(tangent, normal);
                    tangentIsValid = true;
                }
            }

            if (!tangentIsValid)
            {
                BuildOrthonormalBasis(normal, tangent, bitangent);
            }
            tangent.Normalize3();

            NFE_ASSERT(tangent.IsValid(), "");
            NFE_ASSERT(Abs(Vec4f::Dot3(normal, tangent)) < 0.0001f, "Normal and tangent vectors are not orthogonal");

            // Calculate handedness
            const Vec4f computedBitangent = Vec4f::Cross3(normal, tangent);
            float headedness = Vec4f::Dot3(computedBitangent, bitangent) < 0.0f ? -1.0f : 1.0f;
            (void)headedness; // TODO

            mVertexTangents[i] = tangent.ToVec3f();
        }
    }

    MeshShapePtr BuildMesh()
    {
        MeshDesc meshDesc;
        meshDesc.path = mFilePath;
        meshDesc.vertexBufferDesc.numTriangles = static_cast<uint32>(mVertexIndices.Size() / 3);
        meshDesc.vertexBufferDesc.numVertices = static_cast<uint32>(mVertexPositions.Size());
        meshDesc.vertexBufferDesc.numMaterials = static_cast<uint32>(mMaterialPointers.Size());
        meshDesc.vertexBufferDesc.materials = mMaterialPointers.Data();
        meshDesc.vertexBufferDesc.materialIndexBuffer = mMaterialIndices.Data();
        meshDesc.vertexBufferDesc.vertexIndexBuffer = mVertexIndices.Data();
        meshDesc.vertexBufferDesc.positions = mVertexPositions.Data();
        meshDesc.vertexBufferDesc.normals = mVertexNormals.Data();
        meshDesc.vertexBufferDesc.tangents = mVertexTangents.Data();
        meshDesc.vertexBufferDesc.texCoords = mVertexTexCoords.Data();

        MeshShapePtr mesh = MakeSharedPtr<MeshShape>();
        bool result = mesh->Initialize(meshDesc);
        if (!result)
        {
            return nullptr;
        }

        return mesh;
    }

private:
    String mFilePath;

    DynArray<uint32> mVertexIndices;
    DynArray<uint32> mMaterialIndices;
    DynArray<Vec3f> mVertexPositions;
    DynArray<Vec3f> mVertexNormals;
    DynArray<Vec3f> mVertexTangents;
    DynArray<Vec2f> mVertexTexCoords;
    DynArray<MaterialPtr> mMaterialPointers;
    HashMap<tinyobj::index_t, uint32, TriangleIndicesHash, TriangleIndicesComparator> mUniqueIndices;
};

RT::MeshShapePtr LoadMesh(const String& filePath, MaterialsMap& outMaterials, const float scale)
{
    MeshLoader loader;
    if (!loader.LoadMesh(filePath, outMaterials, scale))
    {
        return nullptr;
    }

    return loader.BuildMesh();
}

} // namespace helpers
} // namespace NFE