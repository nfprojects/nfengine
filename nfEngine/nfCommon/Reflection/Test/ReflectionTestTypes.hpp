/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of test types.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "../ReflectionClassMacros.hpp"
#include "../ReflectionEnumMacros.hpp"


namespace NFE {
namespace RTTI {


class InternalTestClass
{
    NFE_DECLARE_CLASS(InternalTestClass)

public:
    bool b = false;
    int8 i8 = 0;
    int16 i16 = 0;
    int32 i32 = 0;
    int64 i64 = 0;
    uint8 u8 = 0;
    uint16 u16 = 0;
    uint32 u32 = 0;
    uint64 u64 = 0;
    float f = 0.0f;
    double d = 0.0;
};

enum class InternalTestEnum : uint8
{
    OptionA,
    OptionB,
    OptionC,
};

class InternalTestClassWithNestedObject
{
    NFE_DECLARE_CLASS(InternalTestClassWithNestedObject)

public:
    InternalTestClass object;
    InternalTestEnum enumVar = InternalTestEnum::OptionA;
    bool nativeArrayOfBools[5];
    int32 nativeArrayOfInt32[5];
    InternalTestClass nativeArrayOfObjects[5];
};

} // namespace RTTI
} // namespace NFE

NFE_DECLARE_ENUM_TYPE(NFE::RTTI::InternalTestEnum);
