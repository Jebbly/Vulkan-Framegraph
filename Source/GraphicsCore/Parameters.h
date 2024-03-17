#pragma once

#include <deque>
#include <memory>
#include <stack>
#include <vector>

#include <vulkan/vulkan.h>

#include "Command.h"
#include "Device.h"
#include "Resources.h"

class DescriptorSetLayout {
public:
    struct BindingInfo {
        VkDescriptorType descriptor_type;
        VkShaderStageFlags stage_flags;
    };

    DescriptorSetLayout(std::shared_ptr<Device> device, VkDescriptorSetLayoutCreateFlags creation_flags = 0);
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

class DescriptorSet {
public:
    friend class DescriptorPool;

    DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> set_layout);

    void WriteBufferDescriptor(uint32_t binding, VkDescriptorType type, std::shared_ptr<Buffer> buffer, uint32_t offset, uint32_t range);
    void WriteImageDescriptor(uint32_t binding, VkDescriptorType type, std::shared_ptr<ImageView> image_view, std::shared_ptr<Sampler> sampler, VkImageLayout layout);

    void Update();

    inline VkDescriptorSet GetDescriptorSet() const {return descriptor_set_;}

private:
    std::shared_ptr<Device> device_;
    std::shared_ptr<DescriptorSetLayout> set_layout_;

    VkDescriptorSet descriptor_set_;
    std::deque<VkDescriptorBufferInfo> buffer_infos_;
    std::deque<VkDescriptorImageInfo> image_infos_;
    std::vector<VkWriteDescriptorSet> write_infos_;
};

class DescriptorPool {
public:
    DescriptorPool(std::shared_ptr<Device> device, VkDescriptorPoolCreateFlags creation_flags = 0);
    ~DescriptorPool();

    std::shared_ptr<DescriptorSet> AllocateDescriptorSet(std::shared_ptr<DescriptorSetLayout> layout);

private:
    std::shared_ptr<Device> device_;
    VkDescriptorPoolCreateFlags creation_flags_;
    std::stack<VkDescriptorPool> descriptor_pools_;
    
    void CreateDescriptorPool();
};
