#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Command.h"
#include "Synchronization.h"

class Buffer {
public:
    struct Desc {
        uint32_t buffer_size;
        VkBufferUsageFlags buffer_usage;
    };

    friend class Allocator;

    Buffer(Desc buffer_desc);
    ~Buffer() = default;

    VkBuffer GetBuffer() const {return buffer_;}

private:
    VkBuffer buffer_;
    Desc buffer_desc_;
    VmaAllocation allocation_;
};

class Image {
public:
    struct Desc {
        VkExtent3D image_extent;
        VkFormat image_format;
        VkImageUsageFlags image_usage;
    };

    friend class Allocator;
    friend class ImageView;

    Image(Desc image_desc);
    // This constructor should only be used to wrap images created directly from the swapchain
    Image(VkImage image, Desc image_desc);
    ~Image() = default; // TODO: fix this

    inline const VkImage& GetImage() const {return image_;}
    inline const Desc& GetImageDesc() const {return image_desc_;}

    void TransitionImage(CommandBuffer command_buffer, VkImageLayout old_layout, VkImageLayout new_layout) const;

private:
    VkImage image_;
    Desc image_desc_;
    VmaAllocation allocation_;
};

class ImageView {
public:
    struct Lens {
        VkImageViewType view_type;
        VkComponentMapping component_map;
        VkImageSubresourceRange subresource_range;
    };

    ImageView(std::shared_ptr<Device> device, std::shared_ptr<Image> image, ImageView::Lens view_lens);
    ~ImageView();

    VkImageView GetImageView() const {return image_view_;}

private:
    std::shared_ptr<Device> device_;

    VkImageView image_view_;
};

// TODO: implement this
class Sampler {
public:
    Sampler();
    ~Sampler();

    VkSampler GetSampler() const {return sampler_;}

private:
    VkSampler sampler_;
};

struct ResourceDesc {
    VmaAllocationCreateFlags allocation_flags;
    VmaMemoryUsage memory_usage;
    VkSharingMode sharing_mode;
    std::vector<uint32_t> queue_families;
};

class Allocator {
public:
    Allocator(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device);
    ~Allocator();

    std::shared_ptr<Buffer> AllocateBuffer(Buffer::Desc& buffer_desc, ResourceDesc resource_desc);
    std::shared_ptr<Image> AllocateImage(Image::Desc& image_desc, ResourceDesc resource_desc);

private:
    std::shared_ptr<Instance> instance_;
    std::shared_ptr<Device> device_;

    VmaAllocator allocator_;

    void CreateAllocator();
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
                               VkImageLayout old_layout, VkImageLayout new_layout, 
                               Image image, VkImageSubresourceRange range);

    void InsertIntoCommandBuffer(CommandBuffer command_buffer);

private:
    VkDependencyFlags dependency_flags_;
    std::vector<VkMemoryBarrier2> memory_barriers_;
    std::vector<VkBufferMemoryBarrier2> buffer_barriers_;
    std::vector<VkImageMemoryBarrier2> image_barriers_;
};
