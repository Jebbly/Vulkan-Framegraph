#include "Window.h"

#include <assert.h>
#include <iostream>

static void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

Window::Window(const std::string& window_name, size_t width, size_t height) :
    window_{ nullptr },
    window_name_{ window_name }, 
    width_{ width }, 
    height_{ height },
    initialized_ {false}
{
    glfwSetErrorCallback(GLFWErrorCallback);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(static_cast<int>(width_), static_cast<int>(height_), window_name_.c_str(), nullptr, nullptr);
    initialized_ = true;
}

Window::~Window() {
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    if (initialized_) {
        glfwTerminate();
        initialized_ = false;
    }
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(window_);
}

bool Window::IsInitialized() const {
    return initialized_;
}

const std::string& Window::GetAppName() const{
    return window_name_;
}

std::vector<std::string> Window::GetRequiredInstanceExtensions() const {
    uint32_t num_glfw_extensions = 0;
    const char** required_glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);

    std::vector<std::string> required_extensions;
    for (uint32_t i = 0; i < num_glfw_extensions; i++) {
        required_extensions.emplace_back(required_glfw_extensions[i]);
    }
    return required_extensions;
}

VkExtent2D Window::GetFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    return VkExtent2D{
        .width = static_cast<uint32_t>(width), 
        .height = static_cast<uint32_t>(height)
    };
}

VkSurfaceKHR Window::CreateSurface(VkInstance instance) const {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window_, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
    return surface;
}

void Window::PollEvents() const {
    glfwPollEvents();
}
