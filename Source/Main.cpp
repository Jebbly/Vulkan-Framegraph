#include <iostream>

#include "GraphicsCore/Window.h"
#include "GraphicsCore/Context.h"

int main() {
    Context context{ "Vulkan Rendergraph", 1600, 900 };
    std::shared_ptr<Window> window = context.GetWindow();
    
    while (!window->ShouldClose()) {
        window->PollEvents();
    }
}
