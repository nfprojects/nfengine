/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's command buffer.
 */

#pragma once

#include "../RendererInterface/CommandBuffer.hpp"
#include "Common.hpp"
#include "ResourceBinding.hpp"

namespace NFE {
namespace Renderer {

class Shader;
class RenderTarget;
class PipelineState;

class CommandList : public ICommandList
{
public:
    D3DPtr<ID3D11CommandList> mD3DList;
};

class CommandBuffer : public ICommandBuffer
{
    friend class Device;

    bool mReset;
    unsigned char mStencilRef;
    unsigned char mCurrentStencilRef;
    RenderTarget* mCurrentRenderTarget;
    PrimitiveType mCurrentPrimitiveType;
    ResourceBindingLayout* mBindingLayout;
    PipelineState* mPipelineState;
    PipelineState* mCurrentPipelineState;
    D3DPtr<ID3D11DeviceContext> mContext;
    D3DPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotation;

    ID3D11VertexShader* mBoundVertexShader;
    ID3D11HullShader* mBoundHullShader;
    ID3D11DomainShader* mBoundDomainShader;
    ID3D11GeometryShader* mBoundGeometryShader;
    ID3D11PixelShader* mBoundPixelShader;

    void UpdateSamplers();
    void UpdateState();

public:
    CommandBuffer(ID3D11DeviceContext* deviceContext);
    ~CommandBuffer();

    /// Shader resources setup methods

    void Reset() override;
    void SetVertexBuffers(int num, IBuffer** vertexBuffers, int* strides, int* offsets) override;
    void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format) override;
    void BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance) override;
    void BindVolatileCBuffer(size_t slot, IBuffer* buffer) override;
    void SetResourceBindingLayout(IResourceBindingLayout* layout) override;
    void SetRenderTarget(IRenderTarget* renderTarget) override;
    void SetPipelineState(IPipelineState* state) override;
    void SetStencilRef(unsigned char ref) override;
    void SetViewport(float left, float width, float top, float height,
                     float minDepth, float maxDepth) override;
    void SetScissors(int left, int top, int right, int bottom) override;

    /// "Executive" methods

    void* MapBuffer(IBuffer* buffer, MapType type) override;
    void UnmapBuffer(IBuffer* buffer) override;
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(ITexture* src, ITexture* dest) override;
    void Clear(int flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors,
               float depthValue, uint8 stencilValue) override;
    void Draw(int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset) override;
    void DrawIndexed(int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset) override;
    std::unique_ptr<ICommandList> Finish() override;

    /// Debugging

    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;
};

} // namespace Renderer
} // namespace NFE
