#include "PCH.hpp"
#include "../nfCommon/Library.hpp"

using namespace NFE::Common;

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
    typedef int(*FuncType)(int, int);
    FuncType WinLife = reinterpret_cast<FuncType>(mLib->GetSymbol("Add"));
    ASSERT_TRUE(WinLife == nullptr);

    // Loading nonexistent function also results in nulltr
    ASSERT_TRUE(mLib->Open("testCalcLib"));
    WinLife = reinterpret_cast<FuncType>(mLib->GetSymbol("WinLife"));
    ASSERT_TRUE(WinLife == nullptr);
}

TEST_F(LibraryTest, LibUsage)
{
    // Open test library
    ASSERT_TRUE(mLib->Open("testCalcLib"));

    // Load functions from the library
    typedef int(*FuncType)(int, int);
    FuncType Add = reinterpret_cast<FuncType>(mLib->GetSymbol("Add"));
    FuncType Subtract = reinterpret_cast<FuncType>(mLib->GetSymbol("Subtract"));
    // Check if functions were loaded successfully
    ASSERT_TRUE(Add != NULL);
    ASSERT_TRUE(Subtract != NULL);

    // Check if loaded functions work properly
    int number1 = 12345;
    int number2 = 54321;

    int result = number1 + number2;
    int funcResult = Add(number1, number2);
    ASSERT_EQ(funcResult, result);

    result = number1 - number2;
    funcResult = Subtract(number1, number2);
    ASSERT_EQ(funcResult, result);
}

TEST_F(LibraryTest, IsOpened)
{
    ASSERT_FALSE(mLib->IsOpened());
    ASSERT_TRUE(mLib->Open("testCalcLib"));
    ASSERT_TRUE(mLib->IsOpened());
}

TEST_F(LibraryTest, CopyConstructor)
{
    ASSERT_TRUE(mLib->Open("testCalcLib"));
    Library lib(*mLib.get());
    ASSERT_TRUE(lib.IsOpened());
    ASSERT_FALSE(mLib->IsOpened());
    lib.Close();
}