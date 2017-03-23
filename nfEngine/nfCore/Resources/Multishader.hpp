/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Multishader utility
 */

#pragma once

#include "Resource.hpp"
#include "../Renderer/RendererModule.hpp"
#include "../Renderer/HighLevelRenderer.hpp"
#include "../Renderer/RenderCommand.hpp"


namespace NFE {
namespace Resource {

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

class Multishader : public ResourceBase
{
    std::vector<MultishaderMacro> mMacros;
    /// don't keep names along with ranges - it's bad for cache
    std::vector<String> mMacroNames;

    std::vector<Renderer::ShaderPtr> mSubShaders;
    Renderer::ShaderType mType;

    bool LoadSubshader(int* macroValues);

public:
    Multishader();

    bool OnLoad();
    void OnUnload();

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
    const Renderer::ShaderPtr& GetShader(int* values) const;

    /**
     * Get shader type.
     */
    NFE_INLINE Renderer::ShaderType GetType() const
    {
        return mType;
    }
};

} // namespace Resource
} // namespace NFE
