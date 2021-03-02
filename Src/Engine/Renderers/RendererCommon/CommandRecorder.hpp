/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "PipelineState.hpp"
#include "ComputePipelineState.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "ResourceBinding.hpp"

#include <memory>


#define NFE_RENDERER_MAX_VERTEX_BUFFERS 4

namespace NFE {
namespace Renderer {

enum class CommandQueueType : uint8;

/**
 * Recorded command list, ready to execute.
 */
class ICommandList
{
public:
    virtual ~ICommandList() {}
};

using CommandListPtr = Common::UniquePtr<ICommandList>;


enum ClearFlags
{
    ClearFlagsColor     = (1 << 0),
    ClearFlagsDepth     = (1 << 1),
    ClearFlagsStencil   = (1 << 2),
};

struct TextureRegion
{
    uint16 mipmap = 0;
    uint16 layer = 0;

    // region location (offset)
    uint32 x = 0;
    uint32 y = 0;
    uint32 z = 0;

    // region size
    uint32 width = 1;
    uint32 height = 1;
    uint32 depth = 1;
};

enum class PipelineType
{
    Graphics,
    Compute,
};

/**
 * Interface allowing to control rendering pipeline state and executing rendering commands.
 */
class ICommandRecorder
{
public:
    virtual ~ICommandRecorder() {}

    /**
     * CommandBufferCommon      Common methods.
     * @{
     */

    /**
     * Turn the command recorder into recording state.
     *
     * @return True on success.
     */
    virtual bool Begin(CommandQueueType queueType) = 0;

    /**
     * Store all recorded commands to a command list and turn the command recorder into
     * non-recording state.
     *
     * @return Recorded Command List or nullptr if recording had errors.
     *
     * @note Command buffer must be in recording state for the method to succeed.
     * @remarks Generated Command List can be executed only once.
     *
     * @see @p Reset()
     */
    virtual CommandListPtr Finish() = 0;

    /**
     * Write data from the CPU memory to a GPU buffer.
     * @param buffer Target buffer.
     * @param offset Offset in the GPU buffer (in bytes).
     * @param size   Number of bytes to write.
     * @param data   Pointer to source CPU buffer.
     * @return true on success.
     */
    virtual bool WriteBuffer(const BufferPtr& buffer, size_t offset, size_t size, const void* data) = 0;

    /**
     * Write data from the CPU memory to a GPU texture.
     * @param texture       Target texture
     * @param data          Pointer to source CPU texture data
     * @param texRegion     Optional texture write region. If null, the whole texture is updated.
     * @param srcRowStride  Optional row stride of the source CPU data.
     * @return true on success.
     */
    virtual bool WriteTexture(const TexturePtr& texture, const void* data, const TextureRegion* texRegion = nullptr, uint32 srcRowStride = 0) = 0;

    /**
     * Copy contents of buffer @p src to @p dest.
     * @param src           Source buffer object
     * @param dest          Destination buffer object
     * @param size          Number of bytes to copy. If zero specified, source buffer size is assumed
     * @param srcOffset     Offset in source buffer, in bytes
     * @param destOffset    Offset in destination buffer, in bytes
     */
    virtual void CopyBuffer(const BufferPtr& src, const BufferPtr& dest, size_t size = 0, size_t srcOffset = 0, size_t destOffset = 0) = 0;

    /**
     * Copy contents of texture @p src to @p dest.
     * The textures source and destination texture must be the same type, size, format
     * and sample count.
     * @param src  Source texture object.
     * @param dest Destination texture object.
     */
    virtual void CopyTexture(const TexturePtr& src, const TexturePtr& dest) = 0;

    /**
     * Copy region of a texture @p src to a buffer @p dest.
     * @param src           Source texture object.
     * @param dest          Destination buffer object.
     * @param texRegion     Optional texture read region. If null, the whole texture is read.
     * @param bufferOffset  Destination buffer offset.
     */
    virtual void CopyTextureToBuffer(const TexturePtr& src, const BufferPtr& dest, const TextureRegion* texRegion = nullptr, uint32 bufferOffset = 0) = 0;

    /**
     * Copy contents of texture @p src to backbuffer @p dest.
     * @param src  Source texture object.
     * @param dest Destination backbuffer object.
     */
    virtual void CopyTexture(const TexturePtr& src, const BackbufferPtr& dest) = 0;

    /**@}*/



    /**
     * CommandBufferBinding     Resource binding methods.
     * @{
     */

    /**
     * Bind shader resources via setting a binding set instance.
     * @param setIndex              Target binding set slot within current binding layout.
     * @param bindingSetInstance    Binding set instance to be bound to the pipeline or NULL
     *                              to clear all bound resources for this set.
     */
    virtual void BindResources(PipelineType pipelineType, uint32 setIndex, const ResourceBindingInstancePtr& bindingSetInstance) = 0;

    /**
     * Bind texture directly without using binding set instance.
     * Only given resource set slot will be affected, all other bound resources will be unaffected.
     * @param setIndex              Target binding set slot within current binding layout.
     * @param slotInSet             Resource slot in given binding set.
     * @param texture,view          Texture and its view to be bound
     */
    virtual void BindTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view = TextureView()) = 0;
    virtual void BindWritableTexture(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const TexturePtr& texture, const TextureView& view = TextureView()) = 0;

    /**
     * Bind a buffer directly without using binding set instance.
     * Only given resource set slot will be affected, all other bound resources will be unaffected.
     * @param setIndex              Target binding set slot within current binding layout.
     * @param slotInSet             Resource slot in given binding set.
     * @param buffer,view           Buffer and its view to be bound
     */
    virtual void BindBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view = BufferView()) = 0;
    virtual void BindWritableBuffer(PipelineType pipelineType, uint32 setIndex, uint32 slotInSet, const BufferPtr& buffer, const BufferView& view = BufferView()) = 0;

