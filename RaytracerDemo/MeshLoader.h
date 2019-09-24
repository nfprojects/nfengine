#pragma once

#include "../nfEngine/Raytracer/Shapes/MeshShape.h"
#include "../nfEngine/Raytracer/Material/Material.h"
#include "../nfEngine/Raytracer/Utils/Bitmap.h"
#include "../nfEngine/nfCommon/Containers/HashMap.hpp"

namespace NFE {
namespace helpers {

using MaterialsMap = Common::HashMap<Common::String, RT::MaterialPtr>;

RT::BitmapPtr LoadBitmapObject(const Common::String& baseDir, const Common::String& path);
RT::TexturePtr LoadTexture(const Common::String& baseDir, const Common::String& path);
RT::MeshShapePtr LoadMesh(const Common::String& filePath, MaterialsMap& outMaterials, const float scale = 1.0f);
RT::MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials);

} // namespace helpers
} // namespace NFE