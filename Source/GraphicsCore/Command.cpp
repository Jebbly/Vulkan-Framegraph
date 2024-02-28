#include "Command.h"

#include <stdexcept>

CommandBuffer::CommandBuffer(VkCommandBuffer command_buffer, const Device::Queue& queue) :
    queue_{ queue },
    command_buffer_{ command_buffer }
{}

void CommandBuffer::Begin(bool use_once) {
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    if (use_once) {
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    if (vkBeginCommandBuffer(command_buffer_, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin command buffer!");
    }
}

void CommandBuffer::Record(std::function<void(VkCommandBuffer)> commands) {
    commands(command_buffer_);
}

void CommandBuffer::Submit() {
    vkEndCommandBuffer(command_buffer_);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer_,
    };

    if (vkQueueSubmit(queue_.queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit command buffer!");
    }
}

CommandPool::CommandPool(std::shared_ptr<Device> device, Device::QueueType queue_type) :
    device_{ device },
    queue_type_{ queue_type }
{
    CreateCommandPool();
}

CommandPool::~CommandPool() {
    device_->WaitIdle();
    vkDestroyCommandPool(device_->GetLogicalDevice(), command_pool_, nullptr);
}

void CommandPool::CreateCommandPool() {
    uint32_t queue_family = device_->GetQueue(queue_type_).queue_family;
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family,
    };

    if (vkCreateCommandPool(device_->GetLogicalDevice(), &pool_info, nullptr, &command_pool_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

CommandBuffer CommandPool::AllocateSinglePrimaryCommandBuffer() const {
    VkCommandBufferAllocateInfo command_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool_,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    if (vkAllocateCommandBuffers(device_->GetLogicalDevice(), &command_info, &command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer!");
    }
    return CommandBuffer{command_buffer, device_->GetQueue(queue_type_)};
}