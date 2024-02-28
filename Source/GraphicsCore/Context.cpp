#include "Context.h"

#include <assert.h>
#include <iostream>

Context::Context(const std::string& app_name, size_t width, size_t height) :
    app_name_{ app_name }
{
    window_ = std::make_shared<Window>(app_name, width, height);

    // Request validation layers and instance extensions for instance creation.
    std::vector<std::string> requested_validation_layers;
    requested_validation_layers.push_back("VK_LAYER_KHRONOS_validation");

    std::vector<std::string> requested_instance_extensions;
    requested_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    // The window may also come with platform specific dependencies,
    // although some of which might already be requested.
    // This technically isn't necessary, but it helps for logging.
    std::vector<std::string> required_window_extensions = window_->GetRequiredInstanceExtensions();
    for (const std::string& window_extension : required_window_extensions) {
        bool already_requested = false;

        for (const std::string& requested_extension : requested_instance_extensions) {
            if (window_extension == requested_extension) {
                already_requested = true;
                break;
            }
        }

        if (!already_requested) {
            requested_instance_extensions.push_back(window_extension);
        }
    }

    instance_ = std::make_shared<Instance>(app_name_, requested_validation_layers, requested_instance_extensions);

    // Here, we create a temporary surface to query for device capabilities.
    // However, the actual surface is created in the swapchain constructor.
    // TODO: handle this better? The issue with passing the instance to window creation
    // is that the instance already relies on the window to query for extensions,
    // so there is currently a circular dependency between the two classes.
    VkSurfaceKHR surface = window_->CreateSurface(instance_->GetInstance());
    device_ = std::make_shared<Device>(instance_, surface);
    vkDestroySurfaceKHR(instance_->GetInstance(), surface, nullptr);

    swapchain_ = std::make_shared<Swapchain>(instance_, device_, window_);
}
