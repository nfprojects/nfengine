#pragma once

#include "nfCommon/nfCommon.hpp"
#include "nfCommon/Containers/DynArray.hpp"

// TODO encapsulate these into one common header
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/Types/ReflectionFundamentalType.hpp"
#include "nfCommon/Reflection/Types/ReflectionDynArrayType.hpp"
#include "nfCommon/Reflection/Types/ReflectionUniquePtr.hpp"

#include <string>


//////////////////////////////////////////////////////////////////////////
// Test types declarations
//////////////////////////////////////////////////////////////////////////

class TestClassWithFundamentalMembers
{
    NFE_DECLARE_CLASS(TestClassWithFundamentalMembers)

public:
    NFE::int32 intValue;
    float floatValue;
    bool boolValue;

    TestClassWithFundamentalMembers()
        : intValue(0), floatValue(0.0f), boolValue(false)
    {}

    TestClassWithFundamentalMembers(NFE::int32 intValue, float floatValue, bool boolValue)
        : intValue(intValue), floatValue(floatValue), boolValue(boolValue)
    {}
};

//////////////////////////////////////////////////////////////////////////

class TestClassWithNestedType
{
    NFE_DECLARE_CLASS(TestClassWithNestedType)

public:
    TestClassWithFundamentalMembers foo;
    NFE::int32 bar;
};

//////////////////////////////////////////////////////////////////////////

class TestClassWithArrayType
{
    NFE_DECLARE_CLASS(TestClassWithArrayType)

public:
    NFE::int32 array[5];
    float foo;
};

//////////////////////////////////////////////////////////////////////////

class TestClassWithDynArrayType
{
    NFE_DECLARE_CLASS(TestClassWithDynArrayType)

public:
    NFE::Common::DynArray<NFE::int32> arrayOfInts;
    NFE::Common::DynArray<TestClassWithFundamentalMembers> arrayOfObjects;
};

//////////////////////////////////////////////////////////////////////////

class TestClassWithUniquePtrType
{
    NFE_DECLARE_CLASS(TestClassWithUniquePtrType)

public:
    std::unique_ptr<NFE::int32> pointerToInt;
    std::unique_ptr<TestClassWithFundamentalMembers> pointerToObject;
    std::unique_ptr<NFE::Common::DynArray<NFE::int32>> pointerToArrayOfInts;
    NFE::Common::DynArray<std::unique_ptr<NFE::int32>> arrayOfPointersToInts;
};

//////////////////////////////////////////////////////////////////////////


namespace helper {

// serialize object to string
void SerializeObject(const NFE::RTTI::Type* type, const void* object, std::string& outString);

// deserialize object from string
void DeserializeObject(const NFE::RTTI::Type* type, void* outObject, const std::string& string);

} // namespace helper
