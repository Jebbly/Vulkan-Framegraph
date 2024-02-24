#include "Device.h"

#include <assert.h>
#include <functional>
#include <iostream>
#include <map>
#include <vector>

Device::Device(const VkInstance instance) :
	instance_{ instance },
	device_features_{ }
{
	// Request device extensions and features
	requested_device_extensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	device_features_.samplerAnisotropy = VK_TRUE;

	SelectPhysicalDevice();
	RequestDeviceExtensions();
	FindQueueFamilies();
	CreateLogicalDeviceAndQueues();
}

Device::~Device() {
	if (logical_device_ != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(logical_device_);
		vkDestroyDevice(logical_device_, nullptr);
	}
}

void Device::SelectPhysicalDevice() {
	uint32_t num_physical_devices = 0;
	vkEnumeratePhysicalDevices(instance_, &num_physical_devices, nullptr);
	assert(num_physical_devices > 0);
	std::vector<VkPhysicalDevice> available_physical_devices(num_physical_devices);
	vkEnumeratePhysicalDevices(instance_, &num_physical_devices, available_physical_devices.data());

	physical_device_ = available_physical_devices[0];

#ifndef NDEBUG
	std::cout << "Available Physical Devices:" << std::endl;
	for (const VkPhysicalDevice& physical_device : available_physical_devices) {
		VkPhysicalDeviceProperties physical_device_properties;
		vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
		std::cout << "\t" << physical_device_properties.deviceName << std::endl;
	}
	std::cout << std::endl;
#endif

	for (const VkPhysicalDevice& physical_device : available_physical_devices) {
		VkPhysicalDeviceProperties physical_device_properties;
		vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
		if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			physical_device_ = physical_device;
			break;
		}
	}

#ifndef NDEBUG
	std::cout << "Selected Physical Device:" << std::endl;
	VkPhysicalDeviceProperties physical_device_properties;
	vkGetPhysicalDeviceProperties(physical_device_, &physical_device_properties);
	std::cout << "\t" << physical_device_properties.deviceName << std::endl;
	std::cout << std::endl;
#endif
}

void Device::RequestDeviceExtensions() {
	uint32_t num_device_extensions = 0;
	vkEnumerateDeviceExtensionProperties(physical_device_, nullptr, &num_device_extensions, nullptr);
	std::vector<VkExtensionProperties> available_extensions(num_device_extensions);
	vkEnumerateDeviceExtensionProperties(physical_device_, nullptr, &num_device_extensions, available_extensions.data());

#ifndef NDEBUG
	std::cout << "Available Device Extensions:" << std::endl;
	for (const VkExtensionProperties& available_extension : available_extensions) {
		std::cout << "\t" << available_extension.extensionName << std::endl;
	}
	std::cout << std::endl;
#endif

	// Check that requested device extensions are available.
	// TODO: only check for extensions that are optional.
	for (const std::string& requested_extension : requested_device_extensions_) {
		const char* requested_extension_name = requested_extension.c_str();

		bool found_requested_extension = false;
		for (const VkExtensionProperties& available_extension : available_extensions) {
			if (strcmp(requested_extension_name, available_extension.extensionName) == 0) {
				enabled_device_extensions_.emplace_back(requested_extension_name);
				found_requested_extension = true;
				break;
			}
		}

		if (!found_requested_extension) {
			throw std::runtime_error("Device extension not available: " + requested_extension);
		}
	}

#ifndef NDEBUG
	std::cout << "Enabled Device Extensions: " << std::endl;
	for (const char* enabled_extension : enabled_device_extensions_) {
		std::cout << "\t" << enabled_extension << std::endl;
	}
	std::cout << std::endl;
#endif
}

