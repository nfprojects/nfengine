#include "PCH.hpp"
#include "Engine/Common/Containers/SharedPtr.hpp"
#include "Engine/Common/Containers/WeakPtr.hpp"

using namespace NFE;
using namespace NFE::Common;

namespace {

// a test class incrementing external counter on destruction
class TestClass
{
public:
    TestClass(int& counter) : payload(0), mCounter(counter)  { }
    virtual ~TestClass() { mCounter++; }
    std::atomic_int payload;
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


TEST(WeakPtr, Constructor)
{
    const WeakPtr<int> weakPtr;

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0u, weakPtr.WeakRefCount());
}

TEST(WeakPtr, AssignEmptySharedPointer)
{
    const SharedPtr<int> sharedPtr;
    WeakPtr<int> weakPtr;
    weakPtr = sharedPtr;

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0u, weakPtr.WeakRefCount());

    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, sharedPtr);
}

TEST(WeakPtr, ConstructFromEmptySharedPointer)
{
    const SharedPtr<int> sharedPtr;
    const WeakPtr<int> weakPtr(sharedPtr);

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0u, weakPtr.WeakRefCount());

    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, sharedPtr);
}

TEST(WeakPtr, AssignEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    WeakPtr<int> weakPtr2;
    weakPtr2 = weakPtr;

    ASSERT_FALSE(weakPtr2.Valid());
    ASSERT_EQ(0u, weakPtr2.WeakRefCount());
}

TEST(WeakPtr, ConstructFromEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    WeakPtr<int> weakPtr2(weakPtr);

    ASSERT_FALSE(weakPtr2.Valid());
    ASSERT_EQ(0u, weakPtr2.WeakRefCount());

    ASSERT_EQ(weakPtr, weakPtr2);
    ASSERT_EQ(weakPtr2, weakPtr);
}

TEST(WeakPtr, ToWeakPtr)
{
    int counter = 0;
    SharedPtr<TestClass> sharedPtr = MakeUniquePtr<TestClass>(counter);
    const WeakPtr<TestClass> weakPtr(sharedPtr);

    ASSERT_EQ(0, counter);
    ASSERT_TRUE(sharedPtr);
    ASSERT_EQ(1u, sharedPtr.RefCount());
    ASSERT_EQ(2u, sharedPtr.WeakRefCount());
    ASSERT_EQ(2u, weakPtr.WeakRefCount());
    ASSERT_TRUE(weakPtr.Valid());
    ASSERT_EQ(weakPtr, sharedPtr);
    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, weakPtr);

    sharedPtr.Reset();

    ASSERT_EQ(1, counter);
    ASSERT_FALSE(sharedPtr);
    ASSERT_EQ(0u, sharedPtr.RefCount());
    ASSERT_EQ(0u, sharedPtr.WeakRefCount());
    ASSERT_EQ(1u, weakPtr.WeakRefCount());
    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(weakPtr, sharedPtr);
    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, weakPtr);
}

TEST(WeakPtr, Reset)
{
    int counter = 0;
    SharedPtr<TestClass> sharedPtr = MakeUniquePtr<TestClass>(counter);

    WeakPtr<TestClass> weakPtr(sharedPtr);
    weakPtr.Reset();

    ASSERT_EQ(0, counter);
    ASSERT_TRUE(sharedPtr);
    ASSERT_EQ(1u, sharedPtr.RefCount());
    ASSERT_EQ(1u, sharedPtr.WeakRefCount());
    ASSERT_EQ(0u, weakPtr.WeakRefCount());
    ASSERT_FALSE(weakPtr.Valid());
}

TEST(WeakPtr, SharedPointerFromEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    const SharedPtr<int> sharedPtr = weakPtr.Lock();

    ASSERT_FALSE(sharedPtr);
    ASSERT_EQ(0u, sharedPtr.RefCount());
    ASSERT_EQ(0u, weakPtr.WeakRefCount());
}

TEST(WeakPtr, SharedPointerFromValidWeakPointer)
{
    int counter = 0;
    const SharedPtr<TestClass> sharedPtr = MakeUniquePtr<TestClass>(counter);
    const WeakPtr<TestClass> weakPtr(sharedPtr);
    const SharedPtr<TestClass> sharedPtr2 = weakPtr.Lock();

    ASSERT_EQ(0, counter);

    EXPECT_TRUE(sharedPtr);
    EXPECT_EQ(2u, sharedPtr.RefCount());

    EXPECT_TRUE(sharedPtr2);
    EXPECT_EQ(2u, sharedPtr2.RefCount());

    EXPECT_EQ(sharedPtr, sharedPtr2);
    EXPECT_EQ(sharedPtr2, sharedPtr);

    EXPECT_EQ(3u, weakPtr.WeakRefCount());
    EXPECT_TRUE(weakPtr.Valid());
    EXPECT_EQ(sharedPtr2, weakPtr);
    EXPECT_EQ(weakPtr, sharedPtr2);
}

