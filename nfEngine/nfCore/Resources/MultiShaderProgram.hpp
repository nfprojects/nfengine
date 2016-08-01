/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Declarations of MultiShaderProgram utility
 */

#pragma once

#include "Resource.hpp"
#include "../Renderer/HighLevelRenderer.hpp"
#include "../Renderer/RenderCommand.hpp"
#include "Multishader.hpp"

#define NFE_SHADER_TYPES_NUM static_cast<size_t>(Renderer::ShaderType::Number)

namespace NFE {
namespace Resource {

// TODO this class should inherit from ResourceBase
class MultiShaderProgram
{
    NFE_MAKE_NONCOPYABLE(MultiShaderProgram)
    NFE_MAKE_NONMOVEABLE(MultiShaderProgram)

private:
    std::string mName;

    typedef std::unique_ptr<Multishader, void(*)(Multishader*)> ShaderResourcePtr;
    ShaderResourcePtr mShaderResources[NFE_SHADER_TYPES_NUM];

    std::vector<MultishaderMacro> mMacros;
    /// don't keep names along with ranges - it's bad for cache
    std::vector<std::string> mMacroNames;

    /**
     * Macro mapping table.
     * Maps the shader program macro IDs to each shader macro's ID.
     * Some of the values can be -1, which means that the shader does not use shader macro
     * from the shader program.
     */
    std::vector<int> mShaderMacroMapping[NFE_SHADER_TYPES_NUM];

    std::vector<std::unique_ptr<Renderer::IShaderProgram>> mSubPrograms;

    bool GenerateShaderPrograms();
    bool LoadSubShaderProgram(int* macroValues);

public:
    MultiShaderProgram();

    /**
     * Get number of macros used in the multi shader program.
     */
    size_t GetMacrosNumber() const;

    /**
     * Get shader program macro definition by name.
     *
     * @param name Macro definition name
     * @return Macro definition handle or negative value if not found.
     */
    int GetMacroByName(const char* name) const;

    /**
     * Get sub shader program defined by a list of macro values.
     *
     * @param values Array of macro values. The array size must be equal to number of macros
     *               in the multishader obtained with @p GetMacrosNumber() method or set to
     *               NULL to use default macro values.
     * @param type   Shader type.
     * @return IShaderProgram interface pointer.
     */
    Renderer::IShaderProgram* GetShaderProgram(int* values = nullptr) const;

    /**
     * Get sub shader defined by a list of macro values and shader type.
     *
     * @param type   Shader type.
     * @param values Array of macro values. The array size must be equal to number of macros
     *               in the multishader program obtained with @p GetMacrosNumber() method
     *               or set to NULL to use default macro values.
     * @return IShader interface pointer.
     */
    Renderer::IShader* GetShader(Renderer::ShaderType type, int* values = nullptr) const;

    /**
     * Load multishader from config file.
     * @return True on success.
     */
    bool Load(const char* name);

    /**
     * Get low-level renderer's shader program slot ID by name.
     *
     * @return Negative value on error.
     */
    int GetResourceSlotByName(const char* slotName);
};

} // namespace Resource
} // namespace NFE
