#include "PCH.hpp"
#include "Engine/Common/Containers/SharedPtr.hpp"
#include "Engine/Common/System/Thread.hpp"


using namespace NFE::Common;

namespace {

// a test class incrementing external counter on destruction
class TestClass
{
public:
    TestClass(int& counter) : mCounter(counter)  { }
    virtual ~TestClass() { mCounter++; }
    std::atomic_int payload = 0;
    bool* deleted = nullptr;
private:
    int& mCounter;
};

class ChildTestClass : public virtual TestClass
{
public:
    ChildTestClass(int& counter) : TestClass(counter) { }
    virtual ~ChildTestClass() { }
};

} // namespace


//////////////////////////////////////////////////////////////////////////
// Single reference tests - SharedPtr used as UniquePtr
//////////////////////////////////////////////////////////////////////////

TEST(SharedPtr, Empty_SingleRef)
{
    SharedPtr<int> pointer;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0u, pointer.RefCount());
}

TEST(SharedPtr, Empty_MoveConstructor_SingleRef)
{
    SharedPtr<int> pointer;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0u, pointer.RefCount());

    SharedPtr<int> pointer2(std::move(pointer));

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0u, pointer.RefCount());

    ASSERT_EQ(pointer2, nullptr);
    ASSERT_FALSE(pointer2);
    ASSERT_EQ(0u, pointer.RefCount());
}

TEST(SharedPtr, Empty_MoveAssignment_SingleRef)
{
    SharedPtr<int> pointer;
    SharedPtr<int> pointer2;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0u, pointer.RefCount());

    pointer2 = std::move(pointer);

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0u, pointer.RefCount());

    ASSERT_EQ(pointer2, nullptr);
    ASSERT_FALSE(pointer2);
    ASSERT_EQ(0u, pointer.RefCount());
}

TEST(SharedPtr, SimpleType_SingleRef)
{
    SharedPtr<int> pointer = MakeSharedPtr<int>();
    *pointer = 123;

    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);
    ASSERT_EQ(123, *pointer);
    ASSERT_EQ(123, *pointer.Get());
    ASSERT_EQ(1u, pointer.RefCount());
}

TEST(SharedPtr, Destructor_SingleRef)
{
    int counter = 0;
    {
        SharedPtr<TestClass> pointer = MakeSharedPtr<TestClass>(counter);
        ASSERT_EQ(0, counter);
    }
    ASSERT_EQ(1, counter);
}

TEST(SharedPtr, Reset_SingleRef)
{
    int counter = 0;
    SharedPtr<TestClass> pointer = MakeSharedPtr<TestClass>(counter);

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);
    ASSERT_EQ(1u, pointer.RefCount());

    // destroy TestClass
    pointer.Reset();

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(1, counter);
    ASSERT_EQ(0u, pointer.RefCount());
}

TEST(SharedPtr, MoveConstructor_UniquePtr)
{
    int counter = 0;
    UniquePtr<TestClass> pointer = MakeUniquePtr<TestClass>(counter);

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    SharedPtr<TestClass> pointer2(std::move(pointer));

    // first pointer should be cleared
    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);

    ASSERT_NE(pointer2, nullptr);
    ASSERT_TRUE(pointer2);

    ASSERT_EQ(0, counter);
}

TEST(SharedPtr, MoveConstructor_UniquePtr_CustomDestructor)
{
    bool deleted = false;

    struct TestDeleter
    {
        static void Delete(TestClass* pointer)
        {
            if (pointer)
            {
                *(pointer->deleted) = true;
                pointer->~TestClass();
                NFE_FREE(pointer);
            }
        }
    };

    int counter = 0;
    UniquePtr<TestClass,TestDeleter> pointer = MakeUniquePtr<TestClass,TestDeleter>(counter);
    pointer->deleted = &deleted;

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    SharedPtr<TestClass> pointer2(std::move(pointer));

    // first pointer should be cleared
    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);

    // the object should not be yet deleted
    ASSERT_FALSE(deleted);

    ASSERT_NE(pointer2, nullptr);
    ASSERT_TRUE(pointer2);

    ASSERT_EQ(0, counter);

    pointer2.Reset();

    // now the object should be deleted
    ASSERT_TRUE(deleted);
}

