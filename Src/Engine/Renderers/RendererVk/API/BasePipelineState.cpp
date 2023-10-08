#include "PCH.hpp"
#include "BasePipelineState.hpp"

#include "Device.hpp"
#include "Shader.hpp"

#include "Internal/Debugger.hpp"


namespace NFE {
namespace Renderer {


BasePipelineState::BasePipelineState(VkPipelineBindPoint bindPoint)
    : mSettings(bindPoint)
    , mShaders()
    , mShaderStageDescs()
    , mDescriptorSetMetadata()
    , mDescriptorSetLayouts()
    , mPipelineLayout(VK_NULL_HANDLE)
    , mPipeline(VK_NULL_HANDLE)
{
}

BasePipelineState::~BasePipelineState()
{
    if (mPipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(gDevice->GetDevice(), mPipeline, nullptr);

    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);

    for (auto& dsl: mDescriptorSetLayouts)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), dsl, nullptr);

    for (auto& s: mShaderStageDescs)
        vkDestroyShaderModule(gDevice->GetDevice(), s.module, nullptr);
}

void BasePipelineState::InitializeSettings(const PipelineStateDesc& desc)
{
    mSettings.useTessellation = (desc.numControlPoints > 0) && (desc.hullShader != nullptr) && (desc.domainShader != nullptr);

    for (const auto& vb: desc.volatileBufferBindings)
    {
        mSettings.vrMetadata.EmplaceBack(vb.stage, vb.binding);
    }
}

void BasePipelineState::InitializeSettings(const ComputePipelineStateDesc& desc)
{
    NFE_UNUSED(desc);
    // TODO
    /*for (const auto& vb: desc.volatileBufferBindings)
    {
        mSettings.vrMetadata.EmplaceBack(vb.stage, vb.binding);
    }*/
}

bool BasePipelineState::MapToDescriptorSet(Shader* s, SpvReflectDescriptorType type, uint32& set)
{
    bool allocatedSet = false;

    if (!s->ForEachDescriptorBinding([&](const SpvReflectDescriptorBinding* binding) {
        if (binding->descriptor_type == type)
        {
            if (!allocatedSet)
            {
                mDescriptorSetMetadata.EmplaceBack(s->GetShaderStageInfo().stage, set);
                allocatedSet = true;
            }

            if (!s->ChangeDescriptorSetBinding(binding, set))
                return false;

            DescriptorBindings& descs = mDescriptorSetMetadata.Back().bindings;
            descs.EmplaceBack(type, binding->binding);
            if (type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
                (binding->binding < VK_MAX_VOLATILE_BUFFERS))
            {
                descs.Back().type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            }
        }

        return true;
    }))
    {
        NFE_LOG_ERROR("Failed to remap Descriptor Set numbers");
        return false;
    }

    if (allocatedSet)
        set++;

    return true;
}

bool BasePipelineState::RemapDescriptorSets()
{
    /**
     * Separate Descriptor Binding types to match HLSL:
     *   - (c#) Constant Buffer  (SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER[_DYNAMIC] )
     *   - (t#) Texture  (SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
     *   - (s#) Sampler  (SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER)
     *   - (u#) WritableBuffer  (SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
     *   - (u#) WritableTexture  (SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE)
     *   - (u#) Buffer (SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
     *   - (u#) RWBuffer (SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
     */

    uint32 totalSets = 0;

    for (auto& shader: mShaders)
    {
        Shader* s = dynamic_cast<Shader*>(shader.Get());
        NFE_ASSERT(s != nullptr, "Invalid shader pointer");

        if (!MapToDescriptorSet(s, SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER, totalSets))
            return false;

        if (!MapToDescriptorSet(s, SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE, totalSets))
            return false;

        if (!MapToDescriptorSet(s, SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER, totalSets))
            return false;

        if (!MapToDescriptorSet(s, SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER, totalSets))
            return false;

        if (!MapToDescriptorSet(s, SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE, totalSets))
            return false;

        if (!MapToDescriptorSet(s, SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, totalSets))
            return false;

        if (!MapToDescriptorSet(s, SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, totalSets))
            return false;
    }

    NFE_LOG_DEBUG("Remapped %d descriptor sets", totalSets);

    // TODO check if we actually remapped all sets

    return true;
}

