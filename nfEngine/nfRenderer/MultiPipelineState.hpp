/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of MultiPipelineState utility
 */

#pragma once

#include "nfRenderer.hpp"
#include "HighLevelRenderer.hpp"
#include "RenderCommand.hpp"
#include "Multishader.hpp"

namespace NFE {
namespace Renderer {

class MultiPipelineState
{
    NFE_MAKE_NONCOPYABLE(MultiPipelineState)
    NFE_MAKE_NONMOVEABLE(MultiPipelineState)

public:
    MultiPipelineState();

    /**
     * Get number of macros used in this multi pipeline state.
     */
    uint32 GetMacrosNumber() const;

    /**
     * Get macro ID by name.
     *
     * @param name Macro definition name
     * @return Macro definition handle or negative value if not found.
     */
    int GetMacroByName(const char* name) const;

    /**
     * Get sub pipeline state defined by a list of macro values.
     *
     * @param values Array of macro values. The array size must be equal to number of macros
     *               in the multishader obtained with @p GetMacrosNumber() method or set to
     *               NULL to use default macro values.
     * @param type   Shader type.
     * @return IPipelineState interface pointer.
     */
    const PipelineStatePtr& GetPipelineState(int* values = nullptr) const;

    /**
     * Get sub shader defined by a list of macro values and shader type.
     *
     * @param type   Shader type.
     * @param values Array of macro values. The array size must be equal to number of macros
     *               in the multi pipeline state obtained with @p GetMacrosNumber() method
     *               or set to NULL to use default macro values.
     * @return IShader interface pointer.
     */
    const ShaderPtr& GetShader(ShaderType type, int* values = nullptr) const;

    /**
     * Load multishader from config file. This operation only parses config file and
     * loads shaders. No pipeline state object creation is performed.
     * @return True on success.
     */
    bool Load(const char* name);

    /**
     * Build pipeline state objects state.
     * @return True on success.
     */
    bool Build(const PipelineStateDesc& desc);

    /**
     * Get low-level renderer's shader slot ID by name.
     *
     * @return Negative value on error.
     */
    int GetResourceSlotByName(const char* slotName);

private:

    struct ShaderSet
    {
        ShaderPtr shaders[NFE_GRAPHICS_SHADER_TYPES_NUM];
    };

    Common::String mName;

    using ShaderResourcePtr = std::unique_ptr<Multishader, void(*)(Multishader*)>;
    ShaderResourcePtr mShaderResources[NFE_GRAPHICS_SHADER_TYPES_NUM];

    Common::DynArray<MultishaderMacro> mMacros;
    Common::DynArray<Common::String> mMacroNames;

    /**
     * Macro mapping table.
     * Maps the pipeline state's macro IDs to each shader macro's ID.
     * Some of the values can be -1, which means that the shader does not use shader macro
     * from the pipeline state.
     */
    Common::DynArray<int> mShaderMacroMapping[NFE_GRAPHICS_SHADER_TYPES_NUM];

    Common::DynArray<PipelineStatePtr> mSubPipelineStates;
    Common::DynArray<ShaderSet> mShaderSets;

    void GenerateShaderSets();
    void LoadShaderSet(int* macroValues);
};

} // namespace Renderer
} // namespace NFE
