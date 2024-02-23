#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "Window.h"

class Context {
public:
	Context(const std::string& app_name);

private:
	std::string app_name_;
	VkInstance instance_;

	std::vector<const char*> enabled_validation_layers_;
	std::vector<const char*> enabled_instance_extensions_;

	void RequestValidationLayers();
	void RequestInstanceExtensions();
	void CreateInstance();
};