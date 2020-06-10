#pragma once

#include "Engine/Raytracer/Shapes/MeshShape.h"
#include "Engine/Raytracer/Material/Material.h"
#include "Engine/Raytracer/Utils/Bitmap.h"
#include "Engine/Common/Containers/HashMap.hpp"

namespace NFE {
namespace helpers {

using MaterialsMap = Common::HashMap<Common::String, RT::MaterialPtr>;

RT::BitmapPtr LoadBitmapObject(const Common::StringView& baseDir, const Common::StringView& path);
RT::TexturePtr LoadTexture(const Common::StringView& baseDir, const Common::StringView& path);
RT::MeshShapePtr LoadMesh(const Common::String& filePath, MaterialsMap& outMaterials, const float scale = 1.0f);
RT::MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials);

} // namespace helpers
} // namespace NFE