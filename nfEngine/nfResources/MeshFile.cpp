/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Mesh resource file definitions
 */

#include "PCH.hpp"
#include "MeshFile.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/FileSystem.hpp"
#include "nfCommon/File.hpp"


namespace NFE {
namespace Resource {

using namespace Math;

namespace {

// replace backslashes with forward slashes
std::string FixTexturePath(std::string str)
{
    std::replace(str.begin(), str.end(), '\\', '/');
    return str;
}

int8 FloatToInt8(float x)
{
    x *= static_cast<float>(SCHAR_MAX);
    x += 0.5f;

    if (x <= static_cast<float>(SCHAR_MIN)) return SCHAR_MIN;
    if (x >= static_cast<float>(SCHAR_MAX)) return SCHAR_MAX;

    return static_cast<int8>(x);
}

} // namespace


bool MeshFile::Load(const std::string& path)
{
    Common::File file;
    if (!file.Open(path, Common::AccessMode::Read))
    {
        LOG_ERROR("Failed to open file '%s'", path);
        return false;
    }

    MeshHeader header;
    if (file.Read(&header, sizeof(header)) != sizeof(header))
    {
        LOG_ERROR("Failed to read mesh signature");
        return false;
    }

    if (header.magic != NFE_MESH_FILE_MAGIC)
    {
        LOG_ERROR("Invalid signature");
        return false;
    }

    // sanity checks
    if (header.indicesCount == 0 || header.subMeshesCount == 0 || header.verticesCount == 0)
    {
        LOG_ERROR("Corrupted mesh file");
        return false;
    }

    const size_t verticesSize = header.verticesCount * sizeof(MeshVertex);
    const size_t indicesSize = header.indicesCount * sizeof(uint32);
    const size_t subMeshesSize = header.subMeshesCount * sizeof(MeshSubMesh);

    mVertices.resize(header.verticesCount);
    mIndices.resize(header.indicesCount);
    mSubMeshes.resize(header.subMeshesCount);

    if (file.Read(mVertices.data(), verticesSize) != verticesSize)
    {
        LOG_ERROR("Failed to load vertices data");
        return false;
    }

    if (file.Read(mIndices.data(), indicesSize) != indicesSize)
    {
        LOG_ERROR("Failed to load indices data");
        return false;
    }

    if (file.Read(mSubMeshes.data(), subMeshesSize) != subMeshesSize)
    {
        LOG_ERROR("Failed to load submeshes data");
        return false;
    }

    return true;
}

bool MeshFile::Save(const std::string& filePath) const
{
    // TODO use async IO
    Common::File outputFile;
    if (!outputFile.Open(filePath, Common::AccessMode::Write, true))
    {
        LOG_ERROR("Could not open output file '%s'!", filePath.c_str());
        return false;
    }

    MeshHeader header;
    header.magic = NFE_MESH_FILE_MAGIC;
    header.verticesCount = static_cast<uint32>(mVertices.size());
    header.indicesCount = static_cast<uint32>(mIndices.size());
    header.subMeshesCount = static_cast<uint32>(mSubMeshes.size());

    if (!outputFile.Write(&header, sizeof(header)))
        return false;
    if (!outputFile.Write(mVertices.data(), sizeof(MeshVertex) * header.verticesCount))
        return false;
    if (!outputFile.Write(mIndices.data(), sizeof(uint32) * header.indicesCount))
        return false;
    if (!outputFile.Write(mSubMeshes.data(), sizeof(MeshSubMesh) * header.subMeshesCount))
        return false;

    return true;
}

} // namespace Resource
} // namespace NFE
