/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 12 render's command recorder.
 */

#pragma once

#include "../RendererCommon/CommandRecorder.hpp"
#include "Common.hpp"
#include "RingBuffer.hpp"
#include "ResourceBinding.hpp"
#include "CommandList.hpp"
#include "ResourceStateCache.hpp"
#include "HeapAllocator.hpp"

#include "Engine/Common/Containers/HashSet.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


namespace NFE {
namespace Renderer {

// predeclarations
class RenderTarget;
class PipelineState;
class ComputePipelineState;
class Buffer;
class InternalCommandList;
enum class CommandQueueType : uint8;

class CommandRecorder : public ICommandRecorder
{
public:
    CommandRecorder();
    ~CommandRecorder();

    bool Begin(CommandQueueType queueType) override;
    CommandListPtr Finish() override;

    bool WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data) override;
    bool WriteTexture(const TexturePtr& texture, const void* data, const TextureRegion* texRegion, uint32 srcRowStride) override;
    void CopyBuffer(const BufferPtr& src, const BufferPtr& dest, size_t size, size_t srcOffset, size_t destOffset) override;
    void CopyTexture(const TexturePtr& src, const TexturePtr& dest) override;
    void CopyTextureToBuffer(const TexturePtr& src, const BufferPtr& dest, const TextureRegion* texRegion, uint32 bufferOffset);
    void CopyTexture(const TexturePtr& src, const BackbufferPtr& dest) override;

    /// Resource binding methods
    void BindResources(PipelineType pipelineType, uint32 slot, const ResourceBindingInstancePtr& bindingSetInstance) override;
    void BindTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view) override;
    void BindWritableTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view) override;
    void BindBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view) override;
    void BindWritableBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view) override;
    void BindVolatileCBuffer(PipelineType pipelineType, uint32 slot, const BufferPtr& buffer) override;
    void SetResourceBindingLayout(PipelineType pipelineType, const ResourceBindingLayoutPtr& layout) override;

    /// Graphics pipeline methods
    void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets) override;
    void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) override;
    void SetRenderTarget(const RenderTargetPtr& renderTarget) override;
    void SetPipelineState(const PipelineStatePtr& state) override;
    void SetStencilRef(uint8 ref) override;
    void SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth) override;
    void SetScissors(int32 left, int32 top, int32 right, int32 bottom) override;
    void Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors, float depthValue, uint8 stencilValue) override;
    void Draw(uint32 vertexNum, uint32 instancesNum, uint32 vertexOffset, uint32 instanceOffset) override;
    void DrawIndexed(uint32 indexNum, uint32 instancesNum, uint32 indexOffset, int32 vertexOffset, uint32 instanceOffset) override;

    /// Compute pipeline methods
    void SetComputePipelineState(const ComputePipelineStatePtr& state) override;
    void Dispatch(uint32 x, uint32 y, uint32 z) override;

    /// Debugging
    void BeginDebugGroup(const char* text) override;
    void EndDebugGroup() override;
    void InsertDebugMarker(const char* text) override;

private:

    // TODO instead of having everything duplicated maybe keep some bit mask of what was changed?
    struct PendingDirectResourceBind
    {
        uint32 setIndex;
        uint32 slotInSet;
        ResourceType type;
        bool shaderWritable;

        union
        {
            const Texture* texture;
            const Buffer* buffer;
        };

        union
        {
            TextureView textureView;
            BufferView bufferView;
        };

        PendingDirectResourceBind()
            : setIndex(UINT32_MAX)
            , slotInSet(UINT32_MAX)
            , type(ResourceType::Max)
        {}
    };

    struct ResourceBindingState
    {
        bool bindingLayoutChanged : 1;
        bool bindingInstancesChanged : 1;  // TODO bitmask (one bit per slot)

        const ResourceBindingLayout* bindingLayout;
        const ResourceBindingInstance* bindingInstances[NFE_RENDERER_MAX_BINDING_SETS];
        const Buffer* volatileCBuffers[NFE_RENDERER_MAX_VOLATILE_CBUFFERS];

        // TODO instead of groving array, use static array and bitmask
        Common::DynArray<PendingDirectResourceBind> pendingDirectResourceBinds;

        void Reset();
    };

    NFE_FORCE_INLINE ResourceBindingState& GetBindingState(PipelineType pipelineType)
    {
        return (pipelineType == PipelineType::Compute) ? mComputeBindingState : mGraphicsBindingState;
    }

    void ResetState();

    HeapAllocator::DescriptorRange Internal_GenerateDescriptorTableOverride(ResourceBindingState& state, uint32 setIndex, const ResourceBindingInstance* bindingInstance);

    void Internal_UpdateResourceBindings(PipelineType pipelineType);

    void Internal_UpdateGraphicsPipelineState();
    void Internal_UpdateGraphicsResourceBindingLayout();
    void Internal_UpdateVetexAndIndexBuffers();
    void Internal_PrepareForDraw();

    void Internal_UpdateComputePipelineState();
    void Internal_UpdateComputeResourceBindingLayout();
    void Internal_PrepareForDispatch();

    void Internal_UnsetRenderTarget();

    void Internal_ReferenceBindingSetInstance(const ResourceBindingInstancePtr& bindingSetInstance);

    void Internal_WriteDynamicBuffer(Buffer* buffer, size_t offset, size_t size, const void* data);
    void Internal_WriteVolatileBuffer(Buffer* buffer, const void* data);

    ReferencedResourcesList& Internal_GetReferencedResources();

    CommandQueueType mQueueType;
    InternalCommandListPtr mCommandListObject;
    ID3D12GraphicsCommandList* mCommandList; // cached D3D12 command list pointer

    ResourceStateCache mResourceStateCache;

    ResourceBindingState mGraphicsBindingState;
    ResourceBindingState mComputeBindingState;

    const RenderTarget* mCurrRenderTarget;
    const PipelineState* mGraphicsPipelineState;
    const ComputePipelineState* mComputePipelineState;

    D3D12_PRIMITIVE_TOPOLOGY mCurrPrimitiveTopology;

    bool mGraphicsPipelineStateChanged : 1;
    bool mComputePipelineStateChanged : 1;
    bool mVertexBufferChanged : 1;
    bool mIndexBufferChanged : 1;

    D3D12_INDEX_BUFFER_VIEW mCurrIndexBufferView;
    const Buffer* mBoundIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW mCurrVertexBufferViews[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    const Buffer* mBoundVertexBuffers[NFE_RENDERER_MAX_VERTEX_BUFFERS];
    uint8 mNumBoundVertexBuffers;
};

} // namespace Renderer
} // namespace NFE
