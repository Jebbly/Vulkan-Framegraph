#pragma once

#include <deque>
#include <memory>
#include <stack>
#include <vector>

#include <vulkan/vulkan.h>

#include "Command.h"
#include "Device.h"
#include "Resources.h"

class DescriptorSet {
public:
    friend class DescriptorPool;

    DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> set_layout);

    void WriteBufferDescriptor(uint32_t binding, VkDescriptorType type, Buffer buffer, uint32_t offset, uint32_t range);
    void WriteImageDescriptor(uint32_t binding, VkDescriptorType type, ImageView image_view, Sampler sampler, VkImageLayout layout);

    void Update();

private:
    std::shared_ptr<Device> device_;
    std::shared_ptr<DescriptorSetLayout> set_layout_;

    VkDescriptorSet descriptor_set_;
    std::deque<VkDescriptorBufferInfo> buffer_infos_;
    std::deque<VkDescriptorImageInfo> image_infos_;
    std::vector<VkWriteDescriptorSet> write_infos_;
};

class DescriptorSetLayout {
public:
    struct BindingInfo {
        VkDescriptorType descriptor_type;
        VkShaderStageFlags stage_flags;
    };

    DescriptorSetLayout(std::shared_ptr<Device> device, VkDescriptorSetLayoutCreateFlags creation_flags);
    ~DescriptorSetLayout();

    void AddBinding(BindingInfo binding);
    void Compile();

    inline VkDescriptorSetLayout GetLayout() const {return set_layout_;}
    inline bool IsCompiled() const {return is_compiled_;}
    inline VkDescriptorType GetType(uint32_t binding) const {return layout_bindings_.at(binding).descriptorType;}

private:
    std::shared_ptr<Device> device_;
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings_;

    bool is_compiled_;
    VkDescriptorSetLayoutCreateFlags creation_flags_;
    VkDescriptorSetLayout set_layout_;
};

class DescriptorPool {
public:
    DescriptorPool(std::shared_ptr<Device> device, VkDescriptorPoolCreateFlagBits creation_flags);
    ~DescriptorPool();

    std::shared_ptr<DescriptorSet> AllocateDescriptorSet(std::shared_ptr<DescriptorSetLayout> layout);

private:
    std::shared_ptr<Device> device_;
    VkDescriptorPoolCreateFlagBits creation_flags_;
    std::stack<VkDescriptorPool> descriptor_pools_;
    
    void CreateDescriptorPool();
};
