#pragma once

#include <vulkan/vulkan.h>

class Device {
public:
	Device();

private:
	VkDevice logical_device_;
	VkPhysicalDevice physical_device_;

	void SelectPhysicalDevice();
	void CreateLogicalDevice();
};