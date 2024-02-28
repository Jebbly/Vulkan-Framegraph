#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "Instance.h"

class Device {
public:
    Device(std::shared_ptr<Instance> instance, const VkSurfaceKHR surface);
    ~Device();

    enum QueueType {
        GRAPHICS,
        COMPUTE,
        PRESENT,
        TRANSFER,
        MAX_QUEUE_TYPES,
    };

    struct Queue {
        uint32_t queue_family;
        uint32_t queue_index;
        float queue_priority;
        VkQueue queue;
    };

    inline const VkDevice& GetLogicalDevice() const {return logical_device_;}
    inline const VkPhysicalDevice& GetPhysicalDevice() const {return physical_device_;}
    inline const Queue& GetQueue(QueueType queue_type) const {
        assert(queue_type < QueueType::MAX_QUEUE_TYPES);
        return queues_[queue_type];
    }

    inline void WaitIdle() const {
        assert(logical_device_ != VK_NULL_HANDLE);
        vkDeviceWaitIdle(logical_device_);
    }

private:
    std::shared_ptr<Instance> instance_;

    VkDevice logical_device_;
    VkPhysicalDevice physical_device_;

    Queue queues_[QueueType::MAX_QUEUE_TYPES];

    VkPhysicalDeviceFeatures device_features_;
    std::vector<std::string> requested_device_extensions_;
    std::vector<const char*> enabled_device_extensions_;

    void SelectPhysicalDevice();
    void RequestDeviceExtensions();
    void FindQueueFamilies(const VkSurfaceKHR surface);
    void CreateLogicalDeviceAndQueues();
};
