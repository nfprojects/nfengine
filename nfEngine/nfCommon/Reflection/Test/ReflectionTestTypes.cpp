#include "PCH.hpp"
#include "ReflectionTestTypes.hpp"
#include "../Types/ReflectionNativeArrayType.hpp"

NFE_BEGIN_DEFINE_CLASS(NFE::RTTI::InternalTestClass)
{
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(i8);
    NFE_CLASS_MEMBER(i16);
    NFE_CLASS_MEMBER(i32);
    NFE_CLASS_MEMBER(i64);
    NFE_CLASS_MEMBER(u8);
    NFE_CLASS_MEMBER(u16);
    NFE_CLASS_MEMBER(u32);
    NFE_CLASS_MEMBER(u64);
    NFE_CLASS_MEMBER(f);
    NFE_CLASS_MEMBER(d);
}
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_CLASS(NFE::RTTI::InternalTestClassWithNestedObject)
{
    NFE_CLASS_MEMBER(object);
    NFE_CLASS_MEMBER(enumVar);
    NFE_CLASS_MEMBER(nativeArrayOfBools);
    NFE_CLASS_MEMBER(nativeArrayOfInt32);
    NFE_CLASS_MEMBER(nativeArrayOfObjects);
}
NFE_END_DEFINE_CLASS()

NFE_BEGIN_DEFINE_ENUM(NFE::RTTI::InternalTestEnum)
{
    NFE_ENUM_OPTION(OptionA);
    NFE_ENUM_OPTION(OptionB);
    NFE_ENUM_OPTION(OptionC);
}
NFE_END_DEFINE_ENUM()