#pragma once

#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Window.h"

class Context {
public:
    Context(std::shared_ptr<Window> window);

private:
    std::string app_name_;
    VkInstance instance_;
    
    std::shared_ptr<Device> device_;
    std::shared_ptr<Window> window_;

    std::vector<std::string> requested_validation_layers_;
    std::vector<const char*> enabled_validation_layers_;
    std::vector<std::string> requested_instance_extensions_;
    std::vector<const char*> enabled_instance_extensions_;

    void RequestValidationLayers();
    void RequestInstanceExtensions();
    void CreateInstance();
};
