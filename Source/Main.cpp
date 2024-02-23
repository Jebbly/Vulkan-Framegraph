#include <iostream>

#include <glm/glm.hpp>
#include <slang.h>
#include <vulkan/vulkan.h>

#include "GraphicsCore/Window.h"
#include "GraphicsCore/Context.h"

int main() {
    Window window{ "Vulkan Rendergraph", 1600, 900 };
    Context context{ "Vulkan Rendergraph" };
    
    while (!window.ShouldClose()) {
        std::cout << "Running..." << std::endl;
    }
}
