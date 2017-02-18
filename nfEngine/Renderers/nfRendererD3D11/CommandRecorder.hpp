/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 render's command buffer.
 */

#pragma once

#include "../RendererInterface/CommandRecorder.hpp"
#include "Common.hpp"


namespace NFE {
namespace Renderer {

// predeclarations
class Shader;
class RenderTarget;
class PipelineState;
class ComputePipelineState;
class ResourceBindingLayout;


class CommandList : public ICommandList
{
public:
    D3DPtr<ID3D11CommandList> mD3DList;
};

class CommandRecorder : public ICommandRecorder
{
    friend class Device;

    bool mReset;
    D3DPtr<ID3D11DeviceContext> mContext;
    D3DPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotation;

    // graphics pipeline
    unsigned char mStencilRef;
    unsigned char mCurrentStencilRef;
    RenderTarget* mCurrentRenderTarget;
    PrimitiveType mCurrentPrimitiveType;
    ResourceBindingLayout* mBindingLayout;
    PipelineState* mPipelineState;
    PipelineState* mCurrentPipelineState;
    ID3D11VertexShader* mBoundVertexShader;
    ID3D11HullShader* mBoundHullShader;
    ID3D11DomainShader* mBoundDomainShader;
    ID3D11GeometryShader* mBoundGeometryShader;
    ID3D11PixelShader* mBoundPixelShader;

    // compute pipeline
    ResourceBindingLayout* mComputeBindingLayout;
    ComputePipelineState* mComputePipelineState;
    ID3D11ComputeShader* mBoundComputeShader;

    void UpdateSamplers();
    void UpdateState();

public:
    CommandRecorder(ID3D11DeviceContext* deviceContext);
    ~CommandRecorder();

    /// Common methods
    void Reset() override;
    void* MapBuffer(IBuffer* buffer, MapType type) override;
    void UnmapBuffer(IBuffer* buffer) override;
    bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) override;
    void CopyTexture(ITexture* src, ITexture* dest) override;
    std::unique_ptr<ICommandList> Finish() override;

    /// Graphics pipeline methods
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
    void Clear(int flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors,
               float depthValue, uint8 stencilValue) override;
    void Draw(int vertexNum, int instancesNum, int vertexOffset,
              int instanceOffset) override;
    void DrawIndexed(int indexNum, int instancesNum, int indexOffset,
                     int vertexOffset, int instanceOffset) override;

    /// Compute pipeline methods
    void BindComputeResources(size_t slot, IResourceBindingInstance* bindingSetInstance) override;
    void BindComputeVolatileCBuffer(size_t slot, IBuffer* buffer) override;
    void SetComputeResourceBindingLayout(IResourceBindingLayout* layout) override;
    void SetComputePipelineState(IComputePipelineState* state) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;

    /// Debugging

    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;
};

} // namespace Renderer
} // namespace NFE
