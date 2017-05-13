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
    ~TestClass() { mCounter++; }
    std::atomic_int payload;
private:
    int& mCounter;
};

} // namespace


TEST(WeakPtr, Constructor)
{
    const WeakPtr<int> weakPtr;

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0, weakPtr.RefCount());
}

TEST(WeakPtr, AssignEmptySharedPointer)
{
    const SharedPtr<int> sharedPtr;
    WeakPtr<int> weakPtr;
    weakPtr = sharedPtr;

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0, weakPtr.RefCount());
}

TEST(WeakPtr, ConstructFromEmptySharedPointer)
{
    const SharedPtr<int> sharedPtr;
    const WeakPtr<int> weakPtr(sharedPtr);

    ASSERT_FALSE(weakPtr.Valid());
    ASSERT_EQ(0, weakPtr.RefCount());
}

TEST(WeakPtr, AssignEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    WeakPtr<int> weakPtr2;
    weakPtr2 = weakPtr;

    ASSERT_FALSE(weakPtr2.Valid());
    ASSERT_EQ(0, weakPtr2.RefCount());
}

TEST(WeakPtr, ConstructFromEmptyWeakPointer)
{
    const WeakPtr<int> weakPtr;
    WeakPtr<int> weakPtr2(weakPtr);

    ASSERT_FALSE(weakPtr2.Valid());
    ASSERT_EQ(0, weakPtr2.RefCount());
}

TEST(WeakPtr, ToWeakPtr)
{
    int counter = 0;
    SharedPtr<TestClass> sharedPtr(new TestClass(counter));
    WeakPtr<TestClass> weakPtr(sharedPtr);

    ASSERT_EQ(0, counter);
    ASSERT_TRUE(sharedPtr);
    ASSERT_EQ(1, sharedPtr.RefCount());
    ASSERT_EQ(1, weakPtr.RefCount());
    ASSERT_TRUE(weakPtr.Valid());
    ASSERT_EQ(weakPtr, sharedPtr);
    ASSERT_EQ(sharedPtr, weakPtr);
    ASSERT_EQ(weakPtr, weakPtr);

    sharedPtr.Reset();

    ASSERT_EQ(1, counter);
    ASSERT_FALSE(sharedPtr);
    ASSERT_EQ(0, sharedPtr.RefCount());
    ASSERT_EQ(0, weakPtr.RefCount());
    ASSERT_FALSE(weakPtr.Valid());
}

TEST(WeakPtr, SharedPointerFromEmptyWeakPointer)
{
    WeakPtr<int> weakPtr;
    SharedPtr<int> sharedPtr = weakPtr.Lock();

    ASSERT_FALSE(sharedPtr);
    ASSERT_EQ(0, sharedPtr.RefCount());
    ASSERT_EQ(0, weakPtr.RefCount());
}

TEST(WeakPtr, SharedPointerFromValidWeakPointer)
{
    int counter = 0;
    SharedPtr<TestClass> sharedPtr(new TestClass(counter));
    WeakPtr<TestClass> weakPtr(sharedPtr);
    SharedPtr<TestClass> sharedPtr2 = weakPtr.Lock();

    ASSERT_EQ(0, counter);

    EXPECT_TRUE(sharedPtr);
    EXPECT_EQ(2, sharedPtr.RefCount());

    EXPECT_TRUE(sharedPtr2);
    EXPECT_EQ(2, sharedPtr2.RefCount());

    EXPECT_EQ(sharedPtr, sharedPtr2);

    EXPECT_EQ(2, weakPtr.RefCount());
    EXPECT_TRUE(weakPtr.Valid());
}