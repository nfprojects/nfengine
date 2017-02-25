#pragma once

#include "nfCommon/nfCommon.hpp"
#include "nfCommon/Containers/DynArray.hpp"

// TODO encapsulate these into one common header
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/Types/ReflectionFundamentalType.hpp"
#include "nfCommon/Reflection/Types/ReflectionDynArrayType.hpp"
#include "nfCommon/Reflection/Types/ReflectionUniquePtrType.hpp"
#include "nfCommon/Reflection/Types/ReflectionNativeArrayType.hpp"
#include "nfCommon/Reflection/Types/ReflectionStringType.hpp"


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
    NFE::Common::String strValue;

    TestClassWithFundamentalMembers()
        : intValue(0), floatValue(0.0f), boolValue(false), strValue()
    {}

    TestClassWithFundamentalMembers(NFE::int32 intValue, float floatValue, bool boolValue, const NFE::Common::String& strValue)
        : intValue(intValue), floatValue(floatValue), boolValue(boolValue), strValue(strValue)
    {}
};


class TestClassWithNestedType
{
    NFE_DECLARE_CLASS(TestClassWithNestedType)

public:
    TestClassWithFundamentalMembers foo;
    NFE::int32 bar;
};


class TestClassWithArrayType
{
    NFE_DECLARE_CLASS(TestClassWithArrayType)

public:
    NFE::int32 arrayOfInts[5];
    float foo;
};


class TestClassWithDynArrayType
{
    NFE_DECLARE_CLASS(TestClassWithDynArrayType)

public:
    NFE::Common::DynArray<NFE::int32> arrayOfInts;
    NFE::Common::DynArray<TestClassWithFundamentalMembers> arrayOfObjects;
};


class TestClassWithUniquePtrType
{
    NFE_DECLARE_CLASS(TestClassWithUniquePtrType)

public:
    NFE::Common::UniquePtr<NFE::int32> pointerToInt;
    NFE::Common::UniquePtr<TestClassWithFundamentalMembers> pointerToObject;
    NFE::Common::UniquePtr<NFE::Common::DynArray<NFE::int32>> pointerToArrayOfInts;
    NFE::Common::DynArray<NFE::Common::UniquePtr<NFE::int32>> arrayOfPointersToInts;
};


//////////////////////////////////////////////////////////////////////////


class TestAbstractClass
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TestAbstractClass)

public:
    NFE::int32 intVal;
    float floatVal;
    virtual ~TestAbstractClass() { }
    virtual void PureVirtualMethod() = 0;
};


class TestBaseClass
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TestBaseClass)

public:
    NFE::int32 intVal;
    float floatVal;
    TestBaseClass()
        : mPrivateBool(false)
    {}
    virtual ~TestBaseClass() { }
    bool GetBool() const { return mPrivateBool; };
private:
    bool mPrivateBool;
};


class TestChildClassA : public TestBaseClass
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TestChildClassA)

public:
    NFE::int32 foo;
};

class TestChildClassB : public TestBaseClass
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TestChildClassB)

public:
    float bar;
};


//////////////////////////////////////////////////////////////////////////


namespace helper {

// serialize object to string
bool SerializeObject(const NFE::RTTI::Type* type, const void* object, std::string& outString);

// deserialize object from string
bool DeserializeObject(const NFE::RTTI::Type* type, void* outObject, const std::string& string);

} // namespace helper
