/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of OpenGL 4 Command Buffer
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "Translations.hpp"
#include "CommandBuffer.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include "VertexLayout.hpp"


namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer()
    : mCurrentIndexBufferFormat(GL_NONE)
    , mCurrentRenderTarget(nullptr)
    , mCurrentVertexLayoutElementsNum(0)
    , mSetVertexBuffer(nullptr)
    , mSetIndexBuffer(nullptr)
    , mSetVertexLayout(nullptr)
    , mSetShaderProgram(GL_NONE)
    , mSetConstantBufferSlot(GL_NONE)
    , mVertexBufferNeedsUpdate(false)
    , mIndexBufferNeedsUpdate(false)
    , mVertexLayoutNeedsUpdate(false)
    , mSSOEnabled(false)
    , mProgramPipeline(GL_NONE)
    , mVAO(GL_NONE)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mProgramPipeline)
    {
        glBindProgramPipeline(GL_NONE);
        glDeleteProgramPipelines(1, &mProgramPipeline);
    }

    if (mVAO)
    {
        glBindVertexArray(GL_NONE);
        glDeleteVertexArrays(1, &mVAO);
    }
}

void CommandBuffer::BindVertexBuffer()
{
    if (mSetVertexBuffer == nullptr)
    {
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
        return;
    }

    /// TODO multiple VB support
    Buffer* vb = dynamic_cast<Buffer*>(mSetVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vb->mBuffer);
    mVertexBufferNeedsUpdate = false;
}

void CommandBuffer::BindIndexBuffer()
{
    if (mSetIndexBuffer == nullptr)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
        mCurrentIndexBufferFormat = GL_NONE;
        return;
    }

    Buffer* ib = dynamic_cast<Buffer*>(mSetIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->mBuffer);
    mIndexBufferNeedsUpdate = false;
}

void CommandBuffer::BindConstantBuffer()
{
    if (mSetConstantBuffer == nullptr)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, mSetConstantBufferSlot, GL_NONE, 0, 0);
        mConstantBufferNeedsUpdate = false;
        return;
    }

    Buffer* cb = dynamic_cast<Buffer*>(mSetConstantBuffer);
    glBindBufferRange(GL_UNIFORM_BUFFER, mSetConstantBufferSlot, cb->mBuffer, 0, cb->mSize);
    mConstantBufferNeedsUpdate = false;
}

void CommandBuffer::BindVertexLayout()
{
    // disable current attributes
    for (int i = 0; i < mCurrentVertexLayoutElementsNum; ++i)
        glDisableVertexAttribArray(i);

    if (mSetVertexLayout == nullptr)
        return;

    VertexLayout* vl = dynamic_cast<VertexLayout*>(mSetVertexLayout);
    mCurrentVertexLayoutElementsNum = vl->mDesc.numElements;

    // calculate stride for current vertex layout
    GLsizei stride = 0;
    for (int i = 0; i < mCurrentVertexLayoutElementsNum; ++i)
        stride += vl->mDesc.elements[i].size * GetElementFormatSize(vl->mDesc.elements[i].format);

    // enable & assgin new ones
    for (int i = 0; i < mCurrentVertexLayoutElementsNum; ++i)
    {
        bool isNormalized = false;
        GLenum type = TranslateElementFormat(vl->mDesc.elements[i].format, isNormalized);

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i,
            vl->mDesc.elements[i].size,
            type,
            isNormalized ? GL_TRUE : GL_FALSE,
            stride,
            reinterpret_cast<const void*>(static_cast<size_t>(vl->mDesc.elements[i].offset)));
    }

    mVertexLayoutNeedsUpdate = false;
}

void CommandBuffer::Reset()
{
    mCurrentRenderTarget = nullptr;

    glUseProgram(GL_NONE);
    glBindProgramPipeline(GL_NONE);
    mSSOEnabled = false;

    // disable vertex layouts
    for (int i = 0; i < mCurrentVertexLayoutElementsNum; ++i)
        glDisableVertexAttribArray(i);
    mCurrentVertexLayoutElementsNum = 0;

    // unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
    mCurrentIndexBufferFormat = GL_NONE;

    // reset pointers
    mSetVertexBuffer = nullptr;
    mSetIndexBuffer = nullptr;
    mSetVertexLayout = nullptr;
}

