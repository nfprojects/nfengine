/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance utilities definitions.
 */

#include "PCH.hpp"
#include "Performance.hpp"

namespace NFE {
namespace Util {

FrameStats g_FrameStats;

FrameStats::FrameStats()
{
    deltaTime = 0.0;
    Reset();
}

void FrameStats::Reset()
{
    deltaTime.Step();
    physics.Step();
    shadowsRendering.Step();

    renderedMeshes = 0;
}

} // namespace Util
} // namespace NFE
