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
        gMainCommandQueue->Signal()->Wait();
    }
};

TEST_F(CommandList, ResetAndFinish)
{
    commandRecorder->Begin(CommandQueueType::Graphics);

    const CommandListPtr commandList = commandRecorder->Finish();
    ASSERT_NE(commandList, nullptr);

    gMainCommandQueue->Execute(commandList);
}