void CommandBuffer::SetVertexLayout(IVertexLayout* vertexLayout)
{
    mSetVertexLayout = vertexLayout;
    mVertexLayoutNeedsUpdate = true;
}

void CommandBuffer::SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets)
{
    UNUSED(strides);
    UNUSED(offsets);

    if (mVAO == GL_NONE)
    {
        // Here Linux has it's needs.
        // On Linux using OGL Core Profile a VAO must be bound to the pipeline for rendering. It can
        // be any VAO, however without it no drawing is performed. Since we don't have use for it
        // right now, just create a dummy and bind it for the future. We will probably reuse the VAO
        // later on when multiple VB support will be implemented.
        // Moreover, as with mProgramPipeline, the OGL extensions are not accessible in constructor,
        // so, we must do the VAO generation and binding here.
        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);
    }

    // TODO multiple vertex buffers
    if (num > 1)
        LOG_WARNING("Binding multiple Vertex Buffers is not yet supported! Only first Buffer will be set.");

    mSetVertexBuffer = vertexBuffers[0];
    mVertexBufferNeedsUpdate = true;
}

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format)
{
    mSetIndexBuffer = indexBuffer;
    mCurrentIndexBufferFormat = TranslateIndexBufferFormat(format);
    mIndexBufferNeedsUpdate = true;
}

void CommandBuffer::SetSamplers(ISampler** samplers, int num, ShaderType target, int slotOffset)
{
    UNUSED(samplers);
    UNUSED(num);
    UNUSED(target);
    UNUSED(slotOffset);
}

void CommandBuffer::SetTextures(ITexture** textures, int num, ShaderType target, int slotOffset)
{
    UNUSED(textures);
    UNUSED(num);
    UNUSED(target);
    UNUSED(slotOffset);
}

void CommandBuffer::SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target,
                                       int slotOffset)
{
    UNUSED(num);
    UNUSED(target);

    // TODO support multiple Constant Buffers
    if (num > 1)
         LOG_WARNING("Binding multiple CBuffers is not yet supported! Only first will be set.");

    mSetConstantBuffer = constantBuffers[0];
    mSetConstantBufferSlot = slotOffset;
    mConstantBufferNeedsUpdate = true;
}

void CommandBuffer::SetRenderTarget(IRenderTarget* renderTarget)
{
    RenderTarget* rt = dynamic_cast<RenderTarget*>(renderTarget);
    if (rt == nullptr && renderTarget != nullptr)
    {
        LOG_ERROR("Invalid 'renderTarget' pointer");
        return;
    }

    if (rt == mCurrentRenderTarget)
        return;

    // TODO support (renderTarget == nullptr) situation

    mCurrentRenderTarget = rt;
}

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    ShaderProgram* newShaderProgram = dynamic_cast<ShaderProgram*>(shaderProgram);
    if (newShaderProgram == nullptr)
    {
        LOG_ERROR("Invalid 'shader' pointer");
        return;
    }

    if (newShaderProgram->mProgram == GL_NONE)
    {
        LOG_ERROR("Invalid or uninitialized Shader Program provided.");
        return;
    }

    if (mSSOEnabled)
    {
        // SSO was activated, unbind Program Pipeline and switch the flag
        glBindProgramPipeline(GL_NONE);
        mSSOEnabled = false;
    }

    glUseProgram(newShaderProgram->mProgram);
    mSetShaderProgram = newShaderProgram->mProgram;
}

void CommandBuffer::SetShader(IShader* shader)
{
    Shader* newShader = dynamic_cast<Shader*>(shader);
    if (newShader == nullptr)
    {
        LOG_ERROR("Invalid 'shader' pointer");
        return;
    }

    if (newShader->mShaderProgram == GL_NONE)
    {
        LOG_ERROR("Invalid or uninitialized Separable Shader Program provided.");
        return;
    }

    if (!mSSOEnabled)
    {
        // generate a program pipeline to use if needed
        // unfortunately we cannot do this in constructor, the exceptions are probably
        // uninitialized at this stage and the function ptr equals to NULL
        if (!mProgramPipeline)
            glGenProgramPipelines(1, &mProgramPipeline);

        // unbind any bound program and bind our program pipeline
        glUseProgram(GL_NONE);
        glBindProgramPipeline(mProgramPipeline);
        mSSOEnabled = true;
    }

    // attach created program to the pipeline
    glUseProgramStages(mProgramPipeline, TranslateShaderTypeToGLBit(newShader->mType),
                       newShader->mShaderProgram);
}

