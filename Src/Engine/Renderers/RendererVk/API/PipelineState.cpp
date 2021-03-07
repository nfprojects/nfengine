/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
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
    : mPipeline(VK_NULL_HANDLE)
{
}

PipelineState::~PipelineState()
{
    if (mPipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(gDevice->GetDevice(), mPipeline, nullptr);
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
    for (i = 0; i < desc.numRenderTargets; ++i)
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
    for (i = 0; i < desc.numRenderTargets; ++i)
        colorFormats[i] = TranslateFormatToVkFormat(desc.rtFormats[i]);

    RenderPassDesc rpDesc(colorFormats, desc.numRenderTargets, depthFormat);
    VkRenderPass renderPass = gDevice->GetRenderPassManager()->GetRenderPass(rpDesc);
    if (renderPass == VK_NULL_HANDLE)
        return false;


    // bind resource layout
    ResourceBindingLayout* rbl = dynamic_cast<ResourceBindingLayout*>(desc.resBindingLayout.Get());


    // shader stages
    VkPipelineShaderStageCreateInfo stages[5];
    uint32 stageCount = 0;

    if (mDesc.vertexShader)
    {
        Shader* s = dynamic_cast<Shader*>(mDesc.vertexShader.Get());
        stages[stageCount] = s->mStageInfo;
        stageCount++;
    }
    if (mDesc.hullShader)
    {
        Shader* s = dynamic_cast<Shader*>(mDesc.hullShader.Get());
        stages[stageCount] = s->mStageInfo;
        stageCount++;
    }
    if (mDesc.domainShader)
    {
        Shader* s = dynamic_cast<Shader*>(mDesc.domainShader.Get());
        stages[stageCount] = s->mStageInfo;
        stageCount++;
    }
    if (mDesc.geometryShader)
    {
        Shader* s = dynamic_cast<Shader*>(mDesc.geometryShader.Get());
        stages[stageCount] = s->mStageInfo;
        stageCount++;
    }
    if (mDesc.pixelShader)
    {
        Shader* s = dynamic_cast<Shader*>(mDesc.pixelShader.Get());
        stages[stageCount] = s->mStageInfo;
        stageCount++;
    }


    VkGraphicsPipelineCreateInfo pipeInfo;
    VK_ZERO_MEMORY(pipeInfo);
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = stageCount;
    pipeInfo.pStages = stages;
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
    pipeInfo.layout = rbl->mPipelineLayout;
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
