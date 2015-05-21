/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  All engine's global variables declarations.
 */

#pragma once

#include "Core.hpp"
#include "Prerequisites.hpp"

namespace NFE {

/**
 *  Resources paths, relative to executable.
 *  Temporary - This must be changed!!!
 */
extern const std::string g_DataPath;

// High-level renderer
extern std::unique_ptr<Renderer::HighLevelRenderer> gRenderer;

extern std::shared_ptr<Common::ThreadPool> g_pMainThreadPool;
extern Resource::ResManager* g_pResManager;

} // namespace NFE
