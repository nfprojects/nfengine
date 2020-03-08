#include "PCH.hpp"
#include "Box.hpp"
#include "Transform.hpp"
#include "../Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_CLASS(NFE::Math::Float2)
{
    NFE_CLASS_MEMBER(x);
    NFE_CLASS_MEMBER(y);
}
NFE_END_DEFINE_CLASS()


NFE_DEFINE_CLASS(NFE::Math::Float3)
{
    NFE_CLASS_MEMBER(x);
    NFE_CLASS_MEMBER(y);
    NFE_CLASS_MEMBER(z);
}
NFE_END_DEFINE_CLASS()


NFE_DEFINE_CLASS(NFE::Math::Float4)
{
    NFE_CLASS_MEMBER(x);
    NFE_CLASS_MEMBER(y);
    NFE_CLASS_MEMBER(z);
    NFE_CLASS_MEMBER(w);
}
NFE_END_DEFINE_CLASS()


NFE_DEFINE_CLASS(NFE::Math::Vector4)
{
    NFE_CLASS_MEMBER(x);
    NFE_CLASS_MEMBER(y);
    NFE_CLASS_MEMBER(z);
    NFE_CLASS_MEMBER(w);
}
NFE_END_DEFINE_CLASS()


NFE_DEFINE_CLASS(NFE::Math::Box)
{
    NFE_CLASS_MEMBER(min);
    NFE_CLASS_MEMBER(max);
}
NFE_END_DEFINE_CLASS()


NFE_DEFINE_CLASS(NFE::Math::Quaternion)
{
    NFE_CLASS_MEMBER(i);
    NFE_CLASS_MEMBER(j);
    NFE_CLASS_MEMBER(k);
    NFE_CLASS_MEMBER(r);
}
NFE_END_DEFINE_CLASS()


NFE_DEFINE_CLASS(NFE::Math::Transform)
{
    NFE_CLASS_MEMBER(mTranslation);
    NFE_CLASS_MEMBER(mRotation);
}
NFE_END_DEFINE_CLASS()
