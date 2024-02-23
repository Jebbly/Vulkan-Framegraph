#include "Window.h"

static void GLFWErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << std::endl;
}

Window::Window(const std::string& window_name, size_t width, size_t height) :
	window_name_{ window_name }, 
	width_{ width }, 
	height_{ height }
{
	glfwSetErrorCallback(GLFWErrorCallback);

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
}

Window::~Window() {
	glfwDestroyWindow(window_);
}

bool Window::ShouldClose() const {
	return glfwWindowShouldClose(window_);
}