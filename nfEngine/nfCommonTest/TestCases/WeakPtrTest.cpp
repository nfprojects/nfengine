#include "PCH.hpp"
#include "nfCommon/Containers/SharedPtr.hpp"
#include "nfCommon/Containers/WeakPtr.hpp"

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
    ASSERT_EQ(0, weakPtr.WeakRefCount());
}

TEST(WeakPtr, AssignEmptySharedPointer)
{
    const SharedPtr<int> sharedPtr;
    WeakPtr<int> weakPtr;
    weakPtr = sharedPtr;

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0, weakPtr.WeakRefCount());

    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, sharedPtr);
}

TEST(WeakPtr, ConstructFromEmptySharedPointer)
{
    const SharedPtr<int> sharedPtr;
    const WeakPtr<int> weakPtr(sharedPtr);

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0, weakPtr.WeakRefCount());

    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, sharedPtr);
}

TEST(WeakPtr, AssignEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    WeakPtr<int> weakPtr2;
    weakPtr2 = weakPtr;

    ASSERT_FALSE(weakPtr2.Valid());
    ASSERT_EQ(0, weakPtr2.WeakRefCount());
}

TEST(WeakPtr, ConstructFromEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    WeakPtr<int> weakPtr2(weakPtr);

    ASSERT_FALSE(weakPtr2.Valid());
    ASSERT_EQ(0, weakPtr2.WeakRefCount());

    ASSERT_EQ(weakPtr, weakPtr2);
    ASSERT_EQ(weakPtr2, weakPtr);
}

TEST(WeakPtr, ToWeakPtr)
{
    int counter = 0;
    SharedPtr<TestClass> sharedPtr(new TestClass(counter));
    const WeakPtr<TestClass> weakPtr(sharedPtr);

    ASSERT_EQ(0, counter);
    ASSERT_TRUE(sharedPtr);
    ASSERT_EQ(1, sharedPtr.RefCount());
    ASSERT_EQ(2, sharedPtr.WeakRefCount());
    ASSERT_EQ(2, weakPtr.WeakRefCount());
    ASSERT_TRUE(weakPtr.Valid());
    ASSERT_EQ(weakPtr, sharedPtr);
    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, weakPtr);

    sharedPtr.Reset();

    ASSERT_EQ(1, counter);
    ASSERT_FALSE(sharedPtr);
    ASSERT_EQ(0, sharedPtr.RefCount());
    ASSERT_EQ(0, sharedPtr.WeakRefCount());
    ASSERT_EQ(1, weakPtr.WeakRefCount());
    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(weakPtr, sharedPtr);
    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, weakPtr);
}

TEST(WeakPtr, Reset)
{
    int counter = 0;
    SharedPtr<TestClass> sharedPtr(new TestClass(counter));

    WeakPtr<TestClass> weakPtr(sharedPtr);
    weakPtr.Reset();

    ASSERT_EQ(0, counter);
    ASSERT_TRUE(sharedPtr);
    ASSERT_EQ(1, sharedPtr.RefCount());
    ASSERT_EQ(1, sharedPtr.WeakRefCount());
    ASSERT_EQ(0, weakPtr.WeakRefCount());
    ASSERT_FALSE(weakPtr.Valid());
}

TEST(WeakPtr, SharedPointerFromEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    const SharedPtr<int> sharedPtr = weakPtr.Lock();

    ASSERT_FALSE(sharedPtr);
    ASSERT_EQ(0, sharedPtr.RefCount());
    ASSERT_EQ(0, weakPtr.WeakRefCount());
}

TEST(WeakPtr, SharedPointerFromValidWeakPointer)
{
    int counter = 0;
    const SharedPtr<TestClass> sharedPtr(new TestClass(counter));
    const WeakPtr<TestClass> weakPtr(sharedPtr);
    const SharedPtr<TestClass> sharedPtr2 = weakPtr.Lock();

    ASSERT_EQ(0, counter);

    EXPECT_TRUE(sharedPtr);
    EXPECT_EQ(2, sharedPtr.RefCount());

    EXPECT_TRUE(sharedPtr2);
    EXPECT_EQ(2, sharedPtr2.RefCount());

    EXPECT_EQ(sharedPtr, sharedPtr2);
    EXPECT_EQ(sharedPtr2, sharedPtr);

    EXPECT_EQ(3, weakPtr.WeakRefCount());
    EXPECT_TRUE(weakPtr.Valid());
    EXPECT_EQ(sharedPtr2, weakPtr);
    EXPECT_EQ(weakPtr, sharedPtr2);
}

