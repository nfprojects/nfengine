#include "stdafx.hpp"
#include "../nfCommon/Window.hpp"


class WindowTest : public testing::Test
{
protected:
    NFE::Common::Window mWindow;
};


TEST_F(WindowTest, CreateTest)
{
    void* result;
    result = mWindow.GetHandle();
    EXPECT_EQ(result, NULL);
    bool openResult = mWindow.Open();
    ASSERT_TRUE(openResult);
    openResult = mWindow.Open();
    ASSERT_FALSE(openResult);
    result = mWindow.GetHandle();
    ASSERT_NE(result, NULL);
}
