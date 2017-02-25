/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of test types.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "../ReflectionMacros.hpp"
#include "../Types/ReflectionFundamentalType.hpp"


namespace NFE {
namespace RTTI {


class InternalTestClass
{
    NFE_DECLARE_CLASS(InternalTestClass)

public:
    int32 foo;
};

class InternalTestClassWithNestedObject
{
    NFE_DECLARE_CLASS(InternalTestClassWithNestedObject)

public:
    InternalTestClass foo;
};

} // namespace RTTI
} // namespace NFE
