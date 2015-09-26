/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of MultiShaderProgram utility
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {

class SubShaderProgram
{
public:
    IShaderProgram* program;
};


class MultiShaderProgram
{
public:
    MultiShaderProgram(const char* vertexShader, const char* pixelShader, const char* geometryShader);

    /**
     * Get shader macro definition by name.
     *
     * @param name Macro definition name
     * @return Macro definition handle or negative value if not found.
     */
    int GetMacroByName(const char* name) const;

    /**
     * Get SubShaderProgram reference based on macro values.
     */
    const SubShaderProgram& GetSubShaderProgram(int numMacros,
                                                int* macroIDs,
                                                int* macroValues) const;
};

} // namespace Renderer
} // namespace NFE
