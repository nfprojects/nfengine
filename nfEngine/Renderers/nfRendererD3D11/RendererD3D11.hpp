/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declaration of Direct3D 11 rendering backend.
 */

#pragma once

#include "Device.hpp"

#ifdef RENDERERD3D11_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif


namespace NFE {
namespace Renderer {

extern Common::UniquePtr<Device> gDevice;

// export Device creation function
extern "C" RENDERER_API IDevice* Init(const DeviceInitParams* params);
extern "C" RENDERER_API void Release();

} // namespace Renderer
} // namespace NFE
