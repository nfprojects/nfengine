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


struct MultishaderMacro
{
    int minValue;
    int maxValue;
    int defaultValue;

    MultishaderMacro()
        : minValue(0)
        , maxValue(0)
        , defaultValue(0)
    {}
};

class Multishader
{
    std::string mName;

    std::vector<MultishaderMacro> mMacros;
    /// don't keep names along with ranges - it's bad for cache
    std::vector<std::string> mMacroNames;

    std::vector<std::unique_ptr<IShader>> mSubShaders;
    ShaderType mType;

    /// disable unwanted methods
    Multishader(const Multishader&) = delete;
    Multishader& operator=(const Multishader&) = delete;

    bool LoadSubshader(int* macroValues);

public:
    Multishader();

    /**
     * Load a multishader.
     *
     * @param name Multishader name.
     */
    bool Load(const char* name);

    /**
     * Get number of macros used in the multishader.
     */
    size_t GetMacrosNumber() const;

    /**
     * Get shader macro definition by name.
     *
     * @param name Macro definition name
     * @return Macro definition handle or negative value if not found.
     */
    int GetMacroByName(const char* name) const;

    /**
     * Get subshader by list of macro values.
     *
     * @param  values Array of macro values. The array size must be equal to number of macros
                      in the multishader obtained with @p GetMacrosNumber() method.
                      Must not be null.
     * @return Shader interface pointer.
     */
    IShader* GetShader(int* values) const;

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
