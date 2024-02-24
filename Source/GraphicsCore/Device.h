#pragma once

#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class Device {
public:
    Device(const VkInstance instance);
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

private:
    const VkInstance instance_;
    VkDevice logical_device_;
    VkPhysicalDevice physical_device_;

    Queue queues_[QueueType::MAX_QUEUE_TYPES];

    VkPhysicalDeviceFeatures device_features_;
    std::vector<std::string> requested_device_extensions_;
    std::vector<const char*> enabled_device_extensions_;

    void SelectPhysicalDevice();
    void RequestDeviceExtensions();
    void FindQueueFamilies();
    void CreateLogicalDeviceAndQueues();
};
