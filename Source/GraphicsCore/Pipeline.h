#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "Command.h"
#include "Parameters.h"
#include "Shader.h"

template <typename Derived>
class Pipeline {
public:
    Pipeline(std::shared_ptr<Device> device) : 
        device_{ device },
        pipeline_layout_{ VK_NULL_HANDLE },
        pipeline_{ VK_NULL_HANDLE }
    {}

    virtual ~Pipeline() {
        if (pipeline_layout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_->GetLogicalDevice(), pipeline_layout_, nullptr);
        }

        if (pipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_->GetLogicalDevice(), pipeline_, nullptr);
        }
    }

    virtual inline VkPipelineBindPoint GetBindPoint() const = 0;

    inline VkPipelineLayout GetPipelineLayout() const {return pipeline_layout_;}

    void Bind(CommandBuffer command_buffer) {
        command_buffer.Record([&](VkCommandBuffer command) {
            vkCmdBindPipeline(command, GetBindPoint(), pipeline_);
        });
    }

    void BindDescriptorSet(CommandBuffer command_buffer, uint32_t set, std::shared_ptr<DescriptorSet> descriptor_set) {
        command_buffer.Record([&](VkCommandBuffer command) {
            VkDescriptorSet descriptors = descriptor_set->GetDescriptorSet();
            vkCmdBindDescriptorSets(command, GetBindPoint(), GetPipelineLayout(), set, 1, &descriptors, 0, nullptr);
        });
    }

protected:
    std::shared_ptr<Device> device_;
    VkPipeline pipeline_;
    VkPipelineLayout pipeline_layout_;

    void Create() {
        static_cast<Derived*>(this)->CreatePipelineLayout();
        static_cast<Derived*>(this)->CreatePipeline();
    }

    virtual void CreatePipelineLayout() = 0;
    virtual void CreatePipeline() = 0;
};

class GraphicsPipeline : public Pipeline<GraphicsPipeline> {
public:
    friend class Pipeline<GraphicsPipeline>;

    struct AttachmentFormats {
        std::vector<VkFormat> color_formats;
        VkFormat depth_format = VK_FORMAT_UNDEFINED;
        VkFormat stencil_format = VK_FORMAT_UNDEFINED;
    };

    GraphicsPipeline(std::shared_ptr<Device> device, std::shared_ptr<Shader> graphics_shaders, const AttachmentFormats& attachment_formats);
    virtual ~GraphicsPipeline() = default;

    virtual inline VkPipelineBindPoint GetBindPoint() const override {return VK_PIPELINE_BIND_POINT_GRAPHICS;}

protected:
    virtual void CreatePipelineLayout() override;
    virtual void CreatePipeline() override;

private:
    std::shared_ptr<Shader> graphics_shaders_;
    AttachmentFormats attachment_formats_;
};

class ComputePipeline : public Pipeline<ComputePipeline> {
public:
    friend class Pipeline<ComputePipeline>;

    ComputePipeline(std::shared_ptr<Device> device, std::shared_ptr<Shader> compute_shader);
    virtual ~ComputePipeline() = default; 

    virtual inline VkPipelineBindPoint GetBindPoint() const override {return VK_PIPELINE_BIND_POINT_COMPUTE;}

    void DispatchCompute(CommandBuffer command_buffer, uint32_t group_width, uint32_t group_height, uint32_t group_depth);

protected:
    virtual void CreatePipelineLayout() override;
    virtual void CreatePipeline() override;

private:
    std::shared_ptr<Shader> compute_shader_;
};
