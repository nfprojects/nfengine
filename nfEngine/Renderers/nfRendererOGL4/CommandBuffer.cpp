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

// TODO OpenGL provides ARB_vertex_attrib_binding natively since 4.3.
//      Consider switching to these functions for more D3D-compliant API.
void CommandBuffer::BindVertexBufferAndLayout()
{
    // disable current attributes
    for (int i = 0; i < mCurrentVertexLayoutElementsNum; ++i)
        glDisableVertexAttribArray(i);

    mVertexBufferNeedsUpdate = false;
    mVertexLayoutNeedsUpdate = false;

    if (mSetVertexLayout == nullptr)
        return;

    if (mSetVertexBuffers.empty())
        return;

    VertexLayout* vl = dynamic_cast<VertexLayout*>(mSetVertexLayout);
    mCurrentVertexLayoutElementsNum = vl->mDesc.numElements;

    // calculate stride for current vertex layout
    // each of them must be calculated per bound VBO
    std::vector<GLsizei> strides(mSetVertexBuffers.size(), 0);
    for (int i = 0; i < mCurrentVertexLayoutElementsNum; ++i)
        strides[vl->mDesc.elements[i].vertexBufferId] +=
            vl->mDesc.elements[i].size * GetElementFormatSize(vl->mDesc.elements[i].format);

    Buffer* vb = nullptr;

    for (int i = 0; i < mCurrentVertexLayoutElementsNum; ++i)
    {
        glEnableVertexAttribArray(i);
        vb = dynamic_cast<Buffer*>(mSetVertexBuffers[vl->mDesc.elements[i].vertexBufferId]);
        glBindBuffer(GL_ARRAY_BUFFER, vb->mBuffer);

        bool isNormalized = false;
        GLenum type = TranslateElementFormatToType(vl->mDesc.elements[i].format, isNormalized);

        glVertexAttribPointer(i,
            vl->mDesc.elements[i].size,
            type,
            isNormalized ? GL_TRUE : GL_FALSE,
            strides[vl->mDesc.elements[i].vertexBufferId],
            reinterpret_cast<const void*>(static_cast<size_t>(vl->mDesc.elements[i].offset)));

        glVertexAttribDivisor(i, vl->mDesc.elements[i].instanceDataStep);
    }

    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
}

void CommandBuffer::BindIndexBuffer()
{
    mIndexBufferNeedsUpdate = false;

    if (mSetIndexBuffer == nullptr)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
        mCurrentIndexBufferFormat = GL_NONE;
        return;
    }

    Buffer* ib = dynamic_cast<Buffer*>(mSetIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->mBuffer);
}

void CommandBuffer::BindConstantBuffer()
{
    mConstantBufferNeedsUpdate = false;

    if (mSetConstantBuffer == nullptr)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, mSetConstantBufferSlot, GL_NONE, 0, 0);
        return;
    }

    Buffer* cb = dynamic_cast<Buffer*>(mSetConstantBuffer);
    glBindBufferRange(GL_UNIFORM_BUFFER, mSetConstantBufferSlot, cb->mBuffer, 0, cb->mSize);
}

void CommandBuffer::BindTexture()
{
    mTextureNeedsUpdate = false;

    if (mSetTexture == nullptr)
    {
        glBindTextures(mSetTextureSlot, 1, GL_NONE);
        return;
    }

    Texture* t = dynamic_cast<Texture*>(mSetTexture);
    glBindTextures(mSetTextureSlot, 1, &(t->mTexture));
}

void CommandBuffer::BindSampler()
{
    mSamplerNeedsUpdate = false;

    if (mSetSampler == nullptr)
    {
        GLuint nullSampler = GL_NONE;
        glBindSamplers(mSetSamplerSlot, 1, &nullSampler);
        return;
    }

    Sampler* s = dynamic_cast<Sampler*>(mSetSampler);
    glBindSamplers(mSetSamplerSlot, 1, &(s->mSampler));
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
    mSetVertexBuffers.clear();
    mSetIndexBuffer = nullptr;
    mSetConstantBuffer = nullptr;
    mSetVertexLayout = nullptr;
    mSetTexture = nullptr;
    mSetSampler = nullptr;
}

void CommandBuffer::SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets)
{
    NFE_UNUSED(strides);
    NFE_UNUSED(offsets);

    mSetVertexBuffers.clear();
    for (int i = 0; i < num; ++i)
        mSetVertexBuffers.push_back(vertexBuffers[i]);

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
    NFE_UNUSED(target);

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
    NFE_UNUSED(target);

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
    NFE_UNUSED(num);
    NFE_UNUSED(target);

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
        NFE_LOG_ERROR("Invalid 'renderTarget' pointer");
        return;
    }

    if (rt == mCurrentRenderTarget)
        return;

    mCurrentRenderTarget = rt;

    if (rt == nullptr)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    else
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt->mFBO);

    glDrawBuffers(static_cast<GLsizei>(rt->mAttachments.size()), rt->mAttachments.data());
}

