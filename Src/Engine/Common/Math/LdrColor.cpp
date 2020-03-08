#include "PCH.hpp"
#include "LdrColor.hpp"
#include "../Reflection/ReflectionClassDefine.hpp"

NFE_DEFINE_CLASS(NFE::Math::LdrColorRGB)
{
    NFE_CLASS_MEMBER(r);
    NFE_CLASS_MEMBER(g);
    NFE_CLASS_MEMBER(b);
}
NFE_END_DEFINE_CLASS()

NFE_DEFINE_CLASS(NFE::Math::LdrColorRGBA)
{
    NFE_CLASS_MEMBER(r);
    NFE_CLASS_MEMBER(g);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(a);
}
NFE_END_DEFINE_CLASS()
