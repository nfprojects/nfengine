#include "PCH.hpp"
#include "../nfCommon/EnumIterator.hpp"
#include <iostream>

using namespace NFE::Common;


namespace {
enum class testEnum
{
    None,
    value1,
    value2,
    value3,
    value4,
    value5,
    First = value1,
    Last = value4,
};

const char* testEnumToStr(testEnum obj)
{
    switch (obj)
    {
        case testEnum::value1:
            return "val1";
            break;
        case testEnum::value2:
            return "val2";
            break;
        case testEnum::value3:
            return "val3";
            break;
        case testEnum::value4:
            return "val4";
            break;
        case testEnum::value5:
            return "val5";
            break;
        default:
            return "None";
    }
}
}

TEST(EnumIteratorTest, SimpleTest)
{
    auto iter = EnumIterator<testEnum>().begin();
    auto endIter = EnumIterator<testEnum>().end();

    ASSERT_EQ(*iter, testEnum::value1);
    ASSERT_EQ(iter, EnumIterator<testEnum>().begin());
    ASSERT_NE(iter, endIter);

    ++iter;
    ASSERT_EQ(*iter, testEnum::value2);
    ASSERT_NE(iter, endIter);

    ++iter;
    ASSERT_EQ(*iter, testEnum::value3);
    ASSERT_NE(iter, endIter);

    ++iter;
    ASSERT_EQ(*iter, testEnum::value4);
    ASSERT_NE(iter, endIter);

    ++iter;
    ASSERT_EQ(iter, endIter);
}