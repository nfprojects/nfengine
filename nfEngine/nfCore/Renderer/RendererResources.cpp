/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of high-level renderer resources.
 */

#include "../PCH.hpp"
#include "RendererResources.hpp"
#include "../Renderers/RendererInterface/Device.hpp"

namespace NFE {
namespace Renderer {

void ShadowMap::Release()
{

}

int ShadowMap::Resize(uint32 size, Type type, uint32 splits)
{
    return 0;
}

uint32 ShadowMap::GetSize() const
{
    return 0;
}

} // namespace Renderer
} // namespace NFE
