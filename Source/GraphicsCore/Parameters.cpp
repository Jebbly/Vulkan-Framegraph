#include "Parameters.h"

DescriptorSet::DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> set_layout) :
    device_{ device },
    set_layout_{ set_layout }
{}

void DescriptorSet::WriteBufferDescriptor(uint32_t binding, VkDescriptorType type, Buffer buffer, uint32_t offset, uint32_t range) {
    assert(type == set_layout_->GetType(binding));

    VkDescriptorBufferInfo& buffer_info = buffer_infos_.emplace_back(
        VkDescriptorBufferInfo {
            .buffer = buffer.GetBuffer(),
            .offset = offset,
            .range = range,
        }
    );

    VkWriteDescriptorSet buffer_descriptor = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set_,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &buffer_info,
    };

    write_infos_.push_back(buffer_descriptor);
}

void DescriptorSet::WriteImageDescriptor(uint32_t binding, VkDescriptorType type, ImageView image_view, Sampler sampler, VkImageLayout layout) {
    assert(type == set_layout_->GetType(binding));

    VkDescriptorImageInfo& image_info = image_infos_.emplace_back(
        VkDescriptorImageInfo {
            .sampler = sampler.GetSampler(),
            .imageView = image_view.GetImageView(),
            .imageLayout = layout,
        }
    );

    VkWriteDescriptorSet image_descriptor = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set_,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &image_info,
    };

    write_infos_.push_back(image_descriptor);
}

void DescriptorSet::Update() {
    vkUpdateDescriptorSets(device_->GetLogicalDevice(), static_cast<uint32_t>(write_infos_.size()), write_infos_.data(), 0, nullptr);
}

DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> device, VkDescriptorSetLayoutCreateFlags creation_flags) :
    device_{ device },
    creation_flags_{ creation_flags },
    is_compiled_{ false },
    set_layout_{ VK_NULL_HANDLE }
{}

DescriptorSetLayout::~DescriptorSetLayout() {
    if (set_layout_ != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device_->GetLogicalDevice(), set_layout_, nullptr);
    }
}

void DescriptorSetLayout::AddBinding(BindingInfo binding) {
    assert(!is_compiled_);

    VkDescriptorSetLayoutBinding binding_info = {
        .binding = static_cast<uint32_t>(layout_bindings_.size()),
        .descriptorType = binding.descriptor_type,
        .descriptorCount = 1,
        .stageFlags = binding.stage_flags,
    };

    layout_bindings_.push_back(binding_info);
}

void DescriptorSetLayout::Compile() {
    VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = creation_flags_,
        .bindingCount = static_cast<uint32_t>(layout_bindings_.size()),
        .pBindings = layout_bindings_.data()
    };

    vkCreateDescriptorSetLayout(device_->GetLogicalDevice(), &layout_info, nullptr, &set_layout_);
    is_compiled_ = true;
}

DescriptorPool::DescriptorPool(std::shared_ptr<Device> device, VkDescriptorPoolCreateFlags creation_flags) :
    device_{ device },
    creation_flags_{ creation_flags }
{
    CreateDescriptorPool();
}

DescriptorPool::~DescriptorPool() {
    while (!descriptor_pools_.empty()) {
        VkDescriptorPool descriptor_pool = descriptor_pools_.top();
        descriptor_pools_.pop();
        vkDestroyDescriptorPool(device_->GetLogicalDevice(), descriptor_pool, nullptr);
    }
}

std::shared_ptr<DescriptorSet> DescriptorPool::AllocateDescriptorSet(std::shared_ptr<DescriptorSetLayout> layout) {
    assert(!descriptor_pools_.empty() && layout->IsCompiled());

    VkDescriptorSetLayout set_layout = layout->GetLayout();
    VkDescriptorSetAllocateInfo descriptor_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptor_pools_.top(),
        .descriptorSetCount = 1,
        .pSetLayouts = &set_layout,
    };

    std::shared_ptr<DescriptorSet> descriptor_set = std::make_shared<DescriptorSet>(device_, layout);
    vkAllocateDescriptorSets(device_->GetLogicalDevice(), &descriptor_info, &descriptor_set->descriptor_set_);
    return descriptor_set;
}

void DescriptorPool::CreateDescriptorPool() {
    const uint32_t num_descriptors = 100;

    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = num_descriptors,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = num_descriptors,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = num_descriptors,
        },
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = creation_flags_,
        .maxSets = 10,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    VkDescriptorPool descriptor_pool;
    vkCreateDescriptorPool(device_->GetLogicalDevice(), &pool_info, nullptr, &descriptor_pool);
    descriptor_pools_.push(descriptor_pool);
}