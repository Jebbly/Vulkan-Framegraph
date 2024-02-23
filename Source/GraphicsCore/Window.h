#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

class Window {
public:
	Window(const std::string& window_name, size_t width, size_t height);
	~Window();

	bool ShouldClose() const;

private:
	GLFWwindow* window_;
	std::string window_name_;
	size_t width_, height_;

	VkSurfaceKHR surface_;
};