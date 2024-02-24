#include <iostream>

#include "GraphicsCore/Window.h"
#include "GraphicsCore/Context.h"

int main() {
    std::shared_ptr<Window> window = std::make_shared<Window>("Vulkan Rendergraph", 1600, 900);
    Context context{ window };
    
    while (!window->ShouldClose()) {
    }
}
