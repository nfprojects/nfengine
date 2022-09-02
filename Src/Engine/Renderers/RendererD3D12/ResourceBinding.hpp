/**
 * @file
 * @author  Witek902
 * @brief   Declaration of Direct3D 12 renderer's shader resource binding
 */

#pragma once

#include "Resource.hpp"
#include "../../Common/Containers/DynArray.hpp"


// max supported number of volatile constant buffers
#define NFE_RENDERER_MAX_VOLATILE_CBUFFERS 4

namespace NFE {
namespace Renderer {


class Texture;
class Buffer;
struct TextureView;
struct BufferView;

class ResourceBindingState
{

};

void CreateTextureSRV(const Texture* texture, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
void CreateTextureUAV(const Texture* texture, const TextureView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
void CreateBufferSRV(const Buffer* buffer, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);
void CreateBufferUAV(const Buffer* buffer, const BufferView& view, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);

} // namespace Renderer
} // namespace NFE
