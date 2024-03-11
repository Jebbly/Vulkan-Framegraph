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
    Image(std::shared_ptr<Device> device, VkImage image, VkImageUsageFlags usage);
    ~Image();

    inline const VkImage& GetImage() const {return image_;}
    inline VkImageLayout getLayout() const {return layout_;}
    inline VkImageUsageFlags GetUsageFlags() const {return usage_;}

    inline void ResetLayout(VkImageLayout layout) {layout_ = layout;}
    void TransitionLayout(CommandBuffer& command_buffer, VkImageLayout new_layout);

private:
    VkImage image_;
    VkImageLayout layout_;
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
    void AddBufferMemoryBarrier(AccessInfo source, AccessInfo destination,
        uint32_t src_queue_family, uint32_t dst_queue_family,
        Buffer buffer);
    void AddImageMemoryBarrier(AccessInfo source, AccessInfo destination,
        VkImageLayout old_layout, VkImageLayout new_layout,
        uint32_t src_queue_family, uint32_t dst_queue_family,
        Image image);

    void InsertIntoCommandBuffer(CommandBuffer command_buffer);

private:
    VkDependencyFlags dependency_flags_;
    std::vector<VkMemoryBarrier2> memory_barriers_;
    std::vector<VkBufferMemoryBarrier2> buffer_barriers_;
    std::vector<VkImageMemoryBarrier2> image_barriers_;
};