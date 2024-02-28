#pragma once

#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "Window.h"

class Context {
public:
    Context(const std::string& app_name, size_t width, size_t height);
    ~Context() = default; 

    std::shared_ptr<Window> GetWindow() {return window_;}

private:
    std::string app_name_;
    
    // Note: order is important here for order of destruction
    std::shared_ptr<Window> window_;
    std::shared_ptr<Instance> instance_;
    std::shared_ptr<Device> device_;
    std::shared_ptr<Swapchain> swapchain_;
};
