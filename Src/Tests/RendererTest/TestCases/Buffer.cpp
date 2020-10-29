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
    const char data[testBufferSize] = { 0 };
    BufferPtr buffer;

    // default (valid) buffer descriptor
    BufferDesc defBufferDesc;
    defBufferDesc.mode = ResourceAccessMode::Upload;
    defBufferDesc.size = testBufferSize;
    defBufferDesc.debugName = "BufferTest::defBufferDesc";

    BufferDesc bufferDesc;

    // zero-sized buffer
    bufferDesc = defBufferDesc;
    bufferDesc.size = 0;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_EQ(nullptr, buffer.Get());

    // valid readback buffer
    bufferDesc = defBufferDesc;
    bufferDesc.mode = ResourceAccessMode::Readback;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_NE(nullptr, buffer.Get());

    // static buffers must have defined content upon creation
    bufferDesc = defBufferDesc;
    bufferDesc.mode = ResourceAccessMode::Static;
    bufferDesc.initialData = nullptr;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_EQ(nullptr, buffer.Get());

    // valid dynamic buffer
    bufferDesc = defBufferDesc;
    bufferDesc.initialData = data;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_NE(nullptr, buffer.Get());

    // valid dynamic buffer
    bufferDesc = defBufferDesc;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_NE(nullptr, buffer.Get());

    // valid static buffer
    bufferDesc = defBufferDesc;
    bufferDesc.mode = ResourceAccessMode::Static;
    bufferDesc.initialData = data;
    buffer = gRendererDevice->CreateBuffer(bufferDesc);
    EXPECT_NE(nullptr, buffer.Get());
}

TEST_F(BufferTest, Download)
{
    const uint32 bufferSize = 1024;
    uint8 readData[bufferSize];
    uint8 initData[bufferSize];

    Math::Random random;
    for (uint32 i = 0; i < bufferSize; ++i)
    {
        initData[i] = (uint8)random.GetInt();
    }

    BufferDesc bufferDesc;
    bufferDesc.mode = ResourceAccessMode::Static;
    bufferDesc.size = bufferSize;
    bufferDesc.initialData = initData;
    bufferDesc.debugName = "BufferTest::BufferRead";
    BufferPtr buffer = gRendererDevice->CreateBuffer(bufferDesc);
    ASSERT_NE(nullptr, buffer.Get());

    const auto readCallback = [&](const void* data, size_t, size_t)
    {
        memcpy(readData, data, bufferSize);
    };

    Common::Waitable waitable;
    {
        Common::TaskBuilder builder{ waitable };
        ASSERT_TRUE(gRendererDevice->DownloadBuffer(buffer, readCallback, builder, 0, bufferSize));
    }
    waitable.Wait();

    for (uint32 i = 0; i < bufferSize; ++i)
    {
        EXPECT_EQ(initData[i], readData[i]);
    }
}