#pragma once

#include "../Raytracer.h"
#include "../../Common/Math/Vector4.hpp"


namespace NFE {
namespace RT {

const Math::Vector4 DecodeBC1(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vector4 DecodeBC4(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vector4 DecodeBC5(const uint8* data, uint32 x, uint32 y, const uint32 width);

} // namespace RT
} // namespace NFE
