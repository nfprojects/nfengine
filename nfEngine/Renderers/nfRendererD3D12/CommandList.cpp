/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Implementation of CommandListManager
 */

#include "PCH.hpp"
#include "CommandListManager.hpp"
#include "CommandList.hpp"
#include "RendererD3D12.hpp"


namespace NFE {
namespace Renderer {

CommandList::CommandList(CommandListManager* manager, ID3D12GraphicsCommandList* d3dCommandList)
    : mManager(manager)
    , mD3DCommandList(d3dCommandList)
    , ringBuffer(nullptr)
{ }

CommandList::~CommandList()
{
    mManager->OnDestroyCommandList(this);
}

} // namespace Renderer
} // namespace NFE
