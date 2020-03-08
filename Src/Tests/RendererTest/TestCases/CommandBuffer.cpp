#include "../PCH.hpp"
#include "../RendererTest.hpp"

#include "Engine/Renderers/Interface/CommandRecorder.hpp"

class CommandList : public RendererTest
{
public:
    CommandRecorderPtr commandRecorder;

private:
    void SetUp() override
    {
        commandRecorder = gRendererDevice->CreateCommandRecorder();
        ASSERT_NE(nullptr, commandRecorder.Get());
    }

    void TearDown() override
    {
        commandRecorder.Reset();
        ASSERT_TRUE(gRendererDevice->WaitForGPU());
    }
};

TEST_F(CommandList, ExecuteInvalid)
{
    EXPECT_FALSE(gRendererDevice->Execute(0));
    EXPECT_FALSE(gRendererDevice->Execute(123456));
}

TEST_F(CommandList, FinishWithoutReset)
{
    // finishing command buffer should fail, because it was not reset first
    CommandListID commandList = commandRecorder->Finish();
    ASSERT_EQ(0u, commandList);
}

TEST_F(CommandList, ResetAndFinish)
{
    commandRecorder->Begin();

    const CommandListID commandList = commandRecorder->Finish();
    ASSERT_NE(0u, commandList);

    // try to finish again - should fail, because command buffer must be reset
    const CommandListID commandList2 = commandRecorder->Finish();
    ASSERT_EQ(0u, commandList2);

    EXPECT_TRUE(gRendererDevice->Execute(commandList));

    // executing the same command list again should fail
    EXPECT_FALSE(gRendererDevice->Execute(commandList));
}