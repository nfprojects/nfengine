/**
 * @file
 * @author  Lookey (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's pipeline state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "VertexLayout.hpp"
#include "Device.hpp"
#include "Shader.hpp"

#include "Internal/RenderPassManager.hpp"
#include "Internal/Debugger.hpp"


namespace NFE {
namespace Renderer {

PipelineState::PipelineState()
    : mDesc()
    , mShaderStageDescs()
    , mDescriptorSetLayouts()
    , mDescriptorSets()
    , mPipelineLayout(VK_NULL_HANDLE)
    , mPipeline(VK_NULL_HANDLE)
{
}

PipelineState::~PipelineState()
{
    if (mPipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(gDevice->GetDevice(), mPipeline, nullptr);

    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);

    for (auto& dsl: mDescriptorSetLayouts)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), dsl, nullptr);

    if (!mDescriptorSets.Empty())
        vkFreeDescriptorSets(gDevice->GetDevice(), gDevice->GetDescriptorPool(), mDescriptorSets.Size(), mDescriptorSets.Data());

    for (auto& s: mShaderStageDescs)
        vkDestroyShaderModule(gDevice->GetDevice(), s.module, nullptr);
}

bool PipelineState::MapToDescriptorSet(const ShaderPtr& shader, SpvReflectDescriptorType type, uint32& set)
{
    SpvReflectResult result = SPV_REFLECT_RESULT_SUCCESS;
    bool allocatedSet = false;

    Shader* s = dynamic_cast<Shader*>(shader.Get());
    NFE_ASSERT(s != nullptr, "Invalid shader pointer");

    for (auto& binding: s->mDescriptorBindings)
    {
        if (binding->descriptor_type == type)
        {
            if (!allocatedSet)
            {
                mDescriptorSetMetadata.EmplaceBack();
                mDescriptorSetMetadata.Back().stage = s->mStageInfo.stage;
                mDescriptorSetMetadata.Back().set = set;
                mDescriptorSetMetadata.Back().type = TranslateSpvReflectDescriptorTypeToVkDescriptorType(type);
                allocatedSet = true;
            }

            result = spvReflectChangeDescriptorBindingNumbers(
                &s->mSpvReflectModule, binding,
                static_cast<uint32_t>(SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE),
                set
            );
            CHECK_SPVREFLECTRESULT(result, "Failed to change Descriptor Binding set number");

            DescriptorBindings& descs = mDescriptorSetMetadata.Back().bindings;
            descs.EmplaceBack();
            descs.Back().binding = binding->binding;
        }
    }

    if (allocatedSet)
        set++;

    // TODO check if bindings are tightly packed?

    // TODO check for volatile buffers

    return true;
}

bool PipelineState::RemapDescriptorSets()
{
    /**
     * Separate Descriptor Binding types to match HLSL:
     *   - (c#) Constant Buffer  (SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER[_DYNAMIC] )
     *   - (t#) Texture  (SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
     *   - (s#) Sampler  (SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER)
     *   - (u#) WritableBuffer  (SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
     *   - (u#) WritableTexture  (SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE)
     */

    uint32 totalSets = 0;

    for (auto& shader: mShaders)
    {
        if (!MapToDescriptorSet(shader, SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER, totalSets))
            return false;

        if (!MapToDescriptorSet(shader, SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE, totalSets))
            return false;

        if (!MapToDescriptorSet(shader, SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER, totalSets))
            return false;

        if (!MapToDescriptorSet(shader, SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER, totalSets))
            return false;

        if (!MapToDescriptorSet(shader, SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE, totalSets))
            return false;
    }

    NFE_LOG_DEBUG("Remapped %d descriptor sets", totalSets);

    return true;
}

bool PipelineState::CreateDescriptorSets()
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
            dslBindings[i].descriptorType = dsData.type;
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

VkShaderModule PipelineState::CreateShaderModule(const SpvReflectShaderModule& shaderSpv)
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

