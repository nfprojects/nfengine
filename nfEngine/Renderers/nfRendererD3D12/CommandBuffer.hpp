/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's command buffer.
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"
#include "Common.hpp"
#include "RenderTarget.hpp"

namespace NFE {
namespace Renderer {

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    D3DPtr<ID3D12CommandAllocator> mCommandAllocator;
    D3DPtr<ID3D12GraphicsCommandList> mCommandList;

    RenderTarget* mCurrRenderTarget;

    void UnsetRenderTarget();

public:
    CommandBuffer(ID3D12Device* device);
    ~CommandBuffer();

    /// Shader resources setup methods

    void Reset() override;
    void SetVertexLayout(IVertexLayout* vertexLayout) override;
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets) override;
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format) override;
    void SetSamplers(ISampler** samplers, int num, ShaderType target, int slotOffset) override;
    void SetTextures(ITexture** textures, int num, ShaderType target, int slotOffset) override;
    void SetConstantBuffers(IBuffer** constantBuffers, int num,
                            ShaderType target, int slotOffset) override;
    void SetRenderTarget(IRenderTarget* renderTarget) override;
    void SetShaderProgram(IShaderProgram* shaderProgram) override;
    void SetPipelineState(IPipelineState* state) override;
    void SetStencilRef(unsigned char ref) override;
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth) override;
    void SetScissors(int left, int top, int right, int bottom) override;

    /// "Executive" methods

    void* MapBuffer(IBuffer* buffer, MapType type) override;
    void UnmapBuffer(IBuffer* buffer) override;
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) override;
    bool ReadBuffer(IBuffer* buffer, size_t offset, size_t size, void* data) override;
    void CopyTexture(ITexture* src, ITexture* dest) override;
    bool ReadTexture(ITexture* tex, void* data) override;
    void Clear(int flags, const float* color, float depthValue,
               unsigned char stencilValue) override;
    void Draw(PrimitiveType type, int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset) override;
    void DrawIndexed(PrimitiveType type, int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset) override;
    void Execute(ICommandList* commandList) override;

    ICommandList* Finish() override;

    /// Debugging

    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;
};

} // namespace Renderer
} // namespace NFE
