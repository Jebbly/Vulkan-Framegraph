#include "Instance.h"

#include "Utility.h"

Instance::Instance(const std::string& app_name,
                   const std::vector<std::string>& requested_validation_layers,
                   const std::vector<std::string>& requested_instance_extensions) :
    app_name_{ app_name },
    instance_{ VK_NULL_HANDLE }
{
    RequestValidationLayers(requested_validation_layers);
    RequestInstanceExtensions(requested_instance_extensions);
    CreateInstance();
}

Instance::~Instance() {
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
}

void Instance::RequestValidationLayers(const std::vector<std::string>& requested_validation_layers) {
    uint32_t num_validation_layers = 0;
    vkEnumerateInstanceLayerProperties(&num_validation_layers, nullptr);
    std::vector<VkLayerProperties> available_validation_layers(num_validation_layers);
    vkEnumerateInstanceLayerProperties(&num_validation_layers, available_validation_layers.data());

    LOG(LogVulkan, Logger::SeverityLevel::INFO, "Available Validation Layers:");
    for (const VkLayerProperties& available_layer : available_validation_layers) {
        LOG(LogVulkan, Logger::SeverityLevel::INFO, "\t{0}", available_layer.layerName);
    }

    // Check that requested validation layers are available.
    // TODO: only check for layers that are optional.
    for (const std::string& requested_layer : requested_validation_layers) {
        const char* requested_layer_name = requested_layer.c_str();

        bool found_requested_layer = false;
        for (const VkLayerProperties& available_layer : available_validation_layers) {
            if (strcmp(requested_layer_name, available_layer.layerName) == 0) {
                enabled_validation_layers_.emplace_back(requested_layer_name);
                found_requested_layer = true;
                break;
            }
        }

        if (!found_requested_layer) {
            throw std::runtime_error("Validation layer not available: " + requested_layer);
        }
    }

    LOG(LogVulkan, Logger::SeverityLevel::INFO, "Enabled Validation Layers:");
    for (const char* enabled_layer : enabled_validation_layers_) {
        LOG(LogVulkan, Logger::SeverityLevel::INFO, "\t{0}", enabled_layer);
    }
}

void Instance::RequestInstanceExtensions(const std::vector<std::string>& requested_instance_extensions) {
    uint32_t num_instance_extensions = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
    std::vector<VkExtensionProperties> available_extensions(num_instance_extensions);
    vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, available_extensions.data());

    LOG(LogVulkan, Logger::SeverityLevel::INFO, "Available Instance Extensions:");
    for (const VkExtensionProperties& available_extension : available_extensions) {
        LOG(LogVulkan, Logger::SeverityLevel::INFO, "\t{0}", available_extension.extensionName);
    }

    // Check that requested instance extensions are available.
    // TODO: only check for extensions that are optional.
    for (const std::string& requested_extension : requested_instance_extensions) {
        const char* requested_extension_name = requested_extension.c_str();

        bool found_requested_extension = false;
        for (const VkExtensionProperties& available_extension : available_extensions) {
            if (strcmp(requested_extension_name, available_extension.extensionName) == 0) {
                enabled_instance_extensions_.emplace_back(requested_extension_name);
                found_requested_extension = true;
                break;
            }
        }

        if (!found_requested_extension) {
            throw std::runtime_error("Instance extension not available: " + requested_extension);
        }
    }

    LOG(LogVulkan, Logger::SeverityLevel::INFO, "Enabled Instance Extensions:");
    for (const char* enabled_extension : enabled_instance_extensions_) {
        LOG(LogVulkan, Logger::SeverityLevel::INFO, "\t{0}", enabled_extension);
    }
}

void Instance::CreateInstance() {
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app_name_.c_str(),
        .apiVersion = VK_HEADER_VERSION_COMPLETE,
    };

    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(enabled_validation_layers_.size()),
        .ppEnabledLayerNames = enabled_validation_layers_.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabled_instance_extensions_.size()),
        .ppEnabledExtensionNames = enabled_instance_extensions_.data(),
    };

    if (vkCreateInstance(&instance_info, nullptr, &instance_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}
