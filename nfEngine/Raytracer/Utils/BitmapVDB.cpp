#include "PCH.h"
#include "Bitmap.h"
#include "../nfCommon/Logger/Logger.hpp"
#include "../nfCommon/Containers/DynArray.hpp"

#include <openvdb/openvdb.h>

namespace NFE {
namespace RT {

using namespace Math;

static bool InitializeOpenVDB()
{
    openvdb::initialize();
    return true;
}

NFE_RAYTRACER_API const bool g_openVdbInitialized = InitializeOpenVDB();

bool Bitmap::LoadVDB(FILE*, const char* path)
{
    openvdb::GridBase::Ptr baseGrid;
    openvdb::CoordBBox box;

    openvdb::io::File file(path);
    if (!file.open())
    {
        return false;
    }

    for (openvdb::io::File::NameIterator nameIter = file.beginName(); nameIter != file.endName(); ++nameIter)
    {
        baseGrid = file.readGrid(nameIter.gridName());
        NFE_LOG_INFO("BitmapVDB: Found grid: %s", nameIter.gridName().c_str());

        box = baseGrid->evalActiveVoxelBoundingBox();
        NFE_LOG_INFO("BitmapVDB: min=[%i,%i,%i], max=[%i,%i,%i]",
            box.min().x(), box.min().y(), box.min().z(),
            box.max().x(), box.max().y(), box.max().z());
    }

    file.close();

    InitData initData;
    initData.width = box.max().x() - box.min().x() + 1;
    initData.height = box.max().y() - box.min().y() + 1;
    initData.depth = box.max().z() - box.min().z() + 1;
    if (initData.width < 1 || initData.height < 1 || initData.depth < 1 ||
        initData.width >= std::numeric_limits<uint16>::max() ||
        initData.height >= std::numeric_limits<uint16>::max() ||
        initData.depth >= std::numeric_limits<uint16>::max())
    {
        NFE_LOG_ERROR("Unsupported VDB format in file '%hs': dimensions are out of bounds (%ux%ux%u)", path, initData.width, initData.height, initData.depth);
        return false;
    }

    if (baseGrid->valueType() == "float")
    {
        initData.format = Format::R8_UNorm;
        initData.linearSpace = true;

        if (!Init(initData))
        {
            return false;
        }

        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

        for (openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn(); iter; ++iter)
        {
            const openvdb::Coord coord = iter.getCoord() - box.min();
            uint8& target = GetPixelRef<uint8>(coord.x(), coord.y(), coord.z());
            target = static_cast<uint8>(Clamp(iter.getValue(), 0.0f, 1.0f) * 255.0f);
        }
    }
    else
    {
        NFE_LOG_ERROR("Unsupported VDB format in file '%hs': %s", path, baseGrid->valueType().c_str());
        return false;
    }

    return true;
}

} // namespace RT
} // namespace NFE
