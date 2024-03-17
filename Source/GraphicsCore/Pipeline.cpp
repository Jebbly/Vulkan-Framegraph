#include "Pipeline.h"

Pipeline::Pipeline(std::shared_ptr<Device> device) :
    device_{ device }
{}

Pipeline::~Pipeline() {
    if (pipeline_layout_ != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device_->GetLogicalDevice(), pipeline_layout_, nullptr);
    }

    if (pipeline_ != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_->GetLogicalDevice(), pipeline_, nullptr);
    }
}

void Pipeline::Bind(CommandBuffer command_buffer) {
    command_buffer.Record([&](VkCommandBuffer command) {
        vkCmdBindPipeline(command, GetBindPoint(), pipeline_);
    });
}

ComputePipeline::ComputePipeline(std::shared_ptr<Device> device, std::shared_ptr<Shader> compute_shader) :
    Pipeline{ device },
    compute_shader_{ compute_shader }
{
    // TODO: handle this better, maybe CRTP?
    CreatePipelineLayout();
    CreatePipeline();
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