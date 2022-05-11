#pragma once

#include "../RendererCommon/PipelineState.hpp"
#include "../RendererCommon/ComputePipelineState.hpp"

#include "Defines.hpp"
#include "Shader.hpp"

#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/StaticArray.hpp"


namespace NFE {
namespace Renderer {

class BasePipelineState
{
public:
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

    struct PipelineStateSettings
    {
        bool useTessellation;
        VkPipelineBindPoint bindPoint;
        VolatileResourceMetadata vrMetadata;

        PipelineStateSettings(VkPipelineBindPoint bindPoint)
            : useTessellation(false)
            , bindPoint(bindPoint)
            , vrMetadata()
        {
        }
    };

protected:
    PipelineStateSettings mSettings;
    Shaders mShaders;
    ShaderStageDescs mShaderStageDescs;
    DescriptorSetMetadata mDescriptorSetMetadata;
    DescriptorSetLayoutCollection mDescriptorSetLayouts;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;

    void InitializeSettings(const PipelineStateDesc& desc);
    void InitializeSettings(const ComputePipelineStateDesc& desc);
    bool MapToDescriptorSet(Shader* s, SpvReflectDescriptorType type, uint32& set);
    bool RemapDescriptorSets();
    bool CreateDescriptorSetLayouts();
    VkShaderModule CreateShaderModule(const SpvReflectShaderModule& shaderSpv);
    bool PrepareShaderStage(const ShaderPtr& shader);
    bool CreatePipelineLayout();
    bool FormShaderModules();

public:
    BasePipelineState(VkPipelineBindPoint bindPoint);
    virtual ~BasePipelineState();

    NFE_INLINE const VkPipelineBindPoint& GetBindPoint() const
    {
        return mSettings.bindPoint;
    }

    NFE_INLINE const DescriptorSetLayoutCollection& GetDescriptorSetLayouts() const
    {
        return mDescriptorSetLayouts;
    }

    NFE_INLINE const VolatileResourceMetadata& GetVolatileResources() const
    {
        return mSettings.vrMetadata;
    }

    NFE_INLINE const VolatileResourceMetadataEntry& GetVolatileResource(uint32 idx) const
    {
        return mSettings.vrMetadata[idx];
    }

    NFE_INLINE uint32 GetVolatileResourceCount() const
    {
        return mSettings.vrMetadata.Size();
    }

    NFE_INLINE const DescriptorSetMetadataEntry& GetDescriptorSet(uint32 idx) const
    {
        return mDescriptorSetMetadata[idx];
    }

    NFE_INLINE VkDescriptorType GetDescriptorType(uint32 set, uint32 binding) const
    {
        return mDescriptorSetMetadata[set].bindings[binding].type;
    }

    NFE_INLINE uint32 GetDescriptorSetCount() const
    {
        return mDescriptorSetMetadata.Size();
    }

    NFE_INLINE const VkPipelineLayout& GetPipelineLayout() const
    {
        return mPipelineLayout;
    }

    NFE_INLINE const VkPipeline& GetPipeline() const
    {
        return mPipeline;
    }
};

} // namespace Renderer
} // namespace NFE