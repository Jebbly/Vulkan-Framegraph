#include "Pipeline.h"

Pipeline::Pipeline(std::shared_ptr<Device> device) :
    device_{ device }
{
    CreatePipelineLayout();
    CreatePipeline();
}

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
    Pipeline{ device }
{

}

ComputePipeline::~ComputePipeline() {

}

void ComputePipeline::DispatchCompute(CommandBuffer command_buffer, uint32_t group_width, uint32_t group_height, uint32_t group_depth) {
    command_buffer.Record([&](VkCommandBuffer command) {
        vkCmdDispatch(command, group_width, group_height, group_depth);
    });
}