bool PipelineState::CreatePipelineLayout()
{
    VkResult result = VK_SUCCESS;

    if (!RemapDescriptorSets())
        return false;

    if (!CreateDescriptorSets())
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

bool PipelineState::PrepareShaderStage(const ShaderPtr& shader)
{
    if (!shader)
        return true; // quietly exit, we are skipping an optional stage

    Shader* s = dynamic_cast<Shader*>(shader.Get());

    mShaderStageDescs.EmplaceBack(s->mStageInfo);
    mShaders.PushBack(shader);

    return true;
}

bool PipelineState::FormShaderModules()
{
    for (uint32 i = 0; i < mShaders.Size(); ++i)
    {
        Shader* s = dynamic_cast<Shader*>(mShaders[i].Get());
        VkShaderModule shaderModule = CreateShaderModule(s->mSpvReflectModule);
        if (shaderModule == VK_NULL_HANDLE)
            return false;

        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(shaderModule), VK_OBJECT_TYPE_SHADER_MODULE, s->mShaderPath.Str());

        mShaderStageDescs[i].module = shaderModule;
    }

    return true;
}

bool PipelineState::Init(const PipelineStateDesc& desc)
{
    mDesc = desc;

    VertexLayout* vl = dynamic_cast<VertexLayout*>(desc.vertexLayout.Get());

    VkPipelineVertexInputDivisorStateCreateInfoEXT pvisDivisorInfo;
    VK_ZERO_MEMORY(pvisDivisorInfo);
    pvisDivisorInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
    pvisDivisorInfo.vertexBindingDivisorCount = vl->mDivisors.Size();
    pvisDivisorInfo.pVertexBindingDivisors = vl->mDivisors.Data();

    VkPipelineVertexInputStateCreateInfo pvisInfo;
    VK_ZERO_MEMORY(pvisInfo);
    pvisInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pvisInfo.pNext = (vl->mDivisors.Size() > 0) ? &pvisDivisorInfo : nullptr;
    pvisInfo.vertexBindingDescriptionCount = vl->mBindings.Size();
    pvisInfo.pVertexBindingDescriptions = vl->mBindings.Data();
    pvisInfo.vertexAttributeDescriptionCount = vl->mAttributes.Size();
    pvisInfo.pVertexAttributeDescriptions = vl->mAttributes.Data();


    VkPipelineInputAssemblyStateCreateInfo piasInfo;
    VK_ZERO_MEMORY(piasInfo);
    piasInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    piasInfo.topology = TranslatePrimitiveTypeToVkTopology(desc.primitiveType);
    piasInfo.primitiveRestartEnable = VK_FALSE; // TODO?


    VkPipelineTessellationStateCreateInfo ptsInfo;
    VK_ZERO_MEMORY(ptsInfo);
    ptsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    ptsInfo.patchControlPoints = desc.numControlPoints;


    VkPipelineViewportStateCreateInfo pvsInfo;
    VK_ZERO_MEMORY(pvsInfo);
    pvsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    // no attached viewports/scissors here, as we want to dynamically change them
    pvsInfo.viewportCount = 1;
    pvsInfo.scissorCount = 1;


    VkPipelineRasterizationStateCreateInfo prsInfo;
    VK_ZERO_MEMORY(prsInfo);
    prsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    prsInfo.depthClampEnable = VK_FALSE; //?
    prsInfo.rasterizerDiscardEnable = VK_FALSE;
    prsInfo.polygonMode = TranslateFillModeToVkPolygonMode(desc.raterizerState.fillMode);
    prsInfo.cullMode = TranslateCullModeToVkCullMode(desc.raterizerState.cullMode);
    prsInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // to match cull mode in Translations
    prsInfo.depthBiasEnable = VK_FALSE;
    prsInfo.depthBiasConstantFactor = 0.0f;
    prsInfo.depthBiasClamp = 0.0f;
    prsInfo.depthBiasSlopeFactor = 0.0f;
    prsInfo.lineWidth = 1.0f;


    VkPipelineMultisampleStateCreateInfo pmsInfo;
    VK_ZERO_MEMORY(pmsInfo);
    pmsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pmsInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pmsInfo.sampleShadingEnable = VK_FALSE;
    pmsInfo.minSampleShading = 0.0f;
    pmsInfo.pSampleMask = nullptr;
    pmsInfo.alphaToCoverageEnable = desc.blendState.alphaToCoverage;
    pmsInfo.alphaToOneEnable = VK_FALSE;


    VkStencilOpState stencilOps;
    VK_ZERO_MEMORY(stencilOps);
    stencilOps.failOp = TranslateStencilOpToVkStencilOp(desc.depthState.stencilOpFail);
    stencilOps.passOp = TranslateStencilOpToVkStencilOp(desc.depthState.stencilOpPass);
    stencilOps.depthFailOp = TranslateStencilOpToVkStencilOp(desc.depthState.stencilOpDepthFail);
    stencilOps.compareOp = TranslateCompareFuncToVkCompareOp(desc.depthState.stencilFunc);
    stencilOps.compareMask = desc.depthState.stencilMask;
    stencilOps.writeMask = desc.depthState.stencilMask;
    stencilOps.reference = 0; // will be changed dynamically


    VkPipelineDepthStencilStateCreateInfo pdssInfo;
    VK_ZERO_MEMORY(pdssInfo);
    pdssInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pdssInfo.depthTestEnable = desc.depthState.depthTestEnable;
    pdssInfo.depthWriteEnable = desc.depthState.depthWriteEnable;
    pdssInfo.depthCompareOp = TranslateCompareFuncToVkCompareOp(desc.depthState.depthCompareFunc);
    pdssInfo.depthBoundsTestEnable = VK_FALSE;
    pdssInfo.stencilTestEnable = desc.depthState.stencilEnable;
    pdssInfo.front = pdssInfo.back = stencilOps;
    pdssInfo.minDepthBounds = 0.0f;
    pdssInfo.maxDepthBounds = 1.0f;


    VkPipelineColorBlendStateCreateInfo pcbsInfo;
    VK_ZERO_MEMORY(pcbsInfo);
    pcbsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pcbsInfo.logicOpEnable = VK_FALSE;
    pcbsInfo.logicOp = VK_LOGIC_OP_CLEAR;
    VkPipelineColorBlendAttachmentState states[MAX_RENDER_TARGETS];

    uint32 ind, i;
    for (i = 0; i < desc.renderTargetFormats.Size(); ++i)
    {
        VK_ZERO_MEMORY(states[i]);
        ind = desc.blendState.independent ? i : 0;
        states[i].blendEnable = desc.blendState.rtDescs[ind].enable;
        states[i].srcColorBlendFactor = TranslateBlendFuncToVkBlendFactor(desc.blendState.rtDescs[ind].srcColorFunc);
        states[i].dstColorBlendFactor = TranslateBlendFuncToVkBlendFactor(desc.blendState.rtDescs[ind].destColorFunc);
        states[i].colorBlendOp = TranslateBlendOpToVkBlendOp(desc.blendState.rtDescs[ind].colorOperator);
        states[i].srcAlphaBlendFactor = TranslateBlendFuncToVkBlendFactor(desc.blendState.rtDescs[ind].srcAlphaFunc);
        states[i].dstAlphaBlendFactor = TranslateBlendFuncToVkBlendFactor(desc.blendState.rtDescs[ind].destAlphaFunc);
        states[i].colorBlendOp = TranslateBlendOpToVkBlendOp(desc.blendState.rtDescs[ind].alphaOperator);
        states[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    pcbsInfo.attachmentCount = i;
    pcbsInfo.pAttachments = states;


    // make viewport, scissor and stencil ref dynamic
    VkPipelineDynamicStateCreateInfo pdsInfo;
    VK_ZERO_MEMORY(pdsInfo);
    pdsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    VkDynamicState dynStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE
    };
    pdsInfo.dynamicStateCount = sizeof(dynStates) / sizeof(dynStates[0]);
    pdsInfo.pDynamicStates = dynStates;


    // request a render pass from manager
    VkFormat colorFormats[MAX_RENDER_TARGETS];
    VkFormat depthFormat = TranslateFormatToVkFormat(desc.depthFormat);
    for (i = 0; i < desc.renderTargetFormats.Size(); ++i)
        colorFormats[i] = TranslateFormatToVkFormat(desc.renderTargetFormats[i]);

    RenderPassDesc rpDesc(colorFormats, desc.renderTargetFormats.Size(), depthFormat);
    VkRenderPass renderPass = gDevice->GetRenderPassManager()->GetRenderPass(rpDesc);
    if (renderPass == VK_NULL_HANDLE)
        return false;


    // shader stages
    if (!PrepareShaderStage(desc.vertexShader))
    {
        NFE_LOG_ERROR("Failed to prepare Vertex Shader Stage for Pipeline");
        return false;
    }

    if (!PrepareShaderStage(desc.hullShader))
    {
        NFE_LOG_ERROR("Failed to prepare Hull Shader Stage for Pipeline");
        return false;
    }

    if (!PrepareShaderStage(desc.domainShader))
    {
        NFE_LOG_ERROR("Failed to prepare Domain Shader Stage for Pipeline");
        return false;
    }

    if (!PrepareShaderStage(desc.geometryShader))
    {
        NFE_LOG_ERROR("Failed to prepare Geometry Shader Stage for Pipeline");
        return false;
    }

    if (!PrepareShaderStage(desc.pixelShader))
    {
        NFE_LOG_ERROR("Failed to prepare Pixel Shader Stage for Pipeline");
        return false;
    }


    // Now that we prepared shader stages, create a PipelineLayout based on reflection data
    if (!CreatePipelineLayout())
    {
        NFE_LOG_ERROR("Failed to create a Pipeline Layout for this PipelineState");
        return false;
    }


    if (!FormShaderModules())
    {
        NFE_LOG_ERROR("Failed to compile Shaders for PipelineState");
        return false;
    }


    VkGraphicsPipelineCreateInfo pipeInfo;
    VK_ZERO_MEMORY(pipeInfo);
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = mShaderStageDescs.Size();
    pipeInfo.pStages = mShaderStageDescs.Data();
    pipeInfo.pVertexInputState = &pvisInfo;
    pipeInfo.pInputAssemblyState = &piasInfo;
    if ((mDesc.numControlPoints > 0) && (mDesc.hullShader != nullptr) && (mDesc.domainShader != nullptr))
        pipeInfo.pTessellationState = &ptsInfo;
    pipeInfo.pViewportState = &pvsInfo;
    pipeInfo.pRasterizationState = &prsInfo;
    pipeInfo.pMultisampleState = &pmsInfo;
    pipeInfo.pDepthStencilState = &pdssInfo;
    pipeInfo.pColorBlendState = &pcbsInfo;
    pipeInfo.pDynamicState = &pdsInfo;
    pipeInfo.renderPass = renderPass;
    pipeInfo.layout = mPipelineLayout;
    pipeInfo.subpass = 0;
    VkResult result = vkCreateGraphicsPipelines(gDevice->GetDevice(), VK_NULL_HANDLE, 1, &pipeInfo,
                                                nullptr, &mPipeline);
    CHECK_VKRESULT(result, "Failed to create Graphics Pipeline");

    if (desc.debugName)
        Debugger::Instance().NameObject(reinterpret_cast<uint64_t>(mPipeline), VK_OBJECT_TYPE_PIPELINE, desc.debugName);

    return true;
}

} // namespace Renderer
} // namespace NFE
