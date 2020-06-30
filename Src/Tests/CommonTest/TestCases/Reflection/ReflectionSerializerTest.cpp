#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/Serializer.hpp"
#include "Engine/Common/Reflection/ReflectionUtils.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Utils/Stream/FileOutputStream.hpp"
#include "Engine/Common/Utils/Stream/FileInputStream.hpp"
#include "Engine/Common/Memory/Buffer.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


namespace {

using TestSharedPtr = SharedPtr<TestBaseClass>;

} // namespace

TEST(ReflectionTest, Serialize_Simple_SingleObject)
{
    Buffer buffer;

    {
        const TestSharedPtr obj = MakeUniquePtr<TestBaseClass>();
        obj->intVal = 123;
        obj->floatVal = 123.0f;

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(Serialize(obj, stream));
        ASSERT_GE(buffer.Size(), 0u);
    }

    {
        BufferInputStream stream(buffer);
        Common::DynArray<ObjectPtr> readObjects;

        ASSERT_TRUE(Deserialize(readObjects, stream));
        ASSERT_EQ(1u, readObjects.Size());

        const TestSharedPtr readObj = Cast<TestBaseClass>(readObjects[0]);

        ASSERT_TRUE(readObj != nullptr);
        ASSERT_EQ(123, readObj->intVal);
        ASSERT_EQ(123.0f, readObj->floatVal);
    }
}

TEST(ReflectionTest, Serialize_CorruptedBuffer)
{
    Buffer originalBuffer;

    {
        const TestSharedPtr obj = MakeUniquePtr<TestBaseClass>();
        obj->intVal = 123;
        obj->floatVal = 123.0f;

        BufferOutputStream stream(originalBuffer);
        ASSERT_TRUE(Serialize(obj, stream));
        ASSERT_GE(originalBuffer.Size(), 0u);
    }

    for (uint32 i = 0; i < 8u * originalBuffer.Size(); ++i)
    {
        Buffer bufferCopy = originalBuffer;

        // flip single bit
        const uint32 byteIndex = i / 8u;
        const uint32 bitIndex = i % 8u;
        reinterpret_cast<uint8*>(bufferCopy.Data())[byteIndex] ^= 1u << bitIndex;

        BufferInputStream stream(bufferCopy);
        Common::DynArray<ObjectPtr> readObjects;

        // not checking if suceeded, just checking if didn't crash
        Deserialize(readObjects, stream);
    }
}

TEST(ReflectionTest, Serialize_Simple_MultipleObject)
{
    Buffer buffer;
    const uint32 numObjects = 10;

    {
        DynArray<ObjectPtr> objects;
        for (uint32 i = 0; i < numObjects; ++i)
        {
            TestSharedPtr obj = MakeUniquePtr<TestBaseClass>();
            obj->intVal = i;
            obj->floatVal = (float)i;
            objects.PushBack(std::move(obj));
        }

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(Serialize(objects, stream));
        ASSERT_GE(buffer.Size(), 0u);
    }

    {
        BufferInputStream stream(buffer);
        Common::DynArray<ObjectPtr> readObjects;

        ASSERT_TRUE(Deserialize(readObjects, stream));
        ASSERT_EQ(numObjects, readObjects.Size());

        for (uint32 i = 0; i < numObjects; ++i)
        {
            const TestSharedPtr readObj = Cast<TestBaseClass>(readObjects[i]);
            ASSERT_TRUE(readObj != nullptr);
            ASSERT_EQ((int32)i, readObj->intVal);
            ASSERT_EQ((float)i, readObj->floatVal);
        }
    }
}

TEST(ReflectionTest, Serialize_Complex_SingleRootObject)
{
    Buffer buffer;

    {
        const SharedPtr<SerializationTestClass> obj = MakeSharedPtr<SerializationTestClass>();
        obj->i32 = 123;
        obj->e = TestEnum::OptionB;

        const SharedPtr<SerializationTestClass> childObj = MakeSharedPtr<SerializationTestClass>();
        obj->sharedPtrA = childObj;
        obj->sharedPtrB = childObj;
        childObj->i32 = 456;
        childObj->e = TestEnum::OptionC;

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(Serialize(obj, stream));
        ASSERT_GE(buffer.Size(), 0u);
    }

    {
        BufferInputStream stream(buffer);
        Common::DynArray<ObjectPtr> readObjects;

        ASSERT_TRUE(Deserialize(readObjects, stream));
        ASSERT_EQ(1u, readObjects.Size());

        const SharedPtr<SerializationTestClass> readObj = Cast<SerializationTestClass>(readObjects[0]);

        ASSERT_TRUE(readObj != nullptr);
        EXPECT_EQ(123, readObj->i32);
        EXPECT_EQ(TestEnum::OptionB, readObj->e);

        ASSERT_TRUE(readObj->sharedPtrA != nullptr);
        EXPECT_TRUE(readObj->sharedPtrA == readObj->sharedPtrB);
        const SharedPtr<SerializationTestClass> readChildObj = Cast<SerializationTestClass>(readObj->sharedPtrA);
        ASSERT_TRUE(readChildObj != nullptr);
        EXPECT_EQ(456, readChildObj->i32);
        EXPECT_EQ(TestEnum::OptionC, readChildObj->e);
    }
}


/*
TEST(ReflectionTest, Serialize_Complex_SingleRootObject_Write)
{
    const SharedPtr<SerializationTestClass> obj = MakeSharedPtr<SerializationTestClass>();
    obj->i32 = 123;
    obj->e = TestEnum::OptionB;

    const SharedPtr<SerializationTestClass> childObj = MakeSharedPtr<SerializationTestClass>();
    obj->sharedPtrA = childObj;
    obj->sharedPtrB = childObj;
    childObj->arrayOfObj[0].intValue = 123;
    childObj->arrayOfObj[1].intValue = 456;

    FileOutputStream stream("a.dat");
    ASSERT_TRUE(Serialize(obj, stream));
}

TEST(ReflectionTest, Serialize_Complex_SingleRootObject_Read)
{
    FileInputStream stream("a.dat");
    Common::DynArray<ObjectPtr> readObjects;

    ASSERT_TRUE(Deserialize(readObjects, stream));
    ASSERT_EQ(1u, readObjects.Size());

    const SharedPtr<SerializationTestClass> readObj = Cast<SerializationTestClass>(readObjects[0]);

    ASSERT_TRUE(readObj != nullptr);
    EXPECT_EQ(123, readObj->i32);
    EXPECT_EQ(TestEnum::OptionB, readObj->e);

    ASSERT_TRUE(readObj->sharedPtrA != nullptr);
    EXPECT_TRUE(readObj->sharedPtrA == readObj->sharedPtrB);
    const SharedPtr<SerializationTestClass> readChildObj = Cast<SerializationTestClass>(readObj->sharedPtrA);
    ASSERT_TRUE(readChildObj != nullptr);
    EXPECT_EQ(123, readChildObj->arrayOfObj[0].intValue);
    EXPECT_EQ(456, readChildObj->arrayOfObj[1].intValue);
}
*/