#include "PCH.hpp"
#include "../nfCommon/Library.hpp"

using namespace NFE::Common;


TEST(LibraryTest, Simple)
{
    Library lib;
    EXPECT_FALSE(lib.Open("this_file_does_not_exist"));
    ASSERT_TRUE(lib.Open("testCalcLib"));

    typedef int (*FuncType)(int, int);
    FuncType Add = static_cast<FuncType>(lib.GetSymbol("Add"));
    FuncType Subtract = static_cast<FuncType>(lib.GetSymbol("Subtract"));
    ASSERT_TRUE(Add != NULL);
    ASSERT_TRUE(Subtract != NULL);

    int number1 = 12345;
    int number2 = 54321;

    int result = number1 + number2;
    int funcResult = Add(number1, number2);
    ASSERT_EQ(funcResult, result);

    result = number1 - number2;
    funcResult = Subtract(number1, number2);
    ASSERT_EQ(funcResult, result);

    lib.Close();
}