//TEST(SharedPtr, SharedPointerFromWeakPointer_MultiThreaded)
//{
//    uint32 numObjects = 10000;
//
//    std::vector<SharedPtr<uint32>> pointers;
//    std::vector<SharedPtr<uint32>> weakPointers;
//    pointers.reserve(numObjects);
//    weakPointers.reserve(numObjects);
//
//    for (uint32 i = 0; i < numObjects; ++i)
//    {
//        pointers.push_back(MakeSharedPtr<uint32>(0));
//        weakPointers.push_back = pointers.back();
//    }
//
//    const auto accessWeakPointersFunc = [&] ()
//    {
//        for (uint32 i = 0; i < 1000000; ++i)
//        {
//
//        }
//    };
//
//    const auto deletePointersFunc = [&] ()
//    {
//        while (!finish)
//        {
//            PtrType localPointer(globalPointer);
//            localPointer->payload++;
//        }
//    };
//
//    std::thread threadA(func);
//    std::thread threadB(func);
//
//    std::this_thread::sleep_for(std::chrono::milliseconds(400));
//    finish = true;
//
//    threadA.join();
//    threadB.join();
//
//    ASSERT_EQ(0, counter);
//}

TEST(WeakPtr, SharedPointerFromExpiredWeakPointer)
{
    int counter = 0;
    SharedPtr<TestClass> sharedPtr = MakeUniquePtr<TestClass>(counter);
    const WeakPtr<TestClass> weakPtr(sharedPtr);

    ASSERT_EQ(0, counter);
    EXPECT_TRUE(sharedPtr);
    EXPECT_TRUE(weakPtr.Valid());
    EXPECT_EQ(1u, sharedPtr.RefCount());
    EXPECT_EQ(2u, sharedPtr.WeakRefCount());
    EXPECT_EQ(1u, weakPtr.RefCount());
    EXPECT_EQ(2u, weakPtr.WeakRefCount());

    sharedPtr.Reset();

    ASSERT_EQ(1, counter);
    EXPECT_FALSE(sharedPtr);
    EXPECT_FALSE(weakPtr.Valid());
    EXPECT_EQ(0u, sharedPtr.RefCount());
    EXPECT_EQ(0u, sharedPtr.WeakRefCount());
    EXPECT_EQ(0u, weakPtr.RefCount());
    EXPECT_EQ(1u, weakPtr.WeakRefCount());

    const SharedPtr<TestClass> sharedPtr2 = weakPtr.Lock();

    EXPECT_FALSE(weakPtr.Valid());
    EXPECT_FALSE(sharedPtr2);
    EXPECT_EQ(nullptr, sharedPtr2.Get());
    EXPECT_EQ(0u, weakPtr.RefCount());
    EXPECT_EQ(1u, weakPtr.WeakRefCount());
    EXPECT_EQ(0u, sharedPtr2.RefCount());
    EXPECT_EQ(0u, sharedPtr2.WeakRefCount());
}

TEST(WeakPtr, CastToBaseClassViaWeakPtr)
{
    int counter = 0;
    {
        auto childPtr = MakeSharedPtr<ChildTestClass>(counter);
        childPtr->payload = 123;

        // create weak pointer to child class
        WeakPtr<ChildTestClass> childPtrWeak = childPtr;
        ASSERT_EQ(0, counter);
        ASSERT_EQ(1u, childPtr.RefCount());
        ASSERT_EQ(2u, childPtr.WeakRefCount());
        ASSERT_EQ(2u, childPtrWeak.WeakRefCount());

        // cast the weak pointer to base class
        WeakPtr<TestClass> basePtrWeak = childPtrWeak;
        ASSERT_EQ(0, counter);
        ASSERT_EQ(1u, childPtr.RefCount());
        ASSERT_EQ(3u, childPtr.WeakRefCount());
        ASSERT_EQ(3u, childPtrWeak.WeakRefCount());
        ASSERT_EQ(3u, basePtrWeak.WeakRefCount());
        ASSERT_TRUE(childPtr);
        ASSERT_TRUE(basePtrWeak.Valid());

        // convert the weak pointer to strong pointer
        SharedPtr<TestClass> basePtr = basePtrWeak.Lock();
        ASSERT_EQ(0, counter);
        ASSERT_EQ(2u, childPtr.RefCount());
        ASSERT_EQ(4u, childPtr.WeakRefCount());
        ASSERT_EQ(4u, childPtrWeak.WeakRefCount());
        ASSERT_EQ(4u, basePtrWeak.WeakRefCount());
        ASSERT_TRUE(childPtr);
        ASSERT_TRUE(basePtrWeak.Valid());
        ASSERT_EQ(basePtr, basePtrWeak);
        ASSERT_EQ(basePtrWeak, basePtr);

        ASSERT_EQ(123, basePtr->payload);
    }
    ASSERT_EQ(1, counter);
}
