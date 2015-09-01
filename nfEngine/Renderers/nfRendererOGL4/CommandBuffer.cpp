/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Definitions of OpenGL 4 Command Buffer
 */

#include "PCH.hpp"

#include "Defines.hpp"
#include "CommandBuffer.hpp"
#include "Shader.hpp"
#include "Translations.hpp"


namespace NFE {
namespace Renderer {

CommandBuffer::CommandBuffer()
    : mCurrentRenderTarget(nullptr)
    , mSSOEnabled(false)
    , mProgramPipeline(GL_NONE)
{
}

CommandBuffer::~CommandBuffer()
{
    if (mProgramPipeline)
        glDeleteProgramPipelines(1, &mProgramPipeline);
}

void CommandBuffer::Reset()
{
    mCurrentRenderTarget = nullptr;

    glUseProgram(GL_NONE);
    glBindProgramPipeline(GL_NONE);
    if (mProgramPipeline)
    {
        glDeleteProgramPipelines(1, &mProgramPipeline);
        mProgramPipeline = GL_NONE;
    }
    mSSOEnabled = false;
}

void CommandBuffer::SetVertexLayout(IVertexLayout* vertexLayout)
{
    UNUSED(vertexLayout);
}

void CommandBuffer::SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets)
{
    UNUSED(num);
    UNUSED(vertexBuffers);
    UNUSED(strides);
    UNUSED(offsets);
}

void CommandBuffer::SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format)
{
    UNUSED(indexBuffer);
    UNUSED(format);
}

void CommandBuffer::SetSamplers(ISampler** samplers, int num, ShaderType target)
{
    UNUSED(samplers);
    UNUSED(num);
    UNUSED(target);
}

void CommandBuffer::SetTextures(ITexture** textures, int num, ShaderType target)
{
    UNUSED(textures);
    UNUSED(num);
    UNUSED(target);
}

void CommandBuffer::SetConstantBuffers(IBuffer** constantBuffers, int num, ShaderType target)
{
    UNUSED(constantBuffers);
    UNUSED(num);
    UNUSED(target);
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
        // generate program pipeline for further use
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
    UNUSED(buffer);
    UNUSED(offset);
    UNUSED(size);
    UNUSED(data);

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
    UNUSED(type);
    UNUSED(vertexNum);
    UNUSED(instancesNum);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
}

void CommandBuffer::DrawIndexed(PrimitiveType type, int indexNum, int instancesNum,
                                int indexOffset, int vertexOffset, int instanceOffset)
{
    UNUSED(type);
    UNUSED(indexNum);
    UNUSED(instancesNum);
    UNUSED(indexOffset);
    UNUSED(vertexOffset);
    UNUSED(instanceOffset);
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
