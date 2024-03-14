#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Instance.h"
#include "Resources.h"
#include "Synchronization.h"
#include "Window.h"

class Swapchain {
public:
    Swapchain(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device, std::shared_ptr<Window> window);
    ~Swapchain();

    uint32_t AcquireNextImage(const Semaphore& present_semaphore);
    void Present(uint32_t image_index, const Semaphore& wait_semaphore);

    std::shared_ptr<Image> GetImage(uint32_t image_index) const {return images_.at(image_index);}

private:
    std::shared_ptr<Instance> instance_;
    std::shared_ptr<Device> device_;
    std::shared_ptr<Window> window_;
    VkSurfaceKHR surface_;

    VkSwapchainKHR swapchain_;
    std::vector<std::shared_ptr<Image>> images_;
    std::vector<std::shared_ptr<ImageView>> image_views_;
    VkSurfaceCapabilitiesKHR capabilities_;
    VkSurfaceFormatKHR surface_format_;
    VkExtent2D surface_extent_;

    void GetCapabilities();
    void ChooseFormat();
    void ChooseExtent();
    void CreateSwapchain();
    void GetSwapchainImages();
};