TEST(SharedPtr, MoveConstructor_SingleRef)
{
    int counter = 0;
    SharedPtr<TestClass> pointer = MakeSharedPtr<TestClass>(counter);

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    SharedPtr<TestClass> pointer2(std::move(pointer));

    // first pointer should be cleared
    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);

    ASSERT_NE(pointer2, nullptr);
    ASSERT_TRUE(pointer2);

    ASSERT_EQ(0, counter);
}

TEST(SharedPtr, MoveAssignment_SingleRef)
{
    int counter = 0;
    int counter2 = 0;
    SharedPtr<TestClass> pointer = MakeSharedPtr<TestClass>(counter);
    SharedPtr<TestClass> pointer2 = MakeSharedPtr<TestClass>(counter2);
    const TestClass* rawPointer = pointer.Get();

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);

    ASSERT_EQ(0, counter2);
    ASSERT_NE(pointer2, nullptr);
    ASSERT_TRUE(pointer2);

    ASSERT_FALSE(pointer == pointer2);
    ASSERT_TRUE(pointer != pointer2);

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

//////////////////////////////////////////////////////////////////////////
// Serious SharedPtr tests
//////////////////////////////////////////////////////////////////////////

TEST(SharedPtr, CopyConstructor)
{
    int counter = 0;
    SharedPtr<TestClass> pointerA = MakeSharedPtr<TestClass>(counter);

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(1u, pointerA.RefCount());

    SharedPtr<TestClass> pointerB(pointerA);

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(2u, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(2u, pointerB.RefCount());
    ASSERT_EQ(pointerA.Get(), pointerB.Get());

    // remove original reference
    pointerA.Reset();

    ASSERT_EQ(0, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0u, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(1u, pointerB.RefCount());

    // remove second reference, object should be destroyed
    pointerB.Reset();

    ASSERT_EQ(1, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0u, pointerA.RefCount());
    ASSERT_EQ(pointerB, nullptr);
    ASSERT_FALSE(pointerB);
    ASSERT_EQ(0u, pointerA.RefCount());
}

TEST(SharedPtr, Assignment)
{
    int counter = 0;
    SharedPtr<TestClass> pointerA = MakeSharedPtr<TestClass>(counter);
    SharedPtr<TestClass> pointerB;

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(1u, pointerA.RefCount());
    ASSERT_TRUE(pointerA != pointerB);
    ASSERT_FALSE(pointerA == pointerB);

    pointerB = pointerA;

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(2u, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(2u, pointerB.RefCount());
    ASSERT_EQ(pointerA.Get(), pointerB.Get());
    ASSERT_FALSE(pointerA != pointerB);
    ASSERT_TRUE(pointerA == pointerB);

    // remove original reference
    pointerA.Reset();

    ASSERT_EQ(0, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0u, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(1u, pointerB.RefCount());
    ASSERT_TRUE(pointerA != pointerB);
    ASSERT_FALSE(pointerA == pointerB);

    // remove second reference, object should be destroyed
    pointerB.Reset();

    ASSERT_EQ(1, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0u, pointerA.RefCount());
    ASSERT_EQ(pointerB, nullptr);
    ASSERT_FALSE(pointerB);
    ASSERT_EQ(0u, pointerA.RefCount());
    ASSERT_FALSE(pointerA != pointerB);
    ASSERT_TRUE(pointerA == pointerB);
}

TEST(SharedPtr, CopyConstructor_Nullptr)
{
    SharedPtr<TestClass> pointerA;
    SharedPtr<TestClass> pointerB(pointerA);

    pointerB = pointerA;

    EXPECT_EQ(0u, pointerA.RefCount());
    EXPECT_EQ(0u, pointerB.RefCount());
}

TEST(SharedPtr, MoveConstructor_Nullptr)
{
    SharedPtr<TestClass> pointerA;
    SharedPtr<TestClass> pointerB(std::move(pointerA));

    pointerB = std::move(pointerA);

    EXPECT_EQ(0u, pointerA.RefCount());
    EXPECT_EQ(0u, pointerB.RefCount());
}

TEST(SharedPtr, Assignment_Nullptr)
{
    SharedPtr<TestClass> pointerA;
    SharedPtr<TestClass> pointerB;

    pointerB = pointerA;

    EXPECT_EQ(0u, pointerA.RefCount());
    EXPECT_EQ(0u, pointerB.RefCount());
}

TEST(SharedPtr, MoveAssignment_Nullptr)
{
    SharedPtr<TestClass> pointerA;
    SharedPtr<TestClass> pointerB;

    pointerB = std::move(pointerA);

    EXPECT_EQ(0u, pointerA.RefCount());
    EXPECT_EQ(0u, pointerB.RefCount());
}

TEST(SharedPtr, CopyConstructor_MultiThreaded)
{
    using PtrType = SharedPtr<TestClass>;

    int counter = 0;
    PtrType globalPointer = MakeSharedPtr<TestClass>(counter);
    std::atomic_bool finish(false);

    const auto func = [&finish, &globalPointer]()
    {
        while (!finish)
        {
            PtrType localPointer(globalPointer);
            localPointer->payload++;
        }
    };

    Thread threadA, threadB;

    ASSERT_TRUE(threadA.Run(func));
    ASSERT_TRUE(threadB.Run(func));

    Thread::SleepCurrentThread(0.4);
    finish = true;

    threadA.Wait();
    threadB.Wait();

    ASSERT_EQ(0, counter);
}

TEST(SharedPtr, CustomDeleter)
{
    int customDeleterCalled = 0;

    const auto deleter = [&customDeleterCalled](TestClass* object)
    {
        customDeleterCalled++;
        delete object;
    };

    using PtrType = SharedPtr<TestClass>;

    int counter = 0;
    {
        PtrType globalPointer(new TestClass(counter), deleter);
        ASSERT_EQ(0, counter);
        ASSERT_EQ(0, customDeleterCalled);
    }

    ASSERT_EQ(1, counter);
    ASSERT_EQ(1, customDeleterCalled);
}

TEST(SharedPtr, CastToBaseClass)
{
    int counter = 0;
    {
        auto childPtr = MakeSharedPtr<ChildTestClass>(counter);

        ASSERT_EQ(0, counter);
        ASSERT_EQ(1u, childPtr.RefCount());

        SharedPtr<TestClass> basePtr = childPtr;

        ASSERT_EQ(0, counter);
        ASSERT_EQ(2u, childPtr.RefCount());
        ASSERT_EQ(2u, basePtr.RefCount());
        ASSERT_TRUE(childPtr);
        ASSERT_TRUE(basePtr);

        childPtr.Reset();

        ASSERT_EQ(0, counter);
        ASSERT_EQ(0u, childPtr.RefCount());
        ASSERT_EQ(1u, basePtr.RefCount());
        ASSERT_FALSE(childPtr);
        ASSERT_TRUE(basePtr);
    }
    ASSERT_EQ(1, counter);
}

TEST(SharedPtr, CastToBaseClass_Reference)
{
    int counter = 0;
    {
        auto childPtr = MakeSharedPtr<ChildTestClass>(counter);
        const SharedPtr<ChildTestClass>& childPtrRef = childPtr;

        ASSERT_EQ(0, counter);
        ASSERT_EQ(1u, childPtr.RefCount());

        const SharedPtr<TestClass>& basePtr = childPtrRef;

        ASSERT_EQ(0, counter);
        ASSERT_EQ(2u, childPtr.RefCount());
        ASSERT_EQ(2u, basePtr.RefCount());
        ASSERT_TRUE(childPtr);
        ASSERT_TRUE(basePtr);

        childPtr.Reset();

        ASSERT_EQ(0, counter);
        ASSERT_EQ(0u, childPtr.RefCount());
        ASSERT_EQ(1u, basePtr.RefCount());
        ASSERT_FALSE(childPtr);
        ASSERT_TRUE(basePtr);
    }
    ASSERT_EQ(1, counter);
}
