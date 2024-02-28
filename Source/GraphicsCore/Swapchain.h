#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Instance.h"
#include "Window.h"

class Swapchain {
public:
    Swapchain(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device, std::shared_ptr<Window> window);
    ~Swapchain();

private:
    std::shared_ptr<Instance> instance_;
    std::shared_ptr<Device> device_;
    std::shared_ptr<Window> window_;
    VkSurfaceKHR surface_;

    VkSwapchainKHR swapchain_;
    std::vector<VkImage> images_;
    VkSurfaceCapabilitiesKHR capabilities_;
    VkSurfaceFormatKHR surface_format_;
    VkExtent2D surface_extent_;

    void GetCapabilities();
    void ChooseFormat();
    void ChooseExtent();
    void CreateSwapchain();
    void GetSwapchainImages();
};