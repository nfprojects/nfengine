/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D12 implementation of renderer's lowlevel API
 */

#include "PCH.hpp"
#include "RendererD3D12.hpp"

namespace NFE {
namespace Renderer {

std::unique_ptr<Device> gDevice;

IDevice* Init()
{
    if (gDevice == nullptr)
    {
        gDevice.reset(new Device);
        if (!gDevice->Init())
            gDevice.reset();
    }

    return gDevice.get();
}

void Release()
{
    gDevice.reset();
}

} // namespace Renderer
} // namespace NFE
