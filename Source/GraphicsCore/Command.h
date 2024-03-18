#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Synchronization.h"

class CommandBuffer {
public:
    CommandBuffer(VkCommandBuffer command_buffer, const Device::Queue& queue);

    void Reset();

    void Begin(bool use_once);

    template<typename T> void Record(T commands) {
        commands(command_buffer_);
    }

    void End();
    void Submit(Fence* fence = nullptr);

    void InsertWaitSemaphore(Semaphore& semaphore, VkPipelineStageFlags stage_mask);
    void InsertSignalSemaphore(Semaphore& semaphore, VkPipelineStageFlags stage_mask);

private:
    const Device::Queue& queue_;
    VkCommandBuffer command_buffer_;

    std::vector<VkSemaphoreSubmitInfo> wait_semaphores_;
    std::vector<VkSemaphoreSubmitInfo> signal_semaphores_;
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
