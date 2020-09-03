#include "../PCH.hpp"
#include "../RendererTest.hpp"

#include "Engine/Renderers/RendererCommon/CommandRecorder.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"

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
        gRendererDevice->WaitForGPU()->Wait();
    }
};

TEST_F(CommandList, FinishWithoutReset)
{
    // finishing command buffer should fail, because it was not reset first
    ASSERT_EQ(commandRecorder->Finish(), nullptr);
}

TEST_F(CommandList, ResetAndFinish)
{
    commandRecorder->Begin();

    const CommandListPtr commandList = commandRecorder->Finish();
    ASSERT_NE(commandList, nullptr);

    // try to finish again - should fail, because command buffer must be reset
    const CommandListPtr commandList2 = commandRecorder->Finish();
    ASSERT_EQ(commandList2, nullptr);

    EXPECT_TRUE(gRendererDevice->Execute(commandList));

    // executing the same command list again should fail
    EXPECT_FALSE(gRendererDevice->Execute(commandList));
}