bool BasePipelineState::CreateDescriptorSetLayouts()
{
    VkResult result = VK_SUCCESS;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    for (auto& dsData: mDescriptorSetMetadata)
    {
        Common::DynArray<VkDescriptorSetLayoutBinding> dslBindings(dsData.bindings.Size());
        for (uint32 i = 0; i < dslBindings.Size(); ++i)
        {
            dslBindings[i].binding = dsData.bindings[i].binding;
            dslBindings[i].descriptorCount = 1;
            dslBindings[i].descriptorType = dsData.bindings[i].type;
            dslBindings[i].stageFlags = dsData.stage;
            dslBindings[i].pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo dslInfo;
        VK_ZERO_MEMORY(dslInfo);
        dslInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslInfo.bindingCount = dslBindings.Size();
        dslInfo.pBindings = dslBindings.Data();
        result = vkCreateDescriptorSetLayout(gDevice->GetDevice(), &dslInfo, nullptr, &descriptorSetLayout);
        CHECK_VKRESULT(result, "Failed to allocate descriptor set layout");

        mDescriptorSetLayouts.EmplaceBack(descriptorSetLayout);
    }

    return true;
}

VkShaderModule BasePipelineState::CreateShaderModule(const SpvReflectShaderModule& shaderSpv)
{
    VkShaderModule s = VK_NULL_HANDLE;

    VkShaderModuleCreateInfo shaderInfo;
    VK_ZERO_MEMORY(shaderInfo);
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = spvReflectGetCodeSize(&shaderSpv);
    shaderInfo.pCode = spvReflectGetCode(&shaderSpv);
    VkResult result = vkCreateShaderModule(gDevice->GetDevice(), &shaderInfo, nullptr, &s);
    if (result != VK_SUCCESS)
    {
        NFE_LOG_ERROR("Failed to create Shader module");
        return VK_NULL_HANDLE;
    }

    return s;
}

bool BasePipelineState::CreatePipelineLayout()
{
    VkResult result = VK_SUCCESS;

    if (!RemapDescriptorSets())
        return false;

    if (!CreateDescriptorSetLayouts())
        return false;

    VkPipelineLayoutCreateInfo plInfo;
    VK_ZERO_MEMORY(plInfo);
    plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plInfo.setLayoutCount = mDescriptorSetLayouts.Size();
    plInfo.pSetLayouts = mDescriptorSetLayouts.Data();
    result = vkCreatePipelineLayout(gDevice->GetDevice(), &plInfo, nullptr, &mPipelineLayout);
    CHECK_VKRESULT(result, "Failed to create Pipeline Layout");

    // TODO evaluate the name to be something more interesting and helpful
    Debugger::Instance().NameObject(
        reinterpret_cast<uint64_t>(mPipelineLayout),
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        "Graphics Pipeline Layout"
    );

    return true;
}

bool BasePipelineState::PrepareShaderStage(const ShaderPtr& shader)
{
    if (!shader)
        return true; // quietly exit, we are skipping an optional stage

    Shader* s = dynamic_cast<Shader*>(shader.Get());

    mShaderStageDescs.EmplaceBack(s->GetShaderStageInfo());
    mShaders.PushBack(shader);

    return true;
}

bool BasePipelineState::FormShaderModules()
{
    for (uint32 i = 0; i < mShaders.Size(); ++i)
    {
        Shader* s = dynamic_cast<Shader*>(mShaders[i].Get());
        VkShaderModule shaderModule = CreateShaderModule(s->GetReflectShaderModule());
        if (shaderModule == VK_NULL_HANDLE)
            return false;

        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(shaderModule), VK_OBJECT_TYPE_SHADER_MODULE, s->GetShaderPath().Str());

        mShaderStageDescs[i].module = shaderModule;
    }

    return true;
}

bool BasePipelineState::IsCompatible(BasePipelineState* state)
{
    NFE_UNUSED(state);



    return false;
}


} // namespace Renderer
} // namespace NFE
