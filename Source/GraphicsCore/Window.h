#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

class Window {
public:
    Window(const std::string& window_name, size_t width, size_t height);
    ~Window();

    bool ShouldClose() const;
    bool IsInitialized() const;
    const std::string& GetAppName() const;
    std::vector<std::string> GetRequiredInstanceExtensions() const;
    VkExtent2D GetFramebufferSize() const;

    void PollEvents() const;

    VkSurfaceKHR CreateSurface(VkInstance instance) const;

private:
    GLFWwindow* window_;
    std::string window_name_;
    size_t width_, height_;

    bool initialized_;
};
