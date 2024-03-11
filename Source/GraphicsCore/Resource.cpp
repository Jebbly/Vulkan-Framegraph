#include "Resource.h"

void Image::TransitionLayout(CommandBuffer& command_buffer, VkImageLayout new_layout) {
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange range = {
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };
}

void TransitionImage(VkCommandBuffer command, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange range = {
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };

    VkImageMemoryBarrier2 image_barrier = { 
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .image = image,
        .subresourceRange = range,
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(command, &dependency_info);
}

ResourceBarrier::ResourceBarrier(VkDependencyFlags dependency_flags) :
    dependency_flags_{ dependency_flags_ }
{}

void ResourceBarrier::AddMemoryBarrier(AccessInfo source, AccessInfo destination) {
    VkMemoryBarrier2 memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = source.stage_flags,
        .srcAccessMask = source.access_flags,
        .dstStageMask = destination.stage_flags,
        .dstAccessMask = destination.access_flags,
    };

    memory_barriers_.push_back(memory_barrier);
}

void ResourceBarrier::AddBufferMemoryBarrier(AccessInfo source, AccessInfo destination,
    uint32_t src_queue_family, uint32_t dst_queue_family,
    Buffer buffer) {
    VkBufferMemoryBarrier2 buffer_barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = source.stage_flags,
        .srcAccessMask = source.access_flags,
        .dstStageMask = destination.stage_flags,
        .srcQueueFamilyIndex = src_queue_family,
        .dstQueueFamilyIndex = dst_queue_family,
        // todo: finish buffer implementation
    };

    buffer_barriers_.push_back(buffer_barrier);
}

void ResourceBarrier::AddImageMemoryBarrier(AccessInfo source, AccessInfo destination,
    VkImageLayout old_layout, VkImageLayout new_layout,
    uint32_t src_queue_family, uint32_t dst_queue_family, Image image) {
    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = source.stage_flags,
        .srcAccessMask = source.access_flags,
        .dstStageMask = destination.stage_flags,
        .srcQueueFamilyIndex = src_queue_family,
        .dstQueueFamilyIndex = dst_queue_family,
        // todo: finish image implementation
    };

    image_barriers_.push_back(image_barrier);
}

void ResourceBarrier::InsertIntoCommandBuffer(CommandBuffer command_buffer) { 
    command_buffer.Record([&](VkCommandBuffer command) {
        VkDependencyInfo dependency_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .memoryBarrierCount = static_cast<uint32_t>(memory_barriers_.size()),
            .pMemoryBarriers = memory_barriers_.data(),
            .bufferMemoryBarrierCount = static_cast<uint32_t>(buffer_barriers_.size()),
            .pBufferMemoryBarriers = buffer_barriers_.data(),
            .imageMemoryBarrierCount = static_cast<uint32_t>(image_barriers_.size()),
            .pImageMemoryBarriers = image_barriers_.data(),
        };

        vkCmdPipelineBarrier2(command, &dependency_info);
    });
}