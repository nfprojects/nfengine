#pragma once

#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/Containers/DynArray.hpp"

// TODO encapsulate these into one common header
#include "Engine/Common/Reflection/ReflectionClassDeclare.hpp"
#include "Engine/Common/Reflection/ReflectionEnumMacros.hpp"
#include "Engine/Common/Reflection/Object.hpp"
#include "Engine/Common/Reflection/Types/ReflectionFundamentalType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionDynArrayType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionUniquePtrType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionSharedPtrType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionNativeArrayType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionClassType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionStringType.hpp"


//////////////////////////////////////////////////////////////////////////
// Test types declarations
//////////////////////////////////////////////////////////////////////////

enum class TestEnum : NFE::uint8
{
    OptionA = 0,
    OptionB = 3,
    OptionC = 123,

    UnknownOption = 222,
};

NFE_DECLARE_ENUM_TYPE(TestEnum)

class TestClassWithFundamentalMembers
{
    NFE_DECLARE_CLASS(TestClassWithFundamentalMembers)

public:
    NFE::int32 intValue = 0;
    float floatValue = 0.0f;
    bool boolValue = false;
    NFE::Common::String strValue;

    TestClassWithFundamentalMembers() = default;

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


class TestAbstractClass : public NFE::IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TestAbstractClass)

public:
    NFE::int32 intVal;
    float floatVal;
    virtual ~TestAbstractClass() { }
    virtual void PureVirtualMethod() = 0;
};


class TestBaseClass : public NFE::IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TestBaseClass)

public:
    NFE::int32 intVal;
    float floatVal;
    TestBaseClass()
        : intVal(0)
        , floatVal(0.0f)
        , mPrivateBool(false)
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


class SerializationTestClass : public NFE::IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(SerializationTestClass)

public:
    NFE::int32 i32 = 0;
    float f32 = 0.0f;
    TestEnum e = TestEnum::OptionA;
    TestClassWithFundamentalMembers obj;
    TestClassWithFundamentalMembers arrayOfObj[3];
    NFE::Common::DynArray<TestClassWithFundamentalMembers> dynArrayOfObj;
    NFE::Common::UniquePtr<NFE::IObject> uniquePtr;
    NFE::Common::SharedPtr<NFE::IObject> sharedPtrA;
    NFE::Common::SharedPtr<NFE::IObject> sharedPtrB;
};


//////////////////////////////////////////////////////////////////////////


namespace helper {

// serialize object to string
bool SerializeObject(const NFE::RTTI::Type* type, const void* object, NFE::Common::String& outString);

// serialize object to binary stream
bool SerializeObject(const NFE::RTTI::Type* type, const void* object, NFE::Common::OutputStream& outStream, NFE::RTTI::SerializationContext& context);

// deserialize object from string
bool DeserializeObject(const NFE::RTTI::Type* type, void* outObject, const NFE::Common::StringView& string);

} // namespace helper
