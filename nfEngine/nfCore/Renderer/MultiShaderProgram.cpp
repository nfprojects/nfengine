/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of MultiShaderProgram utility
 */

#pragma once

#include "../PCH.hpp"
#include "MultiShaderProgram.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/File.hpp"
#include "../rapidjson/include/rapidjson/document.h"
#include "../rapidjson/include/rapidjson/filestream.h"

namespace NFE {
namespace Renderer {

namespace {

const std::string gShadersRoot = "nfEngine/Shaders/";

} // namespace

int MultiShaderProgram::GetMacroByName(const char* name) const
{

}

const SubShaderProgram& MultiShaderProgram::GetSubShaderProgram(int numMacros, int* macroIDs,
                                                                int* macroValues) const
{

}

} // namespace Renderer
} // namespace NFE
