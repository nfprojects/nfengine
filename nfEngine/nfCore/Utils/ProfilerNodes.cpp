/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler definitions
 */

#include "PCH.hpp"

#include "Profiler.hpp"

namespace NFE {
namespace Util {

PROFILER_REGISTER_ROOT_NODE("Renderer", RendererNode);
    PROFILER_REGISTER_NODE("Geometry", GeometryRendererNode, RendererNode);
    PROFILER_REGISTER_NODE("GUI", GuiRendererNode, RendererNode);
    PROFILER_REGISTER_NODE("Shadows", ShadowsRendererNode, RendererNode);
        PROFILER_REGISTER_NODE("OmniShadows", OmniShadowsRendererNode, ShadowsRendererNode);
        PROFILER_REGISTER_NODE("SpotShadows", SpotShadowsRendererNode, ShadowsRendererNode);

} // namespace Util
} // namespace NFE
