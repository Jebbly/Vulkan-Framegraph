#include "Swapchain.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>

Swapchain::Swapchain(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device, std::shared_ptr<Window> window) :
    instance_{ instance },
    device_{ device },
    window_{ window },
    swapchain_{ VK_NULL_HANDLE },
    surface_{ window_->CreateSurface(instance_->GetInstance()) }
{
    GetCapabilities();
    ChooseFormat();
    ChooseExtent();
    CreateSwapchain();
    GetSwapchainImages();
}

Swapchain::~Swapchain()
{
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_->GetLogicalDevice(), swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
    
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_->GetInstance(), surface_, nullptr);
    }
}

uint32_t Swapchain::AcquireNextImage(const Semaphore& present_semaphore) {
    uint32_t image_index;
    vkAcquireNextImageKHR(device_->GetLogicalDevice(), swapchain_, 1000000000, present_semaphore.GetSemaphore(), nullptr, &image_index);
    return image_index;
}

void Swapchain::Present(uint32_t image_index, const Semaphore& wait_semaphore) {
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &wait_semaphore.GetSemaphore(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain_,
        .pImageIndices = &image_index,
    };

    if (vkQueuePresentKHR(device_->GetQueue(Device::QueueType::PRESENT).queue, &present_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swapchain image!");
    }
}

void Swapchain::GetCapabilities() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_->GetPhysicalDevice(), surface_, &capabilities_);
}

void Swapchain::ChooseFormat() {
    uint32_t num_formats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_->GetPhysicalDevice(), surface_, &num_formats, nullptr);
    assert(num_formats != 0);
    std::vector<VkSurfaceFormatKHR> available_formats(num_formats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_->GetPhysicalDevice(), surface_, &num_formats, available_formats.data());

    for (const VkSurfaceFormatKHR available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            surface_format_ = available_format;
            return;
        }
    }

    surface_format_ = available_formats[0];
}

void Swapchain::ChooseExtent() {
    if (capabilities_.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        surface_extent_ = capabilities_.currentExtent;
        return;
    }

    surface_extent_ = window_->GetFramebufferSize();
    surface_extent_.width = std::clamp(surface_extent_.width, 
                                       capabilities_.minImageExtent.width, capabilities_.maxImageExtent.width);
    surface_extent_.height = std::clamp(surface_extent_.height,
                                        capabilities_.minImageExtent.height, capabilities_.maxImageExtent.height);
}

void Swapchain::CreateSwapchain() {
    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface_,
        .minImageCount = capabilities_.minImageCount + 1,
        .imageFormat = surface_format_.format,
        .imageColorSpace = surface_format_.colorSpace,
        .imageExtent = surface_extent_,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = capabilities_.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    const Device::Queue& graphics_queue = device_->GetQueue(Device::QueueType::GRAPHICS);
    const Device::Queue& present_queue = device_->GetQueue(Device::QueueType::PRESENT);
    uint32_t queue_families[] = {graphics_queue.queue_family, present_queue.queue_family};
    if (graphics_queue.queue_family != present_queue.queue_family) {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_families;
    } else {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0; 
        swapchain_info.pQueueFamilyIndices = nullptr; 
    }

    if (vkCreateSwapchainKHR(device_->GetLogicalDevice(), &swapchain_info, nullptr, &swapchain_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain!");
    }
}

void Swapchain::GetSwapchainImages() {
    uint32_t num_images = 0;
    vkGetSwapchainImagesKHR(device_->GetLogicalDevice(), swapchain_, &num_images, nullptr);
    std::vector<VkImage> vulkan_images(num_images);
    vkGetSwapchainImagesKHR(device_->GetLogicalDevice(), swapchain_, &num_images, vulkan_images.data());

    for (VkImage& vulkan_image : vulkan_images) {
        VkExtent3D extent = {
            .width = surface_extent_.width,
            .height = surface_extent_.height,
            .depth = 1,
        };

        Image::Desc image_desc = {
            .image_extent = extent,
            .image_format = surface_format_.format,
            .image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        };

        images_.emplace_back(vulkan_image, image_desc);
    }

    for (uint32_t i = 0; i < num_images; i++) {
        VkComponentMapping default_mapping = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        };

        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        ImageView::Lens lens = {
            .view_type = VK_IMAGE_VIEW_TYPE_2D,
            .component_map = default_mapping,
            .subresource_range = range,
        };

        image_views_.emplace_back(ImageView{device_, images_[i], lens});
    }
}