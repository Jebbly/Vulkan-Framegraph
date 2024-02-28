#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class Instance {
public:
    Instance(const std::string& app_name, 
             const std::vector<std::string>& requested_validation_layers, 
             const std::vector<std::string>& requested_instance_extensions);
    ~Instance();

    inline const VkInstance& GetInstance() const {return instance_;}

private:
    std::string app_name_;
    VkInstance instance_;

    std::vector<const char*> enabled_validation_layers_;
    std::vector<const char*> enabled_instance_extensions_;

    void RequestValidationLayers(const std::vector<std::string>& requested_validation_layers);
    void RequestInstanceExtensions(const std::vector<std::string>& requested_instance_extensions);
    void CreateInstance();
};