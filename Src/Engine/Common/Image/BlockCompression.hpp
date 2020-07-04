#pragma once

#include "../../Common/Math/Vec4f.hpp"
#include <memory>

namespace NFE {

using DecodeBCnFunctor = const Math::Vec4f(*)(const uint8*, uint32, uint32, const uint32);
const Math::Vec4f DecodeBC1(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vec4f DecodeBC2(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vec4f DecodeBC3(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vec4f DecodeBC4(const uint8* data, uint32 x, uint32 y, const uint32 width);
const Math::Vec4f DecodeBC5(const uint8* data, uint32 x, uint32 y, const uint32 width);

std::unique_ptr<float[]> DecodeBC1Image(const uint8* data, const uint32 w, const uint32 h);

} // namespace NFE
