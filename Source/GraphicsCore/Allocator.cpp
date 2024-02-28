#include "Allocator.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

Allocator::Allocator(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device) :
    instance_{ instance },
    device_{ device },
    allocator_{ }
{
    CreateAllocator();
}

Allocator::~Allocator() {
    vmaDestroyAllocator(allocator_);
}

void Allocator::CreateAllocator() {
    VmaAllocatorCreateInfo allocator_info = {
        .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        .physicalDevice = device_->GetPhysicalDevice(),
        .device = device_->GetLogicalDevice(),
        .instance = instance_->GetInstance(),
        .vulkanApiVersion = VK_HEADER_VERSION_COMPLETE,
    };

    if (vmaCreateAllocator(&allocator_info, &allocator_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VMA allocator!");
    }
}