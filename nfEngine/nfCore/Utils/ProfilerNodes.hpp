/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler declarations
 */

#pragma once

#include "Profiler.hpp"

namespace NFE {
namespace Util {

PROFILER_DECLARE_NODE(RendererNode);
    PROFILER_DECLARE_NODE(GeometryRendererNode);
    PROFILER_DECLARE_NODE(GuiRendererNode);
    PROFILER_DECLARE_NODE(ShadowsRendererNode);
        PROFILER_DECLARE_NODE(OmniShadowsRendererNode);
        PROFILER_DECLARE_NODE(SpotShadowsRendererNode);

} // namespace Util
} // namespace NFE
