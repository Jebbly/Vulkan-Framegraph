#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "Device.h"

class RenderPass {
public:
    RenderPass(std::shared_ptr<Device> device);
    ~RenderPass();

private:
    std::shared_ptr<Device> device_;

    VkRenderPass render_pass_;
};

class Framebuffer {
public:
    Framebuffer(std::shared_ptr<Device> device);
    ~Framebuffer();

private:
    std::shared_ptr<Device> device_;

    VkFramebuffer framebuffer_;
};