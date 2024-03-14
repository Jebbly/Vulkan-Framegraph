#include <iostream>

#include "GraphicsCore/Window.h"
#include "GraphicsCore/Command.h"
#include "GraphicsCore/Context.h"
#include "GraphicsCore/Resources.h"
#include "GraphicsCore/Shader.h"
#include "GraphicsCore/Synchronization.h"

int main() {
    Context context{ "Vulkan Rendergraph", 1600, 900 };
    std::shared_ptr<Window> window = context.GetWindow();

    Fence render_fence{context.GetDevice(), true};
    Semaphore image_available_semaphore{context.GetDevice()};
    Semaphore render_finished_semaphore{context.GetDevice()};

    CommandPool command_pool{context.GetDevice(), Device::QueueType::GRAPHICS};
    CommandBuffer main_command = command_pool.AllocateSinglePrimaryCommandBuffer();

    ShaderCompiler compiler;
    compiler.LoadShader("C:\\Users\\JeffL\\Desktop\\Personal-Projects\\Vulkan-Rendergraph\\Shaders\\HelloWorld.slang", "compute_main");
    
    while (!window->ShouldClose()) {
        window->PollEvents();

        render_fence.Wait(1000000000);
        render_fence.Reset();

        uint32_t swapchain_index = context.GetSwapchain()->AcquireNextImage(image_available_semaphore);
        std::shared_ptr<Image> swapchain_image = context.GetSwapchain()->GetImage(swapchain_index);

        main_command.Reset();
        main_command.Begin(true);

        swapchain_image->TransitionImage(main_command, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        main_command.Record([&](VkCommandBuffer command) {
            VkClearValue clear_value = {
                .color = { 0.0f, 0.0f, 0.0f, 1.0f },
            };

            VkImageSubresourceRange clear_range = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            };

            vkCmdClearColorImage(command, swapchain_image->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clear_value.color, 1, &clear_range);
        });

        swapchain_image->TransitionImage(main_command, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        main_command.End();

        main_command.InsertWaitSemaphore(image_available_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
        main_command.InsertSignalSemaphore(render_finished_semaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

        main_command.Submit(&render_fence);

        context.GetSwapchain()->Present(swapchain_index, render_finished_semaphore);
    }
}
