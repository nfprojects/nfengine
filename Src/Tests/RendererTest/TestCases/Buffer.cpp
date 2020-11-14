#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "Engine/Common/Math/Random.hpp"
#include "Engine/Common/Utils/LanguageUtils.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"

// TODO: buffer write, readback, copy

class BufferTest : public RendererTest
{
};

TEST_F(BufferTest, Creation)
{
    const size_t testBufferSize = 64;
    BufferPtr buffer;

    // default (valid) buffer descriptor
    BufferDesc defBufferDesc;
    defBufferDesc.size = testBufferSize;
    defBufferDesc.debugName = "BufferTest::defBufferDesc";

    BufferDesc bufferDesc;

    // zero-sized buffer
    bufferDesc = defBufferDesc;
    bufferDesc.size = 0;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_EQ(nullptr, buffer.Get());

    // valid upload buffer
    bufferDesc = defBufferDesc;
    bufferDesc.mode = ResourceAccessMode::Upload;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_NE(nullptr, buffer.Get());

    // valid readback buffer
    bufferDesc = defBufferDesc;
    bufferDesc.mode = ResourceAccessMode::Readback;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_NE(nullptr, buffer.Get());

    // valid GPU-only buffer
    bufferDesc = defBufferDesc;
    bufferDesc.mode = ResourceAccessMode::GPUOnly;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_NE(nullptr, buffer.Get());
}

TEST_F(BufferTest, WriteAndRead)
{
    const uint32 bufferSize = 1024;
    uint8 initData[bufferSize];

    Math::Random random;
    for (uint32 i = 0; i < bufferSize; ++i)
    {
        initData[i] = (uint8)random.GetInt();
    }

    BufferDesc bufferDesc;
    bufferDesc.size = bufferSize;
    bufferDesc.debugName = "BufferTest::WriteAndRead::buffer";
    BufferPtr buffer = gRendererDevice->CreateBuffer(bufferDesc);
    ASSERT_NE(nullptr, buffer.Get());

    bufferDesc.mode = ResourceAccessMode::Readback;
    bufferDesc.debugName = "BufferTest::WriteAndRead::readbackBuffer";
    BufferPtr readbackBuffer = gRendererDevice->CreateBuffer(bufferDesc);
    ASSERT_NE(nullptr, buffer.Get());

    const CommandRecorderPtr recorder = gRendererDevice->CreateCommandRecorder();
    recorder->Begin(CommandQueueType::Copy);
    recorder->WriteBuffer(buffer, 0, bufferSize, initData);
    recorder->CopyBuffer(buffer, readbackBuffer);
    CommandListPtr commandList = recorder->Finish();

    gCopyCommandQueue->Execute(commandList);
    const FencePtr fence = gMainCommandQueue->Signal();
    ASSERT_NE(nullptr, fence.Get());
    fence->Wait();

    buffer.Reset();

    uint8 readData[bufferSize];
    {
        void* mappedMemory = readbackBuffer->Map();
        ASSERT_NE(nullptr, mappedMemory);
        memcpy(readData, mappedMemory, bufferSize);
        readbackBuffer->Unmap();
    }

    for (uint32 i = 0; i < bufferSize; ++i)
    {
        EXPECT_EQ(initData[i], readData[i]) << "i=" << i;
    }
}

TEST_F(BufferTest, WriteAndRead_DifferentQueue)
{
    const uint32 bufferSize = 1024;
    uint8 initData[bufferSize];

    Math::Random random;
    for (uint32 i = 0; i < bufferSize; ++i)
    {
        initData[i] = (uint8)random.GetInt();
    }

    BufferDesc bufferDesc;
    bufferDesc.size = bufferSize;
    bufferDesc.debugName = "BufferTest::WriteAndRead_DifferentQueue::buffer";
    BufferPtr buffer = gRendererDevice->CreateBuffer(bufferDesc);
    ASSERT_NE(nullptr, buffer.Get());

    bufferDesc.mode = ResourceAccessMode::Readback;
    bufferDesc.debugName = "BufferTest::WriteAndRead_DifferentQueue::readbackBuffer";
    BufferPtr readbackBuffer = gRendererDevice->CreateBuffer(bufferDesc);
    ASSERT_NE(nullptr, buffer.Get());

    CommandRecorderPtr recorder = gRendererDevice->CreateCommandRecorder();

    recorder->Begin(CommandQueueType::Graphics);
    recorder->CopyBuffer(buffer, readbackBuffer);
    CommandListPtr commandList_Copy = recorder->Finish();

    recorder->Begin(CommandQueueType::Copy);
    recorder->WriteBuffer(buffer, 0, bufferSize, initData);
    CommandListPtr commandList_Upload = recorder->Finish();

    recorder.Reset();

    gCopyCommandQueue->Execute(commandList_Upload);
    const FencePtr fenceAfterUpload = gCopyCommandQueue->Signal();
    ASSERT_NE(nullptr, fenceAfterUpload.Get());

    gMainCommandQueue->Execute(commandList_Copy, fenceAfterUpload);
    const FencePtr fenceAfterCopy = gMainCommandQueue->Signal();
    ASSERT_NE(nullptr, fenceAfterCopy.Get());

    fenceAfterCopy->Wait();
    EXPECT_TRUE(fenceAfterUpload->IsFinished());

    buffer.Reset();

    uint8 readData[bufferSize];
    {
        void* mappedMemory = readbackBuffer->Map();
        ASSERT_NE(nullptr, mappedMemory);
        memcpy(readData, mappedMemory, bufferSize);
        readbackBuffer->Unmap();
    }

    for (uint32 i = 0; i < bufferSize; ++i)
    {
        EXPECT_EQ(initData[i], readData[i]) << "i=" << i;
    }
}