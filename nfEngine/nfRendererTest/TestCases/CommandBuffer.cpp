#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "nfCommon/Math/Math.hpp"

class CommandList : public RendererTest
{
};

TEST_F(CommandList, FinishWithoutReset)
{
    std::unique_ptr<ICommandBuffer> commandBuffer(gRendererDevice->CreateCommandBuffer());
    ASSERT_NE(nullptr, commandBuffer.get());

    // finishing command buffer should fail, because it was not reset first
    std::unique_ptr<ICommandList> commandList(commandBuffer->Finish());
    ASSERT_EQ(nullptr, commandList.get());
}

TEST_F(CommandList, ResetAndFinish)
{
    std::unique_ptr<ICommandBuffer> commandBuffer(gRendererDevice->CreateCommandBuffer());
    ASSERT_NE(nullptr, commandBuffer.get());

    commandBuffer->Reset();

    std::unique_ptr<ICommandList> commandList(commandBuffer->Finish());
    ASSERT_NE(nullptr, commandList.get());

    for (int i = 0; i < 100; ++i)
        EXPECT_TRUE(gRendererDevice->Execute(commandList.get())) << "i = " << i;

    // try to finish again - should fail, because command buffer must be reset
    std::unique_ptr<ICommandList> commandList2(commandBuffer->Finish());
    ASSERT_EQ(nullptr, commandList2.get());
}