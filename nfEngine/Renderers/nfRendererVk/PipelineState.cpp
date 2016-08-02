/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Vulkan implementation of renderer's pipeline state
 */

#include "PCH.hpp"
#include "PipelineState.hpp"
#include "ShaderProgram.hpp"
#include "VertexLayout.hpp"
#include "Device.hpp"


namespace NFE {
namespace Renderer {

PipelineState::~PipelineState()
{
}

bool PipelineState::Init(const PipelineStateDesc& desc)
{
    mDesc = desc;
    return true;
}

VkPipeline PipelineState::CreateFullPipelineState(const FullPipelineStateParts& parts)
{
    PipelineState* ps = dynamic_cast<PipelineState*>(std::get<0>(parts));
    ShaderProgram* sp = dynamic_cast<ShaderProgram*>(std::get<1>(parts));

    VertexLayout* vl = dynamic_cast<VertexLayout*>(ps->mDesc.vertexLayout);
    VkPipelineVertexInputStateCreateInfo pvisInfo;
    VK_ZERO_MEMORY(pvisInfo);
    pvisInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pvisInfo.vertexBindingDescriptionCount = static_cast<uint32>(vl->mBindings.size());
    pvisInfo.pVertexBindingDescriptions = vl->mBindings.data();
    pvisInfo.vertexAttributeDescriptionCount = static_cast<uint32>(vl->mAttributes.size());
    pvisInfo.pVertexAttributeDescriptions = vl->mAttributes.data();

    VkPipelineInputAssemblyStateCreateInfo piasInfo;
    VK_ZERO_MEMORY(piasInfo);
    piasInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    piasInfo.topology = TranslatePrimitiveTypeToVkTopology(ps->mDesc.primitiveType);
    piasInfo.primitiveRestartEnable = VK_FALSE; // TODO?

    VkPipelineTessellationStateCreateInfo ptsInfo;
    VK_ZERO_MEMORY(ptsInfo);
    ptsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    ptsInfo.patchControlPoints = ps->mDesc.numControlPoints;

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
    prsInfo.rasterizerDiscardEnable = VK_TRUE;
    prsInfo.polygonMode = TranslateFillModeToVkPolygonMode(ps->mDesc.raterizerState.fillMode);
    prsInfo.cullMode = TranslateCullModeToVkCullMode(ps->mDesc.raterizerState.cullMode);
    prsInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // to match cull mode in Translations
    prsInfo.depthBiasEnable = VK_FALSE;
    prsInfo.depthBiasConstantFactor = 0.0f;
    prsInfo.depthBiasClamp = 0.0f;
    prsInfo.depthBiasSlopeFactor = 0.0f;
    prsInfo.lineWidth = 0.0f;

    VkPipelineMultisampleStateCreateInfo pmsInfo;
    VK_ZERO_MEMORY(pmsInfo);
    pmsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pmsInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pmsInfo.sampleShadingEnable = VK_FALSE;
    pmsInfo.minSampleShading = 0.0f;
    pmsInfo.pSampleMask = nullptr;
    pmsInfo.alphaToCoverageEnable = ps->mDesc.blendState.alphaToCoverage;
    pmsInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo pdssInfo;
    VK_ZERO_MEMORY(pdssInfo);
    pdssInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pdssInfo.depthTestEnable = ps->mDesc.depthState.depthTestEnable;
    pdssInfo.depthWriteEnable = ps->mDesc.depthState.depthWriteEnable;
    pdssInfo.depthCompareOp = TranslateCompareFuncToVkCompareOp(ps->mDesc.depthState.depthCompareFunc);
    pdssInfo.depthBoundsTestEnable = VK_FALSE;
    pdssInfo.stencilTestEnable = ps->mDesc.depthState.stencilEnable;
    VkStencilOpState stencilOps;
    VK_ZERO_MEMORY(stencilOps);
    stencilOps.failOp = TranslateStencilOpToVkStencilOp(ps->mDesc.depthState.stencilOpFail);
    stencilOps.passOp = TranslateStencilOpToVkStencilOp(ps->mDesc.depthState.stencilOpPass);
    stencilOps.depthFailOp = TranslateStencilOpToVkStencilOp(ps->mDesc.depthState.stencilOpDepthFail);
    stencilOps.compareOp = TranslateCompareFuncToVkCompareOp(ps->mDesc.depthState.stencilFunc);
    stencilOps.compareMask = ps->mDesc.depthState.stencilMask;
    stencilOps.writeMask = ps->mDesc.depthState.stencilMask;
    stencilOps.reference = 0; // stencilOps.reference will be changed dynamically
    pdssInfo.front = pdssInfo.back = stencilOps;
    pdssInfo.minDepthBounds = pdssInfo.maxDepthBounds = 0.0f;

    VkPipelineColorBlendStateCreateInfo pcbsInfo;
    VK_ZERO_MEMORY(pcbsInfo);
    pcbsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pcbsInfo.logicOpEnable = VK_FALSE;
    pcbsInfo.logicOp = VK_LOGIC_OP_CLEAR;
    std::vector<VkPipelineColorBlendAttachmentState> states;
    VkPipelineColorBlendAttachmentState state;
    VK_ZERO_MEMORY(state);
    uint32 ind;
    for (uint32 i = 0; i < ps->mDesc.numRenderTargets; ++i)
    {
        ind = ps->mDesc.blendState.independent ? i : 0;
        state.blendEnable = ps->mDesc.blendState.rtDescs[ind].enable;
        state.srcColorBlendFactor = TranslateBlendFuncToVkBlendFactor(ps->mDesc.blendState.rtDescs[ind].srcColorFunc);
        state.dstColorBlendFactor = TranslateBlendFuncToVkBlendFactor(ps->mDesc.blendState.rtDescs[ind].destColorFunc);
        state.colorBlendOp = TranslateBlendOpToVkBlendOp(ps->mDesc.blendState.rtDescs[ind].colorOperator);
        state.srcAlphaBlendFactor = TranslateBlendFuncToVkBlendFactor(ps->mDesc.blendState.rtDescs[ind].srcAlphaFunc);
        state.dstAlphaBlendFactor = TranslateBlendFuncToVkBlendFactor(ps->mDesc.blendState.rtDescs[ind].destAlphaFunc);
        state.colorBlendOp = TranslateBlendOpToVkBlendOp(ps->mDesc.blendState.rtDescs[ind].alphaOperator);
        state.colorWriteMask = VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        states.push_back(state);
    }

    pcbsInfo.attachmentCount = static_cast<uint32>(states.size());
    pcbsInfo.pAttachments = states.data();

    // make viewport, scissor and stencil ref dynamic
    VkPipelineDynamicStateCreateInfo pdsInfo;
    VK_ZERO_MEMORY(pdsInfo);
    pdsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    std::vector<VkDynamicState> dynStates;
    dynStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    dynStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
    pdsInfo.dynamicStateCount = static_cast<uint32>(dynStates.size());
    pdsInfo.pDynamicStates = dynStates.data();


    ResourceBindingLayout* rbl = dynamic_cast<ResourceBindingLayout*>(ps->mDesc.resBindingLayout);

    VkPipeline pipeline;
    VkGraphicsPipelineCreateInfo pipeInfo;
    VK_ZERO_MEMORY(pipeInfo);
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = static_cast<uint32>(sp->GetShaderStages().size());
    pipeInfo.pStages = sp->GetShaderStages().data();
    pipeInfo.pVertexInputState = &pvisInfo;
    pipeInfo.pInputAssemblyState = &piasInfo;
    pipeInfo.pTessellationState = &ptsInfo;
    pipeInfo.pViewportState = &pvsInfo;
    pipeInfo.pRasterizationState = &prsInfo;
    pipeInfo.pMultisampleState = &pmsInfo;
    pipeInfo.pDepthStencilState = &pdssInfo;
    pipeInfo.pColorBlendState = &pcbsInfo;
    pipeInfo.pDynamicState = &pdsInfo;
    pipeInfo.renderPass = VK_NULL_HANDLE; // TODO
    pipeInfo.layout = rbl->mPipelineLayout;
    pipeInfo.subpass = 0; // TODO
    VkResult result = vkCreateGraphicsPipelines(gDevice->GetDevice(), gDevice->GetPipelineCache(), 1, &pipeInfo,
                                                nullptr, &pipeline);
    CHECK_VKRESULT(result, "Failed to create Graphics Pipeline");

    return pipeline;
}

} // namespace Renderer
} // namespace NFE
