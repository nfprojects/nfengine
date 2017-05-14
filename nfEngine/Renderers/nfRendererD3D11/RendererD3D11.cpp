/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   D3D11 implementation of renderer's lowlevel API
 */

#include "PCH.hpp"
#include "RendererD3D11.hpp"

namespace NFE {
namespace Renderer {

Common::UniquePtr<Device> gDevice;

IDevice* Init(const DeviceInitParams* params)
{
    if (gDevice == nullptr)
    {
        gDevice = Common::MakeUniquePtr<Device>();
        if (!gDevice->Init(params))
            gDevice.Reset();
    }

    return gDevice.Get();
}

void Release()
{
    gDevice.Reset();
}

} // namespace Renderer
} // namespace NFE
