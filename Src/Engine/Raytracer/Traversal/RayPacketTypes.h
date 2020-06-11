#pragma once

#include "../Raytracer.h"

#define NFE_RT_RAY_GROUP_SIZE 16

#include "../../Common/Math/Ray.hpp"
#include "../../Common/Math/SimdRay.hpp"
#include "../../Common/Math/SimdBox.hpp"
#include "../../Common/Math/SimdTriangle.hpp"

#if (NFE_RT_RAY_GROUP_SIZE == 4)
    #include "../../Common/Math/Vec2x4f.hpp"
    #include "../../Common/Math/Vec3x4f.hpp"
    #include "../../Common/Math/Vec4i.hpp"
#elif (NFE_RT_RAY_GROUP_SIZE == 8)
    #include "../../Common/Math/Vec2x8f.hpp"
    #include "../../Common/Math/Vec3x8f.hpp"
    #include "../../Common/Math/Vec8i.hpp"
#elif (NFE_RT_RAY_GROUP_SIZE == 16)
    #include "../../Common/Math/Vec2x16f.hpp"
    #include "../../Common/Math/Vec3x16f.hpp"
    #include "../../Common/Math/Vec16i.hpp"
#else
    #error Unsupported ray group size
#endif

namespace NFE {
namespace RT {

struct RayPacketTypes
{
    static constexpr uint32 GroupSize = NFE_RT_RAY_GROUP_SIZE;

#if (NFE_RT_RAY_GROUP_SIZE == 4)

    using RayMaskType = uint8;
    using Float = Math::Vec4f;
    using FloatMask = Math::VecBool4f;
    using Int32 = Math::Vec4i;
    using Uint32 = Math::Vec4ui;
    using Vec2f = Math::Vec2x4f;
    using Vec3f = Math::Vec3x4f;

#elif (NFE_RT_RAY_GROUP_SIZE == 8)

    using RayMaskType = uint8;
    using Float = Math::Vec8f;
    using FloatMask = Math::VecBool8f;
    using Int32 = Math::Vec8i;
    using Uint32 = Math::Vec8ui;
    using Vec2f = Math::Vec2x8f;
    using Vec3f = Math::Vec3x8f;

#elif (NFE_RT_RAY_GROUP_SIZE == 16)

    using RayMaskType = uint16;
    using Float = Math::Vec16f;
    using FloatMask = Math::VecBool16f;
    using Int32 = Math::Vec16i;
    using Uint32 = Math::Vec16ui;
    using Vec2f = Math::Vec2x16f;
    using Vec3f = Math::Vec3x16f;

#else
    #error Unsupported ray group size
#endif

    using Ray = Math::SimdRay<Vec3f>;
    using Box = Math::SimdBox<Vec3f>;
    using Triangle = Math::SimdTriangle<Vec3f>;
};


} // namespace RT
} // namespace NFE
