#include "PCH.hpp"
#include "../nfCommon/Window.hpp"

TEST(WindowTest, CreateTest)
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
