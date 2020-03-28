#pragma once

#include "../Raytracer.h"
#include "../../Common/Math/Vec4f.hpp"


namespace NFE {
namespace RT {

const Math::Vec4f DecodeBC1(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vec4f DecodeBC4(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vec4f DecodeBC5(const uint8* data, uint32 x, uint32 y, const uint32 width);

} // namespace RT
} // namespace NFE
