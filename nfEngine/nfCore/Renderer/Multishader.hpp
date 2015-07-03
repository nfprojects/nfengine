/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Multishader utility
 */

#pragma once

#include "RendererModule.hpp"
#include "HighLevelRenderer.hpp"
#include "RenderCommand.hpp"

namespace NFE {
namespace Renderer {


struct MultishaderDefine
{
    std::string name;
    int minValue;
    int maxValue;
    int defaultValue;
};

class Multishader
{
    std::string mName;
    std::vector<MultishaderDefine> mDefines;
    std::vector<std::unique_ptr<IShader>> mSubShaders;
    ShaderType mType;

    /// disable unwanted methods
    Multishader(const Multishader&) = delete;
    Multishader& operator=(const Multishader&) = delete;

    bool LoadSubshader(int* defineValues);

public:
    Multishader();

    /**
     * Load a multishader.
     *
     * @param name Multishader name.
     */
    bool Load(const char* name);

    /**
     * Get shader macro definition by name.
     *
     * @param name Macro definition name
     * @return Macro definition handle or negative value if not found.
     */
    int GetDefineByName(const char* name) const;

    /**
     * Get subshader by list of macro values.
     *
     * @param defines    Array of define handles obtained with @p GetDefineByName method.
     * @param values     Array of macro values.
     * @param definesNum Number of macro values.
     *
     * @return Shader interface pointer.
     */
    IShader* GetShader(int* defines = nullptr, int* values = nullptr,
                       size_t definesNum = 0) const;

    /**
     * Get shader type.
     */
    NFE_INLINE ShaderType GetType() const
    {
        return mType;
    }
};

} // namespace Renderer
} // namespace NFE