void CommandBuffer::SetBlendState(IBlendState* state)
{
    UNUSED(state);
}

void CommandBuffer::SetRasterizerState(IRasterizerState* state)
{
    UNUSED(state);
}

void CommandBuffer::SetDepthState(IDepthState* state)
{
    UNUSED(state);
}

void CommandBuffer::SetViewport(float left, float width, float top, float height,
                                float minDepth, float maxDepth)
{
    UNUSED(minDepth);
    UNUSED(maxDepth);

    glViewport(static_cast<GLint>(left),
               static_cast<GLint>(top),
               static_cast<GLsizei>(width),
               static_cast<GLsizei>(height));
}

bool CommandBuffer::WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data)
{
    Buffer* buf = dynamic_cast<Buffer*>(buffer);
    if (!buf)
        return false;

    // copy data like in D3D11 - map buffer, memcpy, unmap
    glBindBuffer(GL_UNIFORM_BUFFER, buf->mBuffer);
    GLvoid* dataPtr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(static_cast<char*>(dataPtr) + offset, data, size);
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    return false;
}

bool CommandBuffer::ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data)
{
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);

    return false;
}

void CommandBuffer::CopyTexture(ITexture* src, ITexture* dest)
{
    UNUSED(src);
    UNUSED(dest);
}

bool CommandBuffer::ReadTexture(ITexture* tex, void* data)
{
    UNUSED(tex);
    UNUSED(data);

    return false;
}

void CommandBuffer::Clear(int flags, const float* color, float depthValue)
{
    // TODO Right now Clear assumes one default render target.
    //      For multiple render targets, one would have to:
    //        * bind one of the Framebuffers
    //        * call glClearColor and glClear

    GLbitfield glFlags = 0;

    if (flags & NFE_CLEAR_FLAG_TARGET)
    {
        glFlags = GL_COLOR_BUFFER_BIT;
        glClearColor(color[0], color[1], color[2], color[3]);
    }

    if (flags & NFE_CLEAR_FLAG_DEPTH)
    {
        glFlags |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depthValue);
    }

    // TODO: stencil buffer support

    glClear(glFlags);
}


void CommandBuffer::Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
                         int instanceOffset)
{
    // TODO instancing
    UNUSED(instancesNum);
    UNUSED(instanceOffset);

    if (mVertexBufferNeedsUpdate)
        BindVertexBuffer();

    if (mConstantBufferNeedsUpdate)
        BindConstantBuffer();

    if (mVertexLayoutNeedsUpdate)
        BindVertexLayout();

    glDrawArrays(TranslatePrimitiveType(type), vertexOffset, vertexNum);
}

void CommandBuffer::DrawIndexed(PrimitiveType type, int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    // TODO instancing
    UNUSED(instancesNum);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);

    if (mVertexBufferNeedsUpdate)
        BindVertexBuffer();

    if (mConstantBufferNeedsUpdate)
        BindConstantBuffer();

    if (mVertexLayoutNeedsUpdate)
        BindVertexLayout();

    if (mIndexBufferNeedsUpdate)
        BindIndexBuffer();

    glDrawElements(TranslatePrimitiveType(type), indexNum, mCurrentIndexBufferFormat,
                   reinterpret_cast<void*>(static_cast<size_t>(indexOffset)));
}

void CommandBuffer::Execute(ICommandBuffer* commandBuffer, bool saveState)
{
    UNUSED(commandBuffer);
    UNUSED(saveState);
}

void CommandBuffer::BeginDebugGroup(const char* text)
{
    UNUSED(text);
}

void CommandBuffer::EndDebugGroup()
{
}

void CommandBuffer::InsertDebugMarker(const char* text)
{
    UNUSED(text);
}

} // namespace Renderer
} // namespace NFE
