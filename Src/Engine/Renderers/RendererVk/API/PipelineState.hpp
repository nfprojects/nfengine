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
        uint32 binding;
    };

    using DescriptorBindings = Common::StaticArray<DescriptorMetadata, VK_MAX_BINDINGS_PER_SET>;

    struct DescriptorSetMetadata
    {
        VkShaderStageFlags stage;
        VkDescriptorType type;
        uint32 set;
        DescriptorBindings bindings;
    };

    PipelineStateDesc mDesc;
    Common::StaticArray<ShaderPtr, VK_MAX_SHADER_STAGES> mShaders;
    Common::StaticArray<DescriptorSetMetadata, VK_MAX_DESCRIPTOR_SETS> mDescriptorSetMetadata;
    Common::StaticArray<VkPipelineShaderStageCreateInfo, VK_MAX_SHADER_STAGES> mShaderStageDescs;
    DescriptorSetLayoutCollection mDescriptorSetLayouts;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;

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
