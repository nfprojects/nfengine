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

IDevice* Init(const DeviceInitParams* params)
{
    if (gDevice == nullptr)
    {
        gDevice = std::make_unique<Device>();
        if (!gDevice->Init(params))
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
