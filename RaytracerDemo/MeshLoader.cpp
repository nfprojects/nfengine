#include "PCH.h"
#include "MeshLoader.h"

#include "../nfEngine/Raytracer/Utils/Bitmap.h"
#include "../nfEngine/Raytracer/Textures/BitmapTexture.h"
#include "../nfEngine/nfCommon/Logger/Logger.hpp"
#include "../nfEngine/nfCommon/System/Timer.hpp"
#include "../nfEngine/nfCommon/Math/Geometry.hpp"
#include "../nfEngine/nfCommon/FileSystem/FileSystem.hpp"

#include "../nfEngineDeps/tinyobjloader/tiny_obj_loader.h"


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

BitmapPtr LoadBitmapObject(const String& baseDir, const String& path)
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
        bitmapPtr = MakeSharedPtr<Bitmap>(path.Str());
        if (!bitmapPtr->Load(fullPath.Str()))
        {
            return nullptr;
        }
    }

    return bitmapPtr;
}

TexturePtr LoadTexture(const String& baseDir, const String& path)
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

MaterialPtr LoadMaterial(const String& baseDir, const tinyobj::material_t& sourceMaterial)
{
    auto material = MaterialPtr(new Material);

    material->SetBsdf("diffuse"); // TODO
    material->debugName = sourceMaterial.name.c_str();
    material->baseColor = HdrColorRGB(sourceMaterial.diffuse[0], sourceMaterial.diffuse[1], sourceMaterial.diffuse[2]);
    material->emission = HdrColorRGB(sourceMaterial.emission[0], sourceMaterial.emission[1], sourceMaterial.emission[2]);
    material->baseColor.texture = LoadTexture(baseDir, sourceMaterial.diffuse_texname.c_str());
    material->normalMap = LoadTexture(baseDir, sourceMaterial.normal_texname.c_str());
    material->maskMap = LoadTexture(baseDir, sourceMaterial.alpha_texname.c_str());
    material->roughness = 0.075f;

    material->Compile();

    return material;
}

MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials)
{
    auto material = MaterialPtr(new Material);
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

                Vector4 verts[3];
                for (size_t i = 0; i < 3; i++)
                {
                    verts[i] = scale * Vector4(
                        attrib.vertices[3 * idx[i].vertex_index + 0],
                        attrib.vertices[3 * idx[i].vertex_index + 1],
                        attrib.vertices[3 * idx[i].vertex_index + 2]);
                }

                // discard degenerate triangles
                const Vector4 edge1 = verts[1] - verts[0];
                const Vector4 edge2 = verts[2] - verts[0];
                const Vector4 edge3 = verts[2] - verts[1];
                if (edge1.SqrLength3() < MinEdgeLengthSqr ||
                    edge2.SqrLength3() < MinEdgeLengthSqr ||
                    edge3.SqrLength3() < MinEdgeLengthSqr ||
                    TriangleSurfaceArea(edge1, edge2) < MinEdgeLengthSqr)
                {
                    NFE_LOG_WARNING("Mesh has degenerate triangle (shape index = %zu, face index = %zu)", shapeIndex, faceIndex);
                    continue;
                }

                // compute per-face normal
                const Vector4 faceNormal = Vector4::Cross3(verts[1] - verts[0], verts[2] - verts[0]).Normalized3();
                NFE_ASSERT(faceNormal.IsValid());

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

                        mVertexPositions.PushBack(verts[i].ToFloat3());

                        if (hasNormals)
                        {
                            const Vector4 normal(
                                attrib.normals[3 * indices.normal_index + 0],
                                attrib.normals[3 * indices.normal_index + 1],
                                attrib.normals[3 * indices.normal_index + 2]);
                            mVertexNormals.PushBack(normal.Normalized3().ToFloat3());
                        }
                        else
                        {
                            // fallback to face normal
                            // TODO smooth shading
                            mVertexNormals.PushBack(faceNormal.ToFloat3());
                        }

                        if (hasTexCoords)
                        {
                            mVertexTexCoords.PushBack(Float2(attrib.texcoords.data() + 2 * idx[i].texcoord_index));
                        }
                        else
                        {
                            mVertexTexCoords.PushBack(Float2());
                        }
                    }

                    mVertexIndices.PushBack(uniqueIndex);
                }

                mMaterialIndices.PushBack(mesh.material_ids[faceIndex]);
            }
        }

        NFE_ASSERT(mVertexPositions.Size() == mUniqueIndices.Size());
        NFE_ASSERT(mVertexPositions.Size() == mVertexNormals.Size());
        NFE_ASSERT(mVertexPositions.Size() == mVertexTexCoords.Size());

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

        DynArray<Vector4> bitangents;
        bitangents.Resize(mVertexNormals.Size());
        memset(bitangents.Data(), 0, bitangents.Size() * sizeof(Vector4));

        uint32 numTriangles = mVertexIndices.Size() / 3;
        for (uint32 i = 0; i < numTriangles; ++i)
        {
            // algorithm based on: http://www.terathon.com/code/tangent.html
            // (Lengyel�s Method)

            const uint32 i0 = mVertexIndices[3 * i + 0];
            const uint32 i1 = mVertexIndices[3 * i + 1];
            const uint32 i2 = mVertexIndices[3 * i + 2];

            const Vector4 p0(mVertexPositions[i0]);
            const Vector4 p1(mVertexPositions[i1]);
            const Vector4 p2(mVertexPositions[i2]);
            const Vector4 e1 = p1 - p0;
            const Vector4 e2 = p2 - p0;

            const Float2& w0 = mVertexTexCoords[i0];
            const Float2& w1 = mVertexTexCoords[i1];
            const Float2& w2 = mVertexTexCoords[i2];
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
            const Vector4 sdir = (t2 * e1 - t1 * e2) * r;
            const Vector4 tdir = (s1 * e2 - s2 * e1) * r;

            NFE_ASSERT(sdir.IsValid());
            NFE_ASSERT(tdir.IsValid());

            mVertexTangents[i0] += sdir.ToFloat3();
            mVertexTangents[i1] += sdir.ToFloat3();
            mVertexTangents[i2] += sdir.ToFloat3();

            bitangents[i0] += tdir;
            bitangents[i1] += tdir;
            bitangents[i2] += tdir;
        }

        uint32 numVertices = static_cast<uint32>(mVertexPositions.Size());
        for (uint32 i = 0; i < numVertices; ++i)
        {
            Vector4 tangent(mVertexTangents[i]);
            Vector4 normal(mVertexNormals[i]);
            Vector4 bitangent(bitangents[i]);

            NFE_ASSERT(tangent.IsValid());
            NFE_ASSERT(normal.IsValid());
            NFE_ASSERT(bitangent.IsValid());

            bool tangentIsValid = false;
            if (tangent.SqrLength3() > 0.1f)
            {
                tangent.Normalize3();
                if (Vector4::Cross3(tangent, normal).SqrLength3() > 0.01f)
                {
                    tangent = Vector4::Orthogonalize(tangent, normal);
                    tangentIsValid = true;
                }
            }

            if (!tangentIsValid)
            {
                BuildOrthonormalBasis(normal, tangent, bitangent);
            }
            tangent.Normalize3();

            NFE_ASSERT(tangent.IsValid());
            NFE_ASSERT(Abs(Vector4::Dot3(normal, tangent)) < 0.0001f, "Normal and tangent vectors are not orthogonal");

            // Calculate handedness
            const Vector4 computedBitangent = Vector4::Cross3(normal, tangent);
            float headedness = Vector4::Dot3(computedBitangent, bitangent) < 0.0f ? -1.0f : 1.0f;
            (void)headedness; // TODO

            mVertexTangents[i] = tangent.ToFloat3();
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

        MeshShapePtr mesh = MeshShapePtr(new MeshShape);
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
    DynArray<Float3> mVertexPositions;
    DynArray<Float3> mVertexNormals;
    DynArray<Float3> mVertexTangents;
    DynArray<Float2> mVertexTexCoords;
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