/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of low-level rendering interface.
 */

#pragma once

#include "PipelineState.hpp"
#include "VertexLayout.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Backbuffer.hpp"
#include "RenderTarget.hpp"
#include "ResourceBinding.hpp"

#include "nfCommon/nfCommon.hpp"
#include "nfCommon/Math/Math.hpp"

#include <memory>


#define NFE_RENDERER_MAX_VERTEX_BUFFERS 4

namespace NFE {
namespace Renderer {

class ICommandList
{
public:
    virtual ~ICommandList() {}
};


enum ClearFlags
{
    ClearFlagsColor     = (1 << 0),
    ClearFlagsDepth     = (1 << 1),
    ClearFlagsStencil   = (1 << 2),
};


/**
 * Interface allowing to control rendering pipeline state and executing rendering commands.
 */
class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() {}

    /**
     * CommandBufferResources Shader resources setup methods
     * @{
     */

    /**
     * Reset pipeline state (clean all bound resources and shaders to the pipeline)
     * and turn the command buffer into recording state.
     */
    virtual void Reset() = 0;

    virtual void SetVertexBuffers(int num, IBuffer** vertexBuffers,
                                  int* strides, int* offsets) = 0;
    virtual void SetIndexBuffer(IBuffer* indexBuffer, IndexBufferFormat format) = 0;

    /**
     * Bind shader resources via setting a binding set instance.
     * @param slot               Target binding set slot within current binding layout.
     * @param bindingSetInstance Binding set instance to be bound to the pipeline or NULL
     *                           to clear all bound resources for this set.
     */
    virtual void BindResources(size_t slot, IResourceBindingInstance* bindingSetInstance) = 0;

    /**
     * Bind dynamic buffer to the pipeline.
     * @param slot      Dynamic buffer slot in the current resource binding layout.
     * @param buffer    Buffer to bind.
     */
    virtual void BindDynamicBuffer(size_t slot, IBuffer* buffer) = 0;

    /**
     * Set new shaders resources binding layout.
     * @param layout Resource binding layout
     */
    virtual void SetResourceBindingLayout(IResourceBindingLayout* layout) = 0;

    virtual void SetRenderTarget(IRenderTarget* renderTarget) = 0;
    virtual void SetPipelineState(IPipelineState* state) = 0;
    virtual void SetStencilRef(unsigned char ref) = 0;
    virtual void SetViewport(float left, float width, float top, float height,
                             float minDepth, float maxDepth) = 0;
    virtual void SetScissors(int left, int top, int right, int bottom) = 0;
    /**@}*/

    /**
     * CommandBufferExe Executive methods
     * @{
     */

    /**
     * Map buffer content into process virtual memory.
     * @param buffer Target buffer.
     * @param type   Mapping type.
     * @return Valid pointer on success or NULL on failure.
     */
    virtual void* MapBuffer(IBuffer* buffer, MapType type) = 0;

    /**
     * Unmap buffer content mapped with @p Map method.
     * @param buffer Target buffer.
     */
    virtual void UnmapBuffer(IBuffer* buffer) = 0;

    /**
     * Write data from the CPU memory to a GPU buffer.
     * @param buffer Target buffer.
     * @param offset Offset in the GPU buffer (in bytes).
     * @param size   Number of bytes to write.
     * @param data   Pointer to source CPU buffer.
     * @return true on success.
     */
    virtual bool WriteBuffer(IBuffer* buffer, size_t offset, size_t size, const void* data) = 0;

    /**
     * Copy contents of texture @p src to @p dest.
     * The textures source and destination texture must be the same type, size, format
     * and sample count.
     * @param src  Source texture object.
     * @param dest Destination texture object.
     */
    virtual void CopyTexture(ITexture* src, ITexture* dest) = 0;

    /**
     * Clear bound render targets with a color.
     * @param flags         Bitfield specifying if depth and stencil are also cleared.
     * @param numTargets    Number of render target textures to be cleared.
     * @param slots         Render target textures slots.
     * @param colors        New color values for each texture - 4 element array of floats (RGBA).
     * @param depthValue    New depth value (should be between 0.0f and 1.0f).
     * @param stencilValue  New stencil value.
     */
    virtual void Clear(int flags, uint32 numTargets, const uint32* slots, const Math::Float4* colors,
                       float depthValue = 0.0f, uint8 stencilValue = 0) = 0;

    /**
     * Draw geometry.
     *
     * @param vertexNum      Vertices number per instance.
     * @param instancesNum   Number of instances to draw. Set to a negative value to disable
     *                       instancing.
     * @param vertexOffset   Vertex buffer offset (in elements).
     * @param instanceOffset Per-instance buffer offset (in elements).
     */
    virtual void Draw(int vertexNum, int instancesNum = 1, int vertexOffset = 0, int instanceOffset = 0) = 0;

    /**
     * Draw geometry (with indexed verticies).
     *
     * @param indexNum       Indicies number per instance.
     * @param instancesNum   Number of instances to draw. Set to a negative value to disable
     *                       instancing.
     * @param indexOffset    Location of the first index.
     * @param vertexOffset   Vertex buffer offset (in elements).
     * @param instanceOffset Per-instance buffer offset (in elements).
     */
    virtual void DrawIndexed(int indexNum,
                             int instancesNum = 1, int indexOffset = 0,
                             int vertexOffset = 0, int instanceOffset = 0) = 0;

    /**
     * Store all executed commands to a command list and turn the command buffer into
     * non-recording state.
     * @note Command buffer must be in recording state for the method to succeed.
     * @see @p Reset()
     * @return Saved command list or nullptr on an error.
     */
    virtual std::unique_ptr<ICommandList> Finish() = 0;

    /**@}*/


    /**
     * CommandBufferDebug Debugging methods
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
