#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.h"

class CommandBuffer {
public:
    CommandBuffer(VkCommandBuffer command_buffer, const Device::Queue& queue);

    void Begin(bool use_once);
    void Record(std::function<void (VkCommandBuffer)> commands);
    void Submit();

private:
    const Device::Queue& queue_;
    VkCommandBuffer command_buffer_;
};

class CommandPool {
public:
    CommandPool(std::shared_ptr<Device> device, Device::QueueType queue_type);
    ~CommandPool();

    CommandBuffer AllocateSinglePrimaryCommandBuffer() const;

private:
    std::shared_ptr<Device> device_;
    Device::QueueType queue_type_;
    VkCommandPool command_pool_;

    void CreateCommandPool();
};