#include "PCH.hpp"
#include "nfCommon/Window.hpp"

#include <thread>

namespace {

const unsigned int WINDOW_COUNT = 5;

} // namespace


TEST(WindowTest, Create)
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

    while (1) {
        mWindow.ProcessMessages();
        if (mWindow.IsClosed())
            break;
    }
}

TEST(WindowTest, MultiWindow)
{
    NFE::Common::Window mWindows[WINDOW_COUNT];

    // this part is very similar to previous test, only with multiple windows at once
    for (auto& window : mWindows)
        ASSERT_EQ(nullptr, window.GetHandle());

    for (auto& window : mWindows)
        ASSERT_TRUE(window.Open());

    for (auto& window : mWindows)
        ASSERT_NE(nullptr, window.GetHandle());

    // check if handles differ from each other (they should)
    for (unsigned int i = 0; i < WINDOW_COUNT; ++i)
        for (unsigned int j = i + 1; j < WINDOW_COUNT; ++j)
            ASSERT_NE(mWindows[i].GetHandle(), mWindows[j].GetHandle());
}
