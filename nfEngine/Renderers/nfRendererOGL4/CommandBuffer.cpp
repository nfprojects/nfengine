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
#include "Texture.hpp"
#include "Sampler.hpp"
#include "PipelineState.hpp"


namespace NFE {
namespace Renderer {


CommandBuffer::CommandBuffer()
    : mCurrentIndexBufferFormat(GL_NONE)
    , mCurrentStencilFunc(GL_KEEP)
    , mCurrentStencilRef(0)
    , mCurrentStencilMask(0xFF)
    , mCurrentRenderTarget(nullptr)
    , mCurrentVertexLayoutElementsNum(0)
    , mSetVertexBuffer(nullptr)
    , mSetIndexBuffer(nullptr)
    , mSetVertexLayout(nullptr)
    , mSetTexture(nullptr)
    , mSetSampler(nullptr)
    , mSetShaderProgram(GL_NONE)
    , mSetConstantBufferSlot(GL_NONE)
    , mVertexBufferNeedsUpdate(false)
    , mIndexBufferNeedsUpdate(false)
    , mVertexLayoutNeedsUpdate(false)
    , mSSOEnabled(false)
    , mProgramPipeline(GL_NONE)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mProgramPipeline)
    {
        glBindProgramPipeline(GL_NONE);
        glDeleteProgramPipelines(1, &mProgramPipeline);
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
        GLenum type = TranslateElementFormatToType(vl->mDesc.elements[i].format, isNormalized);

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

void CommandBuffer::BindTexture()
{
    if (mSetTexture == nullptr)
    {
        glBindTextures(mSetTextureSlot, 1, GL_NONE);
        return;
    }

    Texture* t = dynamic_cast<Texture*>(mSetTexture);
    glBindTextures(mSetTextureSlot, 1, &(t->mTexture));
    mTextureNeedsUpdate = false;
}

void CommandBuffer::BindSampler()
{
    if (mSetSampler == nullptr)
    {
        GLuint nullSampler = GL_NONE;
        glBindSamplers(mSetSamplerSlot, 1, &nullSampler);
        return;
    }

    Sampler* s = dynamic_cast<Sampler*>(mSetSampler);
    glBindSamplers(mSetSamplerSlot, 1, &(s->mSampler));
    mSamplerNeedsUpdate = false;
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
    // TODO support SSO, or remove target argument (which requires fixes in D3D11 renderer)
    UNUSED(target);

    // TODO support multiple Samplers
    if (num > 1)
        LOG_WARNING("Binding multiple Samplers is not yet supported! Only first will be set.");

    mSetSampler = samplers[0];
    mSetSamplerSlot = slotOffset;
    mSamplerNeedsUpdate = true;
}

void CommandBuffer::SetTextures(ITexture** textures, int num, ShaderType target, int slotOffset)
{
    // TODO support SSO, or remove target argument (which requires fixes in D3D11 renderer)
    UNUSED(target);

    // TODO support multiple Textures
    if (num > 1)
        LOG_WARNING("Binding multiple Textures is not yet supported! Only first will be set.");

    mSetTexture = textures[0];
    mSetTextureSlot = slotOffset;
    mTextureNeedsUpdate = true;
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

    mCurrentRenderTarget = rt;

    if (rt == nullptr)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    else
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt->mFBO);
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

void CommandBuffer::SetBlendState(const BlendStateDesc& desc)
{
    if (desc.alphaToCoverage)
    {
        glEnable(GL_SAMPLE_COVERAGE);
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }
    else
    {
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glDisable(GL_SAMPLE_COVERAGE);
    }

    // TODO multiple Render Targets should also take BlendStateDesc::independent into account
    //      When supporting multiple RTs, replace gl* functions in this call with gl*i equivalents
    if (desc.independent)
        LOG_WARNING("Separate Blend States are not yet supported! Only the first one will be set.");

    const RenderTargetBlendStateDesc& rtBlendState = desc.rtDescs[0];
    if (rtBlendState.enable)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    glBlendFuncSeparate(TranslateBlendFunc(rtBlendState.srcColorFunc),
                        TranslateBlendFunc(rtBlendState.destColorFunc),
                        TranslateBlendFunc(rtBlendState.srcAlphaFunc),
                        TranslateBlendFunc(rtBlendState.destAlphaFunc));
    glBlendEquationSeparate(TranslateBlendOp(rtBlendState.colorOperator),
                            TranslateBlendOp(rtBlendState.alphaOperator));
}

void CommandBuffer::SetRasterizerState(const RasterizerStateDesc& desc)
{
    glFrontFace(GL_CW); // force set front face to CW to be compatible with cullMode options
    glCullFace(TranslateCullMode(desc.cullMode));
    glPolygonMode(GL_FRONT_AND_BACK, TranslateFillMode(desc.fillMode));
}

void CommandBuffer::SetDepthState(const DepthStateDesc& desc)
{
    // set up depth testing
    if (desc.depthTestEnable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    glDepthFunc(TranslateCompareFunc(desc.depthCompareFunc));
    glDepthMask(desc.depthWriteEnable ? GL_TRUE : GL_FALSE);

    // set up stencil testing
    if (desc.stencilEnable)
        glEnable(GL_STENCIL_TEST);
    else
        glDisable(GL_STENCIL_TEST);

    glStencilOp(TranslateStencilOp(desc.stencilOpFail),
                TranslateStencilOp(desc.stencilOpDepthFail),
                TranslateStencilOp(desc.stencilOpPass));

    mCurrentStencilFunc = TranslateCompareFunc(desc.stencilFunc);
    mCurrentStencilMask = static_cast<GLuint>(desc.stencilMask);
    glStencilFunc(mCurrentStencilFunc, mCurrentStencilRef, mCurrentStencilMask);
}

void CommandBuffer::SetPipelineState(IPipelineState* state)
{
    PipelineState* pipelineState = dynamic_cast<PipelineState*>(state);

    SetBlendState(pipelineState->mDesc.blendState);
    SetRasterizerState(pipelineState->mDesc.raterizerState);
    SetDepthState(pipelineState->mDesc.depthState);
}

void CommandBuffer::SetStencilRef(unsigned char ref)
{
    mCurrentStencilRef = ref;
    glStencilFunc(mCurrentStencilFunc, mCurrentStencilRef, mCurrentStencilMask);
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

void CommandBuffer::SetScissors(int left, int top, int right, int bottom)
{
    UNUSED(left);
    UNUSED(top);
    UNUSED(right);
    UNUSED(bottom);
}

void* CommandBuffer::MapBuffer(IBuffer* buffer, MapType type)
{
    UNUSED(buffer);
    UNUSED(type);
    return nullptr;
}

void CommandBuffer::UnmapBuffer(IBuffer* buffer)
{
    UNUSED(buffer);
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

void CommandBuffer::Clear(int flags, const float* color, float depthValue,
                          unsigned char stencilValue)
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


    if (flags & NFE_CLEAR_FLAG_STENCIL)
    {
        glFlags |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(static_cast<GLint>(stencilValue));
    }

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

    if (mTextureNeedsUpdate)
        BindTexture();

    if (mSamplerNeedsUpdate)
        BindSampler();

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

    if (mTextureNeedsUpdate)
        BindTexture();

    if (mSamplerNeedsUpdate)
        BindSampler();

    int bytePerIndex = mCurrentIndexBufferFormat == GL_UNSIGNED_SHORT ? 2 : 4;
    glDrawElements(TranslatePrimitiveType(type), indexNum, mCurrentIndexBufferFormat,
                   reinterpret_cast<void*>(static_cast<size_t>(indexOffset * bytePerIndex)));
}

ICommandList* CommandBuffer::Finish()
{
    return nullptr;
}

void CommandBuffer::Execute(ICommandList* commandList)
{
    UNUSED(commandList);
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
