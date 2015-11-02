/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of high-level Postprocess Renderer
 */

#pragma once

#include "PCH.hpp"
#include "PostProcessRenderer.hpp"

namespace NFE {
namespace Renderer {

// renderer modules instance definition
std::unique_ptr<PostProcessRenderer> PostProcessRenderer::mPtr;

} // namespace Renderer
} // namespace NFE