void CommandBuffer::SetShaderProgram(IShaderProgram* shaderProgram)
{
    ShaderProgram* newShaderProgram = dynamic_cast<ShaderProgram*>(shaderProgram);
    if (newShaderProgram == nullptr)
    {
        NFE_LOG_ERROR("Invalid 'shader' pointer");
        return;
    }

    if (newShaderProgram->mProgram == GL_NONE)
    {
        NFE_LOG_ERROR("Invalid or uninitialized Shader Program provided.");
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

    mSetVertexLayout = pipelineState->mDesc.vertexLayout;
    mVertexLayoutNeedsUpdate = true;
}

void CommandBuffer::SetStencilRef(unsigned char ref)
{
    mCurrentStencilRef = ref;
    glStencilFunc(mCurrentStencilFunc, mCurrentStencilRef, mCurrentStencilMask);
}

void CommandBuffer::SetViewport(float left, float width, float top, float height,
                                float minDepth, float maxDepth)
{
    NFE_UNUSED(minDepth);
    NFE_UNUSED(maxDepth);

    glViewport(static_cast<GLint>(left),
               static_cast<GLint>(top),
               static_cast<GLsizei>(width),
               static_cast<GLsizei>(height));
}

void CommandBuffer::SetScissors(int left, int top, int right, int bottom)
{
    NFE_UNUSED(left);
    NFE_UNUSED(top);
    NFE_UNUSED(right);
    NFE_UNUSED(bottom);
}

void* CommandBuffer::MapBuffer(IBuffer* buffer, MapType type)
{
    NFE_UNUSED(buffer);
    NFE_UNUSED(type);
    return nullptr;
}

void CommandBuffer::UnmapBuffer(IBuffer* buffer)
{
    NFE_UNUSED(buffer);
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
    NFE_UNUSED(buffer);
    NFE_UNUSED(offset);
    NFE_UNUSED(size);
    NFE_UNUSED(data);

    return false;
}

void CommandBuffer::CopyTexture(ITexture* src, ITexture* dest)
{
    Texture* srcTex = dynamic_cast<Texture*>(src);
    Texture* dstTex = dynamic_cast<Texture*>(dest);

    if (!srcTex)
    {
        NFE_LOG_ERROR("Invalid src pointer");
        return;
    }

    if (!dstTex)
    {
        NFE_LOG_ERROR("Invalid dst pointer");
        return;
    }

    // TODO support for other texture dimensions (1D/3D)
    glCopyImageSubData(srcTex->mTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
                       dstTex->mTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
                       srcTex->mWidth, srcTex->mHeight, 1);
}

bool CommandBuffer::ReadTexture(ITexture* tex, void* data)
{
    Texture* texture = dynamic_cast<Texture*>(tex);
    if (!texture)
    {
        NFE_LOG_ERROR("Invalid tex pointer");
        return false;
    }

    if (!data)
    {
        NFE_LOG_ERROR("Invalid data pointer");
        return false;
    }

    GLint boundTex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);
    glBindTexture(GL_TEXTURE_2D, texture->mTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, texture->mGLFormat, texture->mGLType, data);
    glBindTexture(GL_TEXTURE_2D, boundTex);

    return true;
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
    NFE_UNUSED(instanceOffset);

    if (mConstantBufferNeedsUpdate)
        BindConstantBuffer();

    if (mVertexBufferNeedsUpdate || mVertexLayoutNeedsUpdate)
        BindVertexBufferAndLayout();

    if (mTextureNeedsUpdate)
        BindTexture();

    if (mSamplerNeedsUpdate)
        BindSampler();

    if (instancesNum >= 0)
        glDrawArraysInstanced(TranslatePrimitiveType(type), vertexOffset, vertexNum,
                              instancesNum);
    else
        glDrawArrays(TranslatePrimitiveType(type), vertexOffset, vertexNum);
}

void CommandBuffer::DrawIndexed(PrimitiveType type, int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    NFE_UNUSED(vertexOffset);
    NFE_UNUSED(instanceOffset);

    if (mConstantBufferNeedsUpdate)
        BindConstantBuffer();

    if (mVertexBufferNeedsUpdate || mVertexLayoutNeedsUpdate)
        BindVertexBufferAndLayout();

    if (mIndexBufferNeedsUpdate)
        BindIndexBuffer();

    if (mTextureNeedsUpdate)
        BindTexture();

    if (mSamplerNeedsUpdate)
        BindSampler();

    int bytePerIndex = mCurrentIndexBufferFormat == GL_UNSIGNED_SHORT ? 2 : 4;

    if (instancesNum >= 0)
        glDrawElementsInstanced(TranslatePrimitiveType(type), indexNum, mCurrentIndexBufferFormat,
                        reinterpret_cast<void*>(static_cast<size_t>(indexOffset * bytePerIndex)),
                        instancesNum);
    else
        glDrawElements(TranslatePrimitiveType(type), indexNum, mCurrentIndexBufferFormat,
                       reinterpret_cast<void*>(static_cast<size_t>(indexOffset * bytePerIndex)));
}

ICommandList* CommandBuffer::Finish()
{
    return nullptr;
}

void CommandBuffer::Execute(ICommandList* commandList)
{
    NFE_UNUSED(commandList);
}

void CommandBuffer::BeginDebugGroup(const char* text)
{
    NFE_UNUSED(text);
}

void CommandBuffer::EndDebugGroup()
{
}

void CommandBuffer::InsertDebugMarker(const char* text)
{
    NFE_UNUSED(text);
}

} // namespace Renderer
} // namespace NFE
