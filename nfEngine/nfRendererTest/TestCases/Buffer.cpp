#include "../PCH.hpp"
#include "../RendererTest.hpp"
#include "nfCommon/Math/Math.hpp"


// TODO: buffer write, readback, copy

class BufferTest : public RendererTest
{
};

TEST_F(BufferTest, BufferCreation)
{
    const size_t testBufferSize = 64;
    const char data[] = { 0 };
    std::unique_ptr<IBuffer> buffer;

    const BufferType bufferTypes[] =
    {
        BufferType::Constant,
        BufferType::Index,
        BufferType::Vertex,
        // TODO raw buffers, etc.
    };

    for (int i = 0; i < ArraySize(bufferTypes); ++i)
    {
        const char* bufferTypeStr = "unknown";
        switch (bufferTypes[i])
        {
        case BufferType::Constant:
            bufferTypeStr = "Constant";
            break;
        case BufferType::Index:
            bufferTypeStr = "Index";
            break;
        case BufferType::Vertex:
            bufferTypeStr = "Vertex";
            break;
        }
        SCOPED_TRACE("BufferType: " + std::string(bufferTypeStr));

        // default (valid) buffer descriptor
        BufferDesc defBufferDesc;
        defBufferDesc.access = BufferAccess::CPU_Write;
        defBufferDesc.size = testBufferSize;
        defBufferDesc.type = bufferTypes[i];
        defBufferDesc.debugName = "BufferTest::defBufferDesc";

        BufferDesc bufferDesc;

        // zero-sized buffer
        bufferDesc = defBufferDesc;
        bufferDesc.size = 0;
        buffer.reset(gRendererDevice->CreateBuffer(bufferDesc));
        EXPECT_EQ(nullptr, buffer.get());

        if (defBufferDesc.type == BufferType::Constant)
        {
            // constant buffer too big buffer
            bufferDesc = defBufferDesc;
            bufferDesc.size = 1024 * 1024 * 1024;
            buffer.reset(gRendererDevice->CreateBuffer(bufferDesc));
            EXPECT_EQ(nullptr, buffer.get());
        }

        // buffers can not be CPU readable
        bufferDesc = defBufferDesc;
        bufferDesc.access = BufferAccess::CPU_Read;
        buffer.reset(gRendererDevice->CreateBuffer(bufferDesc));
        EXPECT_EQ(nullptr, buffer.get());

        // read-only buffers must have defined content upon creation
        bufferDesc = defBufferDesc;
        bufferDesc.access = BufferAccess::GPU_ReadOnly;
        bufferDesc.initialData = nullptr;
        buffer.reset(gRendererDevice->CreateBuffer(bufferDesc));

        EXPECT_EQ(nullptr, buffer.get());

        // valid dynamic buffer
        bufferDesc = defBufferDesc;
        bufferDesc.initialData = data;
        buffer.reset(gRendererDevice->CreateBuffer(bufferDesc));
        EXPECT_NE(nullptr, buffer.get());

        // valid dynamic buffer
        bufferDesc = defBufferDesc;
        buffer.reset(gRendererDevice->CreateBuffer(bufferDesc));
        EXPECT_NE(nullptr, buffer.get());

        // valid read-only buffer
        bufferDesc = defBufferDesc;
        bufferDesc.access = BufferAccess::GPU_ReadOnly;
        bufferDesc.initialData = data;
        buffer.reset(gRendererDevice->CreateBuffer(bufferDesc));
        EXPECT_NE(nullptr, buffer.get());
    }
}
