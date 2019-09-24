#pragma once

#include "../nfEngine/Raytracer/Scene/Scene.h"

namespace NFE {
namespace helpers {

bool LoadScene(const Common::String& path, RT::Scene& scene, RT::Camera& camera);

} // namespace helpers
} // namespace NFE