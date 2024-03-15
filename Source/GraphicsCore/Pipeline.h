#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "Command.h"
#include "Parameters.h"
#include "Shader.h"

class Pipeline {
public:
    Pipeline(std::shared_ptr<Device> device);
    ~Pipeline();

    virtual inline VkPipelineBindPoint GetBindPoint() const = 0;

    void Bind(CommandBuffer command_buffer);

protected:
    virtual void CreatePipelineLayout() = 0;
    virtual void CreatePipeline() = 0;

private:
    std::shared_ptr<Device> device_;
    VkPipeline pipeline_;
    VkPipelineLayout pipeline_layout_;
};

class ComputePipeline : public Pipeline{
public:
    ComputePipeline(std::shared_ptr<Device> device, std::shared_ptr<Shader> compute_shader);
    ~ComputePipeline();

    virtual inline VkPipelineBindPoint GetBindPoint() const override {return VK_PIPELINE_BIND_POINT_COMPUTE;}

    void DispatchCompute(CommandBuffer command_buffer, uint32_t group_width, uint32_t group_height, uint32_t group_depth);

private:

    void CreatePipelineLayout();
};