    /**
     * Bind dynamic buffer to the graphics pipeline.
     * @param slot      Dynamic buffer slot in the current resource binding layout.
     * @param buffer    Buffer to bind.
     */
    virtual void BindVolatileCBuffer(PipelineType pipelineType, uint32 slot, const BufferPtr& buffer) = 0;

    /**
     * Set new shaders resources binding layout for graphics pipeline.
     * @param layout Resource binding layout
     */
    virtual void SetResourceBindingLayout(PipelineType pipelineType, const ResourceBindingLayoutPtr& layout) = 0;

    /**@}*/



    /**
     * CommandBufferGraphics    Graphics pipeline methods.
     * @{
     */

    virtual void SetVertexBuffers(uint32 num, const BufferPtr* vertexBuffers, uint32* strides, uint32* offsets) = 0;
    virtual void SetIndexBuffer(const BufferPtr& indexBuffer, IndexBufferFormat format) = 0;

    /**
     * Bind render target object.
     * Pass nullptr to unbind current render target.
     */
    virtual void SetRenderTarget(const RenderTargetPtr& renderTarget) = 0;

    /**
     * Set graphics pipeline state.
     * @remarks     When draw call is executed, a matching Resource Binding Layout must be set.
     * @param state New graphics pipeline state.
     */
    virtual void SetPipelineState(const PipelineStatePtr& state) = 0;

    virtual void SetStencilRef(uint8 ref) = 0;
    virtual void SetViewport(float left, float width, float top, float height, float minDepth, float maxDepth) = 0;
    virtual void SetScissors(int32 left, int32 top, int32 right, int32 bottom) = 0;

    /**
     * Clear bound render targets with a color.
     * @param flags         Bitfield specifying if depth and stencil are also cleared.
     * @param numTargets    Number of render target textures to be cleared.
     * @param slots         Render target textures slots.
     * @param colors        New color values for each texture - 4 element array of floats (RGBA).
     * @param depthValue    New depth value (should be between 0.0f and 1.0f).
     * @param stencilValue  New stencil value.
     */
    virtual void Clear(uint32 flags, uint32 numTargets, const uint32* slots, const Math::Vec4fU* colors,
                       float depthValue = 0.0f, uint8 stencilValue = 0) = 0;

    /**
     * Draw geometry.
     *
     * @param instancesNum   Number of instances to draw. Set to a negative value to disable
     * @param vertexNum      Vertices number per instance.
     *                       instancing.
     * @param vertexOffset   Vertex buffer offset (in elements).
     * @param instanceOffset Per-instance buffer offset (in elements).
     */
    virtual void Draw(uint32 vertexNum, uint32 instancesNum = 1, uint32 vertexOffset = 0, uint32 instanceOffset = 0) = 0;

    /**
     * Draw geometry (with indexed verticies).
     *
     * @param indexNum       Indices number per instance.
     * @param instancesNum   Number of instances to draw. Set to a negative value to disable
     *                       instancing.
     * @param indexOffset    Location of the first index.
     * @param vertexOffset   Vertex buffer offset (in elements).
     * @param instanceOffset Per-instance buffer offset (in elements).
     */
    virtual void DrawIndexed(uint32 indexNum,
                             uint32 instancesNum = 1, uint32 indexOffset = 0,
                             int32 vertexOffset = 0, uint32 instanceOffset = 0) = 0;

    /**@}*/



    /**
     * CommandBufferCompute     Compute pipeline methods.
     * @{
     */

    /**
     * Set compute pipeline state.
     * @remarks     When Dispatch() is executed, a matching compute Resource Binding Layout must be set.
     * @param state New compute pipeline state.
     */
    virtual void SetComputePipelineState(const ComputePipelineStatePtr& state) = 0;

    /**
     * Execute compute shader.
     *
     * @note            Compute pipeline must be set up.
     * @param x,y,z     Size of workgroups matrix.
     */
    virtual void Dispatch(uint32 x = 1, uint32 y = 1, uint32 z = 1) = 0;

    /**
     * Execute compute shader.
     * 
     * @param indirectArgBuffer Buffer containing dispatch params.
     * @param bufferOffset      Offset to the params within the buffer.
     */
    virtual void DispatchIndirect(const BufferPtr& indirectArgBuffer, uint32 bufferOffset = 0) = 0;

    /**@}*/


    /**
     * Instruct the renderer that a resource will be used on a different queue type after the command list finishes.
     * @todo    This is a bit hacky, not sure how to handle this properly for now.
     */
    virtual void HintTargetCommandQueueType(const BufferPtr& resource, const CommandQueueType targetType) = 0;
    virtual void HintTargetCommandQueueType(const TexturePtr& resource, const CommandQueueType targetType) = 0;


    /**
     * CommandBufferDebug   Debugging methods
     * @{
     */

    /**
     * Mark the beginning of a group of commands. Can be used for debugging purposes.
     * Every BeginGroup call must have corresponding GroupEnd call.
     * @param name Text displayed in a debugging tool.
     */
    virtual void BeginDebugGroup(const char* text) = 0;

    /**
     * Mark the ending of a group of commands. Can be used for debugging purposes.
     */
    virtual void EndDebugGroup() = 0;

    /**
     * Insert a custom marker into the command buffer. Can be used for debugging purposes.
     * @param text Text displayed in a debugging tool.
     */
    virtual void InsertDebugMarker(const char* text) = 0;

    /**@}*/
};

} // namespace Renderer
} // namespace NFE
