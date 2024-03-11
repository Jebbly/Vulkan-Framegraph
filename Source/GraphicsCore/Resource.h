#pragma once

#include <vulkan/vulkan.h>

#include "Command.h"
#include "Synchronization.h"

class Buffer {
private:
    VkBuffer buffer_;
};

class Image {
public:
    Image(VkImage image, VkImageUsageFlags usage);
    ~Image() = default; // TODO: fix this

    inline const VkImage& GetImage() const {return image_;}
    inline VkImageUsageFlags GetUsageFlags() const {return usage_;}

    void TransitionImage(CommandBuffer command_buffer, VkImageLayout old_layout, VkImageLayout new_layout) const;

private:
    VkImage image_;
    VkImageUsageFlags usage_;
};

class ResourceBarrier {
public:
    struct AccessInfo {
        VkPipelineStageFlags2 stage_flags;
        VkAccessFlags2 access_flags;
        uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    };

    ResourceBarrier(VkDependencyFlags dependency_flags = 0);
    ~ResourceBarrier() = default;

    void AddMemoryBarrier(AccessInfo source, AccessInfo destination);
    void AddBufferMemoryBarrier(AccessInfo source, AccessInfo destination, Buffer buffer);
    void AddImageMemoryBarrier(AccessInfo source, AccessInfo destination,
        VkImageLayout old_layout, VkImageLayout new_layout, Image image, VkImageSubresourceRange range);

    void InsertIntoCommandBuffer(CommandBuffer command_buffer);

private:
    VkDependencyFlags dependency_flags_;
    std::vector<VkMemoryBarrier2> memory_barriers_;
    std::vector<VkBufferMemoryBarrier2> buffer_barriers_;
    std::vector<VkImageMemoryBarrier2> image_barriers_;
};