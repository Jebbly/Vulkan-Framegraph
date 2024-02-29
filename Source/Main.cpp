#include <iostream>

#include "GraphicsCore/Window.h"
#include "GraphicsCore/Command.h"
#include "GraphicsCore/Context.h"
#include "GraphicsCore/Synchronization.h"

VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask)
{
    VkImageSubresourceRange subImage{};
    subImage.aspectMask = aspectMask;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return subImage;
}

void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange = ImageSubresourceRange(aspectMask);
    imageBarrier.image = image;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

int main() {
    Context context{ "Vulkan Rendergraph", 1600, 900 };
    std::shared_ptr<Window> window = context.GetWindow();

    Fence render_fence{context.GetDevice(), true};
    Semaphore image_available_semaphore{context.GetDevice()};
    Semaphore render_finished_semaphore{context.GetDevice()};

    CommandPool command_pool{context.GetDevice(), Device::QueueType::GRAPHICS};
    CommandBuffer main_command = command_pool.AllocateSinglePrimaryCommandBuffer();
    
    while (!window->ShouldClose()) {
        window->PollEvents();

        render_fence.Wait(1000000000);
        render_fence.Reset();

        main_command.Reset();
        main_command.Begin(true);

        uint32_t swapchain_index = context.GetSwapchain()->AcquireNextImage(image_available_semaphore);
        const VkImage& swapchain_image = context.GetSwapchain()->GetImage(swapchain_index);

        main_command.Record([&](VkCommandBuffer command) {
            TransitionImage(command, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

            VkClearColorValue clear_value;
            clear_value = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            VkImageSubresourceRange clear_range = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

            vkCmdClearColorImage(command, swapchain_image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_range);

            TransitionImage(command, swapchain_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        });

        main_command.End();

        main_command.InsertWaitSemaphore(image_available_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
        main_command.InsertSignalSemaphore(render_finished_semaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

        main_command.Submit(&render_fence);

        context.GetSwapchain()->Present(swapchain_index, render_finished_semaphore);
    }
}
