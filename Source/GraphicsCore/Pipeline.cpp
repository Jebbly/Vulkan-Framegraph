#include "Pipeline.h"

GraphicsPipeline::GraphicsPipeline(std::shared_ptr<Device> device, std::shared_ptr<Shader> graphics_shaders, const AttachmentFormats& attachment_formats) :
    Pipeline{ device },
    graphics_shaders_{ graphics_shaders },
    attachment_formats_{ attachment_formats }
{
    Create();
}

void GraphicsPipeline::CreatePipeline() {
    // TODO: some of this should be configurable
    VkPipelineInputAssemblyStateCreateInfo assembly_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo viewport_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    VkPipelineRasterizationStateCreateInfo rasterization_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisample_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    VkPipelineColorBlendAttachmentState attachment_blending = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo color_blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &attachment_blending,
    };

    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data(),
    };

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = static_cast<uint32_t>(attachment_formats_.color_formats.size()),
        .pColorAttachmentFormats = attachment_formats_.color_formats.data(),
        .depthAttachmentFormat = attachment_formats_.depth_format,
        .stencilAttachmentFormat = attachment_formats_.stencil_format,
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_rendering_info,
        .stageCount = 2,
        .pStages = nullptr, // TODO: implement this
        .pInputAssemblyState = &assembly_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterization_info,
        .pMultisampleState = &multisample_info,
        .pDepthStencilState = &depth_stencil_info,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = &dynamic_state_info,
        .layout = pipeline_layout_,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
    };

    vkCreateGraphicsPipelines(device_->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_);
}

ComputePipeline::ComputePipeline(std::shared_ptr<Device> device, std::shared_ptr<Shader> compute_shader) :
    Pipeline{ device },
    compute_shader_{ compute_shader }
{
    Create();
}

void ComputePipeline::CreatePipelineLayout() {
    std::vector<VkDescriptorSetLayout> descriptor_sets;
    const auto& compute_descriptor_sets = compute_shader_->GetParameterLayouts();
    for (const auto& descriptor_set : compute_descriptor_sets) {
        descriptor_sets.push_back(descriptor_set->GetLayout());
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(descriptor_sets.size()),
        .pSetLayouts = descriptor_sets.data(),
    };

    vkCreatePipelineLayout(device_->GetLogicalDevice(), &pipeline_layout_info, nullptr, &pipeline_layout_);
}

void ComputePipeline::CreatePipeline() {
    VkPipelineShaderStageCreateInfo compute_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = compute_shader_->GetModule(),
        .pName = "main",
    };

    VkComputePipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = compute_stage_info,
        .layout = pipeline_layout_,
    };

    vkCreateComputePipelines(device_->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_);
}

void ComputePipeline::DispatchCompute(CommandBuffer command_buffer, uint32_t group_width, uint32_t group_height, uint32_t group_depth) {
    command_buffer.Record([&](VkCommandBuffer command) {
        vkCmdDispatch(command, group_width, group_height, group_depth);
    });
}