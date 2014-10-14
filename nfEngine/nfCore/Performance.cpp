/**
    NFEngine project

    \file   Performance.cpp
    \brief  Performance utilities definitions.
*/

#include "stdafx.hpp"
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
