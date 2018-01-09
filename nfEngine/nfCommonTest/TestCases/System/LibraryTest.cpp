#include "PCH.hpp"
#include "nfCommon/System/Library.hpp"

#include <memory>


using namespace NFE::Common;

typedef int(*FuncType)(int, int);

class LibraryTest : public testing::Test
{
protected:
    std::unique_ptr<Library> mLib;

    void SetUp()
    {
        mLib.reset(new Library);
    }

    void TearDown()
    {
        mLib->Close();
    }
};

TEST_F(LibraryTest, NotExistingLib)
{
    ASSERT_FALSE(mLib->Open("this_file_does_not_exist"));
}

TEST_F(LibraryTest, GetSymbolErrors)
{
    // Loading function before opening library results in nullptr

    FuncType WinLife;
    ASSERT_FALSE(mLib->GetSymbol("Add", WinLife));

    // Loading nonexistent function also results in nulltr
    ASSERT_TRUE(mLib->Open("testCalcLib"));
    ASSERT_FALSE(mLib->GetSymbol("WinLife", WinLife));
}

TEST_F(LibraryTest, LibUsage)
{
    // Open test library
    ASSERT_TRUE(mLib->Open("testCalcLib"));

    // Load functions from the library
    FuncType Add, Subtract;
    ASSERT_TRUE(mLib->GetSymbol("Add", Add));
    ASSERT_TRUE(mLib->GetSymbol("Subtract", Subtract));
    // Check if functions were loaded successfully
    ASSERT_TRUE(nullptr != Add);
    ASSERT_TRUE(nullptr != Subtract);

    // Check if loaded functions work properly
    int number1 = 12345;
    int number2 = 54321;

    int result = number1 + number2;
    int funcResult = Add(number1, number2);
    ASSERT_EQ(result, funcResult);

    result = number1 - number2;
    funcResult = Subtract(number1, number2);
    ASSERT_EQ(result, funcResult);
}

TEST_F(LibraryTest, IsOpened)
{
    ASSERT_FALSE(mLib->IsOpened());
    ASSERT_TRUE(mLib->Open("testCalcLib"));
    ASSERT_TRUE(mLib->IsOpened());
}

TEST_F(LibraryTest, MoveConstructor)
{
    Library lib("testCalcLib");
    ASSERT_TRUE(lib.IsOpened());
    Library lib2 = std::move(lib);
    ASSERT_FALSE(lib.IsOpened());
    ASSERT_TRUE(lib2.IsOpened());
    lib.Close();
    lib2.Close();
}