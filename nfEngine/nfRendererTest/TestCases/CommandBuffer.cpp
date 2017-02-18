#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "nfCommon/Math/Math.hpp"

class CommandList : public RendererTest
{
};

TEST_F(CommandList, FinishWithoutReset)
{
    CommandRecorderPtr commandRecorder(gRendererDevice->CreateCommandRecorder());
    ASSERT_NE(nullptr, commandRecorder.get());

    // finishing command buffer should fail, because it was not reset first
    std::unique_ptr<ICommandList> commandList(commandRecorder->Finish());
    ASSERT_EQ(nullptr, commandList.get());
}

TEST_F(CommandList, ResetAndFinish)
{
    CommandRecorderPtr commandRecorder(gRendererDevice->CreateCommandRecorder());
    ASSERT_NE(nullptr, commandRecorder.get());

    commandRecorder->Reset();

    std::unique_ptr<ICommandList> commandList(commandRecorder->Finish());
    ASSERT_NE(nullptr, commandList.get());

    EXPECT_TRUE(gRendererDevice->Execute(commandList.get()));

    // try to finish again - should fail, because command buffer must be reset
    std::unique_ptr<ICommandList> commandList2(commandRecorder->Finish());
    ASSERT_EQ(nullptr, commandList2.get());
}