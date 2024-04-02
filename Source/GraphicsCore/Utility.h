#pragma once

#include <stdexcept>
#include <string>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "../CoreUtility/Logging.h"

DEFINE_LOGGER_EXTERN(LogVulkan);

#define VK_CHECK(x)                                                                                                     \
    do {                                                                                                                \
        VkResult result = x;                                                                                            \
        if (result != VK_SUCCESS) {                                                                                     \
            LOG(LogVulkan, Logger::SeverityLevel::FATAL, "Vulkan command failed with: %s", string_VkResult(result));    \
            abort();                                                                                                    \
        }                                                                                                               \
    } while (0);

extern PFN_vkCmdBeginRenderingKHR _vkCmdBeginRenderingKHR;
extern PFN_vkCmdEndRenderingKHR _vkCmdEndRenderingKHR;

