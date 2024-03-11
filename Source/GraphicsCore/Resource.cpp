#include "Resource.h"

Image::Image(VkImage image, VkImageUsageFlags usage) :
    image_{ image },
    usage_{ usage }
{}

void Image::TransitionImage(CommandBuffer command_buffer, VkImageLayout old_layout, VkImageLayout new_layout) const {
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange range = {
        .aspectMask = aspect_mask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
    };

    ResourceBarrier::AccessInfo source_access = {
        .stage_flags = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .access_flags = VK_ACCESS_2_MEMORY_WRITE_BIT,
    };

    ResourceBarrier::AccessInfo destination_access = {
        .stage_flags = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .access_flags = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
    };

    ResourceBarrier transition_barrier;
    transition_barrier.AddImageMemoryBarrier(source_access, destination_access, old_layout, new_layout, *this, range);
    transition_barrier.InsertIntoCommandBuffer(command_buffer);
}

ResourceBarrier::ResourceBarrier(VkDependencyFlags dependency_flags) :
    dependency_flags_{ dependency_flags }
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

void ResourceBarrier::AddBufferMemoryBarrier(AccessInfo source, AccessInfo destination, Buffer buffer) {
    VkBufferMemoryBarrier2 buffer_barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = source.stage_flags,
        .srcAccessMask = source.access_flags,
        .dstStageMask = destination.stage_flags,
        .srcQueueFamilyIndex = source.queue_family_index,
        .dstQueueFamilyIndex = destination.queue_family_index,
        // todo: finish buffer implementation
    };

    buffer_barriers_.push_back(buffer_barrier);
}

void ResourceBarrier::AddImageMemoryBarrier(AccessInfo source, AccessInfo destination,
    VkImageLayout old_layout, VkImageLayout new_layout, Image image, VkImageSubresourceRange range) {
    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = source.stage_flags,
        .srcAccessMask = source.access_flags,
        .dstStageMask = destination.stage_flags,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = source.queue_family_index,
        .dstQueueFamilyIndex = destination.queue_family_index,
        .image = image.GetImage(),
        .subresourceRange = range,
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