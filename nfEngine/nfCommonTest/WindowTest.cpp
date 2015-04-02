#include "stdafx.hpp"
#include "../nfCommon/Window.hpp"


class WindowTest : public testing::Test
{
};


TEST_F(WindowTest, CreateTest)
{
    NFE::Common::Window mWindow;
    void* result;
    result = mWindow.GetHandle();
    ASSERT_EQ(nullptr, result);

    bool openResult = mWindow.Open();
    ASSERT_TRUE(openResult);

    openResult = mWindow.Open();
    ASSERT_FALSE(openResult);

    result = mWindow.GetHandle();
    ASSERT_NE(nullptr, result);
}
