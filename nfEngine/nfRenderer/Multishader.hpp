/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Definitions of Multishader utility
 */

#pragma once

#include "nfRenderer.hpp"
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
public:
    Multishader();

    bool OnLoad();
    void OnUnload();

    /**
     * Get number of macros used in the multishader.
     */
    uint32 GetMacrosNumber() const;

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
    const ShaderPtr& GetShader(int* values) const;

    /**
     * Get shader name.
     */
    NFE_INLINE const Common::String& GetName() const
    {
        return mName;
    }

    /**
     * Get shader type.
     */
    NFE_INLINE ShaderType GetType() const
    {
        return mType;
    }

private:

    // shader name
    Common::String mName;
    ShaderType mType;

    Common::DynArray<MultishaderMacro> mMacros;

    // don't keep names along with ranges - it's bad for cache
    Common::DynArray<Common::String> mMacroNames;

    Common::DynArray<ShaderPtr> mSubShaders;

    bool LoadSubshader(int* macroValues);
};

} // namespace Renderer
} // namespace NFE
