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

} //namespace

/**
 * To test correct iteration, we recreate it in a manual way.
 * Only operators and methods used by a ranged-based for loops are used here
 * and operator== for ASSERT_EQ().
 */
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