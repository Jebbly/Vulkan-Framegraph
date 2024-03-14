#include "Resources.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

Allocator::Allocator(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device) :
    instance_{ instance },
    device_{ device },
    allocator_{ VMA_NULL }
{
    CreateAllocator();
}

Allocator::~Allocator() {
    if (allocator_ != VMA_NULL) {
        vmaDestroyAllocator(allocator_);
        allocator_ = VMA_NULL;
    }
}

std::shared_ptr<Buffer> Allocator::AllocateBuffer(Buffer::Desc& buffer_desc, ResourceDesc resource_desc) {
    VkBufferCreateInfo buffer_info = {
        .sType  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = buffer_desc.buffer_size,
        .usage = buffer_desc.buffer_usage,
    };

    if (!resource_desc.queue_families.empty()) {
        buffer_info.sharingMode = resource_desc.sharing_mode;
        buffer_info.queueFamilyIndexCount = static_cast<uint32_t>(resource_desc.queue_families.size());
        buffer_info.pQueueFamilyIndices = resource_desc.queue_families.data();
    }

    VmaAllocationCreateInfo alloc_create_info = {
        .usage = resource_desc.memory_usage,
        .requiredFlags = resource_desc.allocation_flags,
    };

    std::shared_ptr<Buffer> new_buffer = std::make_shared<Buffer>(buffer_desc);
    vmaCreateBuffer(allocator_, &buffer_info, &alloc_create_info, &new_buffer->buffer_, &new_buffer->allocation_, nullptr);
    return new_buffer;
}

std::shared_ptr<Image> Allocator::AllocateImage(Image::Desc& image_desc, ResourceDesc resource_desc) {
    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = image_desc.image_format,
        .extent = image_desc.image_extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = image_desc.image_usage,
    };

    if (!resource_desc.queue_families.empty()) {
        image_info.sharingMode = resource_desc.sharing_mode;
        image_info.queueFamilyIndexCount = static_cast<uint32_t>(resource_desc.queue_families.size());
        image_info.pQueueFamilyIndices = resource_desc.queue_families.data();
    }

    VmaAllocationCreateInfo alloc_create_info = {
        .usage = resource_desc.memory_usage,
        .requiredFlags = resource_desc.allocation_flags,
    };

    std::shared_ptr<Image> new_image = std::make_shared<Image>(image_desc);
    vmaCreateImage(allocator_, &image_info, &alloc_create_info, &new_image->image_, &new_image->allocation_, nullptr);
    return new_image;
}

void Allocator::CreateAllocator() {
    VmaAllocatorCreateInfo allocator_info = {
        .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        .physicalDevice = device_->GetPhysicalDevice(),
        .device = device_->GetLogicalDevice(),
        .instance = instance_->GetInstance(),
        .vulkanApiVersion = VK_HEADER_VERSION_COMPLETE,
    };

    if (vmaCreateAllocator(&allocator_info, &allocator_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VMA allocator!");
    }
}

Buffer::Buffer(Buffer::Desc buffer_desc) :
    buffer_{ VK_NULL_HANDLE },
    buffer_desc_{ buffer_desc},
    allocation_{ VMA_NULL }
{}

Image::Image(Image::Desc image_desc) :
    image_{ VK_NULL_HANDLE },
    image_desc_{ image_desc },
    allocation_{ VMA_NULL }
{}

Image::Image(VkImage image, Image::Desc image_desc) :
    image_{ image },
    image_desc_{ image_desc },
    allocation_{ VMA_NULL }
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

ImageView::ImageView(std::shared_ptr<Device> device, const Image& image, ImageView::Lens view_lens) :
    device_{ device },
    image_view_{ VK_NULL_HANDLE }
{
    VkImageViewCreateInfo image_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.GetImage(),
        .viewType = view_lens.view_type,
        .format = image.GetImageDesc().image_format,
        .components = view_lens.component_map,
        .subresourceRange = view_lens.subresource_range,
    };

    vkCreateImageView(device_->GetLogicalDevice(), &image_view_info, nullptr, &image_view_);
}

ImageView::~ImageView() {
    if (image_view_ != VK_NULL_HANDLE) {
        vkDestroyImageView(device_->GetLogicalDevice(), image_view_, nullptr);
        image_view_ = VK_NULL_HANDLE;
    }
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
