/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Declarations of Vulkan pipeline states.
 */

#pragma once

#include "../RendererCommon/PipelineState.hpp"
#include "Defines.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

class PipelineState : public IPipelineState
{
    friend class CommandRecorder;

    struct DescriptorMetadata
    {
        VkDescriptorType type;
        uint32 binding;

        DescriptorMetadata() = default;
        DescriptorMetadata(SpvReflectDescriptorType type, uint32 binding)
            : type(TranslateSpvReflectDescriptorTypeToVkDescriptorType(type))
            , binding(binding)
        {
        }
    };

    using DescriptorBindings = Common::StaticArray<DescriptorMetadata, VK_MAX_BINDINGS_PER_SET>;

    struct DescriptorSetMetadataEntry
    {
        VkShaderStageFlagBits stage;
        uint32 set;
        DescriptorBindings bindings;

        DescriptorSetMetadataEntry() = default;
        DescriptorSetMetadataEntry(VkShaderStageFlagBits stage, uint32 set)
            : stage(stage)
            , set(set)
            , bindings()
        {
        }
    };

    struct VolatileResourceMetadataEntry
    {
        VkShaderStageFlagBits stage;
        uint32 binding;

        VolatileResourceMetadataEntry() = default;
        VolatileResourceMetadataEntry(ShaderType stage, uint32 binding)
            : stage(TranslateShaderTypeToVkShaderStage(stage))
            , binding(binding)
        {
        }
    };

    using Shaders = Common::StaticArray<ShaderPtr, VK_MAX_SHADER_STAGES>;
    using DescriptorSetMetadata = Common::StaticArray<DescriptorSetMetadataEntry, VK_MAX_DESCRIPTOR_SETS>;
    using ShaderStageDescs = Common::StaticArray<VkPipelineShaderStageCreateInfo, VK_MAX_SHADER_STAGES>;
    using VolatileResourceMetadata = Common::StaticArray<VolatileResourceMetadataEntry, VK_MAX_VOLATILE_BUFFERS>;

    PipelineStateDesc mDesc;
    Shaders mShaders;
    ShaderStageDescs mShaderStageDescs;
    DescriptorSetMetadata mDescriptorSetMetadata;
    VolatileResourceMetadata mVolatileResourceMetadata;
    DescriptorSetLayoutCollection mDescriptorSetLayouts;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;

    void CollectVolatileResourceMetadata();
    bool MapToDescriptorSet(const ShaderPtr& shader, SpvReflectDescriptorType type, uint32& set);
    bool RemapDescriptorSets();
    bool CreateDescriptorSetLayouts();
    VkShaderModule CreateShaderModule(const SpvReflectShaderModule& shaderSpv);
    bool PrepareShaderStage(const ShaderPtr& shader);
    bool CreatePipelineLayout();
    bool FormShaderModules();

public:
    PipelineState();
    ~PipelineState();
    bool Init(const PipelineStateDesc& desc);
};

} // namespace Renderer
} // namespace NFE
