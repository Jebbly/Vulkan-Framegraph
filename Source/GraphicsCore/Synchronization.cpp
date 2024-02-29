#include "Synchronization.h"

#include <iostream>
#include <stdexcept>

Fence::Fence(std::shared_ptr<Device> device, bool start_signaled) :
    device_{ device },
    fence_{ VK_NULL_HANDLE }
{
    CreateFence(start_signaled);
}

Fence::~Fence() {
    std::cout << "Destroying fence" << std::endl;
    vkDestroyFence(device_->GetLogicalDevice(), fence_, nullptr);
}

void Fence::Wait(uint64_t ns) const {
    vkWaitForFences(device_->GetLogicalDevice(), 1, &fence_, VK_TRUE, ns);
}

void Fence::Reset() {
    vkResetFences(device_->GetLogicalDevice(), 1, &fence_);
}

void Fence::CreateFence(bool start_signaled) {
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = start_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<uint32_t>(0),
    };

    if (vkCreateFence(device_->GetLogicalDevice(), &fence_info, nullptr, &fence_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence!");
    }
}

Semaphore::Semaphore(std::shared_ptr<Device> device) :
    device_{ device }
{
    CreateSemaphore();
}

Semaphore::~Semaphore() {
    std::cout << "Destroying semaphore" << std::endl;
    vkDestroySemaphore(device_->GetLogicalDevice(), semaphore_, nullptr);
}

void Semaphore::CreateSemaphore() {
    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    if (vkCreateSemaphore(device_->GetLogicalDevice(), &semaphore_info, nullptr, &semaphore_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphore!");
    }
}