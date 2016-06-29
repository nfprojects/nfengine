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

bool MeshFile::Load(Common::InputStream& stream)
{
    MeshHeader header;
    if (stream.Read(&header, sizeof(header)) != sizeof(header))
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

    const size_t verticesSize = header.verticesCount * sizeof(mVertices[0]);
    const size_t indicesSize = header.indicesCount * sizeof(mIndices[0]);
    const size_t subMeshesSize = header.subMeshesCount * sizeof(mSubMeshes[0]);

    mVertices.resize(header.verticesCount);
    mIndices.resize(header.indicesCount);
    mSubMeshes.resize(header.subMeshesCount);

    if (stream.Read(mVertices.data(), verticesSize) != verticesSize)
    {
        LOG_ERROR("Failed to load vertices data");
        return false;
    }

    if (stream.Read(mIndices.data(), indicesSize) != indicesSize)
    {
        LOG_ERROR("Failed to load indices data");
        return false;
    }

    if (stream.Read(mSubMeshes.data(), subMeshesSize) != subMeshesSize)
    {
        LOG_ERROR("Failed to load submeshes data");
        return false;
    }

    return true;
}

bool MeshFile::Save(Common::OutputStream& stream) const
{
    MeshHeader header;
    header.magic = NFE_MESH_FILE_MAGIC;
    header.verticesCount = static_cast<uint32>(mVertices.size());
    header.indicesCount = static_cast<uint32>(mIndices.size());
    header.subMeshesCount = static_cast<uint32>(mSubMeshes.size());

    if (!stream.Write(&header, sizeof(header)))
        return false;
    if (!stream.Write(mVertices.data(), sizeof(mVertices[0]) * header.verticesCount))
        return false;
    if (!stream.Write(mIndices.data(), sizeof(mIndices[0]) * header.indicesCount))
        return false;
    if (!stream.Write(mSubMeshes.data(), sizeof(mSubMeshes[0]) * header.subMeshesCount))
        return false;

    return true;
}

} // namespace Resource
} // namespace NFE
