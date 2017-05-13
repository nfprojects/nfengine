#include "PCH.hpp"
#include "nfCommon/Containers/SharedPtr.hpp"


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


//////////////////////////////////////////////////////////////////////////
// Single reference tests - SharedPtr used as UniquePtr
//////////////////////////////////////////////////////////////////////////

TEST(SharedPtr, Empty_SingleRef)
{
    SharedPtr<int> pointer;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0, pointer.RefCount());
}

TEST(SharedPtr, Empty_MoveConstructor_SingleRef)
{
    SharedPtr<int> pointer;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0, pointer.RefCount());

    SharedPtr<int> pointer2(std::move(pointer));

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0, pointer.RefCount());

    ASSERT_EQ(pointer2, nullptr);
    ASSERT_FALSE(pointer2);
    ASSERT_EQ(0, pointer.RefCount());
}

TEST(SharedPtr, Empty_MoveAssignment_SingleRef)
{
    SharedPtr<int> pointer;
    SharedPtr<int> pointer2;

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0, pointer.RefCount());

    pointer2 = std::move(pointer);

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(0, pointer.RefCount());

    ASSERT_EQ(pointer2, nullptr);
    ASSERT_FALSE(pointer2);
    ASSERT_EQ(0, pointer.RefCount());
}

TEST(SharedPtr, SimpleType_SingleRef)
{
    SharedPtr<int> pointer(new int);
    *pointer = 123;

    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);
    ASSERT_EQ(123, *pointer);
    ASSERT_EQ(123, *pointer.Get());
    ASSERT_EQ(1, pointer.RefCount());
}

TEST(SharedPtr, Destructor_SingleRef)
{
    int counter = 0;
    {
        SharedPtr<TestClass> pointer(new TestClass(counter));
        ASSERT_EQ(0, counter);
    }
    ASSERT_EQ(1, counter);
}

TEST(SharedPtr, Reset_SingleRef)
{
    int counter = 0;
    SharedPtr<TestClass> pointer(new TestClass(counter));

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointer, nullptr);
    ASSERT_TRUE(pointer);
    ASSERT_EQ(1, pointer.RefCount());

    // destroy TestClass
    pointer.Reset();

    ASSERT_EQ(pointer, nullptr);
    ASSERT_FALSE(pointer);
    ASSERT_EQ(1, counter);
    ASSERT_EQ(0, pointer.RefCount());
}

TEST(SharedPtr, MoveConstructor_SingleRef)
{
    int counter = 0;
    SharedPtr<TestClass> pointer(new TestClass(counter));

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
    SharedPtr<TestClass> pointer(new TestClass(counter));
    SharedPtr<TestClass> pointer2(new TestClass(counter2));
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

//////////////////////////////////////////////////////////////////////////
// Serious SharedPtr tests
//////////////////////////////////////////////////////////////////////////

TEST(SharedPtr, CopyConstructor)
{
    int counter = 0;
    SharedPtr<TestClass> pointerA(new TestClass(counter));

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(1, pointerA.RefCount());

    SharedPtr<TestClass> pointerB(pointerA);

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(2, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(2, pointerB.RefCount());
    ASSERT_EQ(pointerA.Get(), pointerB.Get());

    // remove original reference
    pointerA.Reset();

    ASSERT_EQ(0, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(1, pointerB.RefCount());

    // remove second reference, object should be destroyed
    pointerB.Reset();

    ASSERT_EQ(1, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0, pointerA.RefCount());
    ASSERT_EQ(pointerB, nullptr);
    ASSERT_FALSE(pointerB);
    ASSERT_EQ(0, pointerA.RefCount());
}

TEST(SharedPtr, Assignment)
{
    int counter = 0;
    SharedPtr<TestClass> pointerA(new TestClass(counter));
    SharedPtr<TestClass> pointerB;

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(1, pointerA.RefCount());

    pointerB = pointerA;

    ASSERT_EQ(0, counter);
    ASSERT_NE(pointerA, nullptr);
    ASSERT_TRUE(pointerA);
    ASSERT_EQ(2, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(2, pointerB.RefCount());
    ASSERT_EQ(pointerA.Get(), pointerB.Get());

    // remove original reference
    pointerA.Reset();

    ASSERT_EQ(0, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0, pointerA.RefCount());
    ASSERT_NE(pointerB, nullptr);
    ASSERT_TRUE(pointerB);
    ASSERT_EQ(1, pointerB.RefCount());

    // remove second reference, object should be destroyed
    pointerB.Reset();

    ASSERT_EQ(1, counter);
    ASSERT_EQ(pointerA, nullptr);
    ASSERT_FALSE(pointerA);
    ASSERT_EQ(0, pointerA.RefCount());
    ASSERT_EQ(pointerB, nullptr);
    ASSERT_FALSE(pointerB);
    ASSERT_EQ(0, pointerA.RefCount());
}

TEST(SharedPtr, CopyConstructor_MultiThreaded)
{
    using PtrType = SharedPtr<TestClass>;

    int counter = 0;
    PtrType globalPointer(new TestClass(counter));
    std::atomic_bool finish(false);

    const auto func = [&finish, &globalPointer]()
    {
        while (!finish)
        {
            PtrType localPointer(globalPointer);
            localPointer->payload++;
        }
    };

    std::thread threadA(func);
    std::thread threadB(func);

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    finish = true;

    threadA.join();
    threadB.join();

    ASSERT_EQ(0, counter);
}
