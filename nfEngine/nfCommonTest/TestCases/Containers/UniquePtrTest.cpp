#include "PCH.hpp"
#include "nfCommon/Containers/UniquePtr.hpp"

using namespace NFE::Common;

namespace {

// a test class incrementing external counter on destruction
class TestClass
{
public:
    TestClass(int* counter = nullptr) : mCounter(counter) { }
    ~TestClass()
    {
        if (mCounter)
            (*mCounter)++;
    }

    void SetCounter(int* counter)
    {
        mCounter = counter;
    }

    int payload;
private:
    int* mCounter;
};

class ChildTestClass : public TestClass
{
public:
    ChildTestClass(int* counter = nullptr) : TestClass(counter) { }
};

} // namespace

//////////////////////////////////////////////////////////////////////////

TEST(UniquePtr, Empty)
{
    UniquePtr<int> pointer;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
}

TEST(UniquePtr, Empty_MoveConstructor)
{
    UniquePtr<int> pointer;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);

    UniquePtr<int> pointer2(std::move(pointer));

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(pointer2, nullptr);
    ASSERT_FALSE(pointer2);
}

TEST(UniquePtr, Empty_MoveAssignment)
{
    UniquePtr<int> pointer;
    UniquePtr<int> pointer2;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);

    pointer2 = std::move(pointer);

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(pointer2, nullptr);
    ASSERT_FALSE(pointer2);
}

TEST(UniquePtr, SimpleType)
{
    UniquePtr<int> pointer(new int);
    *pointer = 123;

    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);
    ASSERT_EQ(123, *pointer);
    ASSERT_EQ(123, *pointer.Get());
}

TEST(UniquePtr, Destructor)
{
    int counter = 0;
    {
        UniquePtr<TestClass> pointer(new TestClass(&counter));
        ASSERT_EQ(0, counter);
    }
    ASSERT_EQ(1, counter);
}

TEST(UniquePtr, Reset)
{
    int counter = 0;
    UniquePtr<TestClass> pointer(new TestClass(&counter));

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    // destroy TestClass
    pointer.Reset();

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(1, counter);
}

TEST(UniquePtr, Release)
{
    int counter = 0;
    UniquePtr<TestClass> pointer(new TestClass(&counter));

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    // release - don't destroy TestClass
    TestClass* releasedPtr = pointer.Release();

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0, counter);

    delete releasedPtr;
}

TEST(UniquePtr, MoveConstructor)
{
    int counter = 0;
    UniquePtr<TestClass> pointer(new TestClass(&counter));

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    UniquePtr<TestClass> pointer2(std::move(pointer));

    // first pointer should be cleared
    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);

    ASSERT_NE(pointer2, nullptr);
    ASSERT_TRUE(pointer2);

    ASSERT_EQ(0, counter);
}

TEST(UniquePtr, MoveAssignment)
{
    int counter = 0;
    int counter2 = 0;
    UniquePtr<TestClass> pointer(new TestClass(&counter));
    UniquePtr<TestClass> pointer2(new TestClass(&counter2));
    const TestClass* rawPointer = pointer.Get();

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    ASSERT_EQ(0, counter2);
    ASSERT_NE(pointer2, nullptr);
    ASSERT_TRUE(pointer2);

    pointer2 = std::move(pointer);

    // first pointer should be cleared
    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);

    // second object should be destroyed
    ASSERT_EQ(1, counter2);

    // second pointer should point to the first object
    ASSERT_TRUE(pointer2);
    ASSERT_EQ(pointer2, rawPointer);

    // first object should be untouched
    ASSERT_EQ(0, counter);
}

TEST(UniquePtr, MakeUniquePtr)
{
    int counter = 0;
    {
        auto pointer = MakeUniquePtr<TestClass>(&counter);

        ASSERT_EQ(0, counter);
        ASSERT_NE(pointer, nullptr);
        ASSERT_TRUE(pointer);
    }

    ASSERT_EQ(1, counter);
}

TEST(UniquePtr, CastToBaseClass_Scalar)
{
    int counter = 0;
    {
        UniquePtr<ChildTestClass> childPtr(new ChildTestClass(&counter));

        ASSERT_EQ(0, counter);

        UniquePtr<TestClass> basePtr = childPtr;

        ASSERT_EQ(0, counter);
        ASSERT_FALSE(childPtr);
        ASSERT_TRUE(basePtr);
    }
    ASSERT_EQ(1, counter);
}

TEST(UniquePtr, ArrayType)
{
    int counterA = 0;
    int counterB = 0;

    {
        UniquePtr<TestClass[]> pointer(new TestClass[2]);
        pointer[0].SetCounter(&counterA);
        pointer[1].SetCounter(&counterB);

        ASSERT_NE(pointer, nullptr);
        ASSERT_TRUE(pointer);
    }

    EXPECT_EQ(1, counterA);
    EXPECT_EQ(1, counterB);
}

TEST(UniquePtr, CastToBaseClass_Array)
{
    int counterA = 0;
    int counterB = 0;

    {
        UniquePtr<ChildTestClass[]> childPtrArray(new ChildTestClass[2]);
        childPtrArray[0].SetCounter(&counterA);
        childPtrArray[1].SetCounter(&counterB);

        ASSERT_EQ(0, counterA);
        ASSERT_EQ(0, counterB);

        UniquePtr<TestClass[]> basePtrArray = childPtrArray;

        ASSERT_EQ(0, counterA);
        ASSERT_EQ(0, counterB);
        ASSERT_FALSE(childPtrArray);
        ASSERT_TRUE(basePtrArray);
    }

    EXPECT_EQ(1, counterA);
    EXPECT_EQ(1, counterB);
}