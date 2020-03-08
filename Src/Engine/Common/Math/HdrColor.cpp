#include "PCH.hpp"
#include "HdrColor.hpp"
#include "LdrColor.hpp"
#include "Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_CLASS(NFE::Math::HdrColorRGB)
{
    NFE_CLASS_MEMBER(r);
    NFE_CLASS_MEMBER(g);
    NFE_CLASS_MEMBER(b);
}
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Math {

HdrColorRGB::HdrColorRGB(const LdrColorRGB& color)
{
    r = Convert_sRGB_To_Linear(color.r / 255.0f);
    g = Convert_sRGB_To_Linear(color.g / 255.0f);
    b = Convert_sRGB_To_Linear(color.b / 255.0f);
}

} // namespace Math
} // namespace NFE