void Device::FindQueueFamilies() {
	uint32_t num_queue_families = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &num_queue_families, nullptr);
	std::vector<VkQueueFamilyProperties> available_queue_families(num_queue_families);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &num_queue_families, available_queue_families.data());

	// This takes a function as an argument and iterates through all queue families.
	// If the function returns true, then return the corresponding queue family index.
	auto search =
		[&](const std::function<bool(const VkQueueFamilyProperties&, uint32_t)> search_function) {
			std::optional<uint32_t> result{};
			for (uint32_t queue_family_index = 0; queue_family_index < num_queue_families; queue_family_index++) {
				if (search_function(available_queue_families[queue_family_index], queue_family_index)) {
					result = queue_family_index;
					break;
				}
			}
			return result;
		};

	// Prefer a queue family has supports both graphics and present queues
	auto supports_graphics_present =
		[](const VkQueueFamilyProperties& candidate_queue_family, uint32_t queue_family_index) {
			return (candidate_queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
		};

	auto graphics_present_family = search(supports_graphics_present);
	if (graphics_present_family.has_value()) {
		queues_[QueueType::GRAPHICS].queue_family = graphics_present_family.value();
		queues_[QueueType::PRESENT].queue_family = graphics_present_family.value();
	}

	// Prefer a dedicated compute queue family
	auto supports_dedicated_compute =
		[](const VkQueueFamilyProperties& candidate_queue_family, uint32_t queue_family_index) {
			return (((candidate_queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
				((candidate_queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0));
		};

	auto dedicated_compute_family = search(supports_dedicated_compute);
	if (dedicated_compute_family.has_value()) {
		queues_[QueueType::COMPUTE].queue_family = dedicated_compute_family.value();
	} else {
		auto supports_compute =
			[](const VkQueueFamilyProperties& candidate_queue_family, uint32_t queue_family_index) {
				return ((candidate_queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0);
			};

		auto compute_family = search(supports_compute);
		if (compute_family.has_value()) {
			queues_[QueueType::COMPUTE].queue_family = compute_family.has_value();
		} else {
			throw std::runtime_error("No suitable compute queue family found!");
		}
	}

	// Prefer a dedicated transfer queue family
	auto supports_dedicated_transfer =
		[](const VkQueueFamilyProperties& candidate_queue_family, uint32_t queue_family_index) {
			return (((candidate_queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
					((candidate_queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0) &&
					((candidate_queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0));
		};

	auto dedicated_transfer_family = search(supports_dedicated_transfer);
	if (dedicated_transfer_family.has_value()) {
		queues_[QueueType::TRANSFER].queue_family = dedicated_transfer_family.has_value();
	} else {
		auto supports_transfer =
			[](const VkQueueFamilyProperties& candidate_queue_family, uint32_t queue_family_index) {
				return ((candidate_queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0);
			};

		auto transfer_family = search(supports_transfer);
		if (transfer_family.has_value()) {
			queues_[QueueType::TRANSFER].queue_family = transfer_family.value();
		} else {
			throw std::runtime_error("No suitable transfer queue family found!");
		}
	}

	// Once all queues are found, check what their queues should be in their respective families.
	std::vector<uint32_t> queue_family_counts(num_queue_families, 0);
	for (int queue_type = 0; queue_type < QueueType::MAX_QUEUE_TYPES; queue_type++) {
		Queue& queue = queues_[static_cast<uint32_t>(queue_type)];
		queue.queue_index = queue_family_counts[queue.queue_family]++;
		queue.queue_priority = 1.0f;
	}
}

void Device::CreateLogicalDeviceAndQueues() {
	// The device creation info only takes unique queue family indices,
	// so we need to iterate over all queues and collect info on the unique queue families.
	typedef struct {
		uint32_t queue_count;
		std::vector<float> queue_priorities;
	} UniqueQueueFamily;

	std::map<uint32_t, UniqueQueueFamily> unique_queue_families;
	for (int queue_type = 0; queue_type < QueueType::MAX_QUEUE_TYPES; queue_type++) {
		const Queue& queue = queues_[static_cast<uint32_t>(queue_type)];
		UniqueQueueFamily& unique_queue_family = unique_queue_families[queue.queue_family];
		unique_queue_family.queue_count++;
		unique_queue_family.queue_priorities.push_back(queue.queue_priority);
	}

	std::vector<VkDeviceQueueCreateInfo> queue_infos;
	for (const auto& [queue_family_index, unique_queue_family] : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queue_family_index,
			.queueCount = unique_queue_family.queue_count,
			.pQueuePriorities = unique_queue_family.queue_priorities.data(),
		};
		queue_infos.push_back(queue_info);
	}

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
		.pQueueCreateInfos = queue_infos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(enabled_device_extensions_.size()),
		.ppEnabledExtensionNames = enabled_device_extensions_.data(),
		.pEnabledFeatures = &device_features_,
	};

	if (vkCreateDevice(physical_device_, &device_info, nullptr, &logical_device_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device");
	}

	// Get the actual queue indices
	for (int queue_type = 0; queue_type < QueueType::MAX_QUEUE_TYPES; queue_type++) {
		Queue& queue = queues_[static_cast<uint32_t>(queue_type)];
		vkGetDeviceQueue(logical_device_, queue.queue_family, queue.queue_index, &queue.queue);
	}
}