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
namespace Resource {

class MultiPipelineState
{
    NFE_MAKE_NONCOPYABLE(MultiPipelineState)
    NFE_MAKE_NONMOVEABLE(MultiPipelineState)

private:
    struct ShaderSet
    {
        Renderer::ShaderPtr shaders[NFE_GRAPHICS_SHADER_TYPES_NUM];
    };

    std::string mName;

    typedef std::unique_ptr<Multishader, void(*)(Multishader*)> ShaderResourcePtr;
    ShaderResourcePtr mShaderResources[NFE_GRAPHICS_SHADER_TYPES_NUM];

    std::vector<MultishaderMacro> mMacros;
    /// don't keep names along with ranges - it's bad for cache
    std::vector<std::string> mMacroNames;

    /**
     * Macro mapping table.
     * Maps the pipeline state's macro IDs to each shader macro's ID.
     * Some of the values can be -1, which means that the shader does not use shader macro
     * from the pipeline state.
     */
    std::vector<int> mShaderMacroMapping[NFE_GRAPHICS_SHADER_TYPES_NUM];

    std::vector<Renderer::PipelineStatePtr> mSubPipelineStates;
    std::vector<ShaderSet> mShaderSets;

    void GenerateShaderSets();
    void LoadShaderSet(int* macroValues);

public:
    MultiPipelineState();

    /**
     * Get number of macros used in this multi pipeline state.
     */
    size_t GetMacrosNumber() const;

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
    const Renderer::PipelineStatePtr& GetPipelineState(int* values = nullptr) const;

    /**
     * Get sub shader defined by a list of macro values and shader type.
     *
     * @param type   Shader type.
     * @param values Array of macro values. The array size must be equal to number of macros
     *               in the multi pipeline state obtained with @p GetMacrosNumber() method
     *               or set to NULL to use default macro values.
     * @return IShader interface pointer.
     */
    const Renderer::ShaderPtr& GetShader(Renderer::ShaderType type, int* values = nullptr) const;

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
    bool Build(const Renderer::PipelineStateDesc& desc);

    /**
     * Get low-level renderer's shader slot ID by name.
     *
     * @return Negative value on error.
     */
    int GetResourceSlotByName(const char* slotName);
};

} // namespace Resource
} // namespace NFE