// TEMPSHIT
#include "nfCommon/Logger/Logger.hpp"
TEST(WeakPtr, CastToBaseClassViaWeakPtr_std)
{
    int counter = 0;
    {
        auto childPtr = std::make_shared<ChildTestClass>(counter);
        LOG_WARNING("childPtr     = %p", childPtr.get());
        childPtr->payload = 123;

        // create weak pointer to child class
        std::weak_ptr<ChildTestClass> childPtrWeak = childPtr;
        LOG_WARNING("childPtrWeak = %p", childPtrWeak.lock().get());
        ASSERT_EQ(0, counter);
        ASSERT_EQ(1, childPtr.use_count());

        // cast the weak pointer to base class
        std::weak_ptr<TestClass> basePtrWeak = childPtrWeak;
        LOG_WARNING("basePtrWeak  = %p", basePtrWeak.lock().get());
        ASSERT_EQ(0, counter);
        ASSERT_EQ(1, childPtr.use_count());
        ASSERT_TRUE(!basePtrWeak.expired());

        // convert the weak pointer to strong pointer
        std::shared_ptr<TestClass> basePtr = basePtrWeak.lock();
        LOG_WARNING("basePtrWeak  = %p", basePtr.get());
        ASSERT_EQ(0, counter);
        ASSERT_EQ(2, childPtr.use_count());
        ASSERT_TRUE(!basePtrWeak.expired());

        ASSERT_EQ(123, basePtr->payload);
    }
    ASSERT_EQ(1, counter);
}

TEST(WeakPtr, CastToBaseClassViaWeakPtr)
{
    int counter = 0;
    {
        auto childPtr = MakeSharedPtr<ChildTestClass>(counter);
        TestClass* basePtrRef = childPtr.Get(); // TEMPSHIT
        LOG_WARNING("childPtr     = %p", childPtr.Get()); // TEMPSHIT
        LOG_WARNING("basePtr  REF = %p", basePtrRef); // TEMPSHIT
        childPtr->payload = 123;

        // create weak pointer to child class
        WeakPtr<ChildTestClass> childPtrWeak = childPtr;
        LOG_WARNING("childPtrWeak = %p", childPtrWeak.Lock().Get()); // TEMPSHIT
        ASSERT_EQ(0, counter);
        ASSERT_EQ(1, childPtr.RefCount());
        ASSERT_EQ(2, childPtr.WeakRefCount());
        ASSERT_EQ(2, childPtrWeak.WeakRefCount());

        // cast the weak pointer to base class
        WeakPtr<TestClass> basePtrWeak = childPtrWeak;
        LOG_WARNING("basePtrWeak  = %p", basePtrWeak.Lock().Get()); // TEMPSHIT
        ASSERT_EQ(0, counter);
        ASSERT_EQ(1, childPtr.RefCount());
        ASSERT_EQ(3, childPtr.WeakRefCount());
        ASSERT_EQ(3, childPtrWeak.WeakRefCount());
        ASSERT_EQ(3, basePtrWeak.WeakRefCount());
        ASSERT_TRUE(childPtr);
        ASSERT_TRUE(basePtrWeak.Valid());

        // convert the weak pointer to strong pointer
        SharedPtr<TestClass> basePtr = basePtrWeak.Lock();
        LOG_WARNING("basePtrWeak  = %p", basePtr.Get()); // TEMPSHIT
        ASSERT_EQ(0, counter);
        ASSERT_EQ(2, childPtr.RefCount());
        ASSERT_EQ(4, childPtr.WeakRefCount());
        ASSERT_EQ(4, childPtrWeak.WeakRefCount());
        ASSERT_EQ(4, basePtrWeak.WeakRefCount());
        ASSERT_TRUE(childPtr);
        ASSERT_TRUE(basePtrWeak.Valid());
        ASSERT_EQ(basePtr, basePtrWeak);
        ASSERT_EQ(basePtrWeak, basePtr);

        ASSERT_EQ(123, basePtr->payload);
    }
    ASSERT_EQ(1, counter);
}