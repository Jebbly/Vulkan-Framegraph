#include <iostream>

#include "GraphicsCore/Window.h"
#include "GraphicsCore/Command.h"
#include "GraphicsCore/Context.h"
#include "GraphicsCore/Resources.h"
#include "GraphicsCore/Pipeline.h"
#include "GraphicsCore/Shader.h"
#include "GraphicsCore/Synchronization.h"
#include "GraphicsCore/Utility.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::shared_ptr<Image> LoadTexture(Allocator& allocator, CommandPool& command_pool, const std::string& file_name) {
    int width, height, channels;
    stbi_uc* pixels = stbi_load(file_name.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize image_size = width * height * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    std::shared_ptr<Buffer> stage_buffer = allocator.AllocateBuffer({
        .buffer_size = static_cast<uint32_t>(width) * static_cast<uint32_t>(height) * 4,
        .buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .resource_desc = {
            .allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        }
    });

    void* data = stage_buffer->MapToCPU();
    memcpy(data, pixels, static_cast<size_t>(image_size));
    stage_buffer->UnmapFromCPU();

    stbi_image_free(pixels);

    std::shared_ptr<Image> texture = allocator.AllocateImage({
        .image_extent = {
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
            .depth = 1,
        },
        .image_format = VK_FORMAT_R8G8B8A8_SRGB,
        .image_usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
     });

    CommandBuffer copy_command = command_pool.AllocateSinglePrimaryCommandBuffer();
    copy_command.Begin(true);
    texture->TransitionImage(copy_command, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy from buffer to image
    copy_command.Record([&](VkCommandBuffer command) {
        VkImageSubresourceLayers image_subresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        VkExtent3D image_extent = {
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
            .depth = 1,
        };

        VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = image_subresource,
            .imageOffset = {0, 0, 0},
            .imageExtent = image_extent,
        };

        vkCmdCopyBufferToImage(
            command,
            stage_buffer->GetBuffer(),
            texture->GetImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    });

    texture->TransitionImage(copy_command, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    copy_command.End();
    copy_command.Submit();
}

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 tex_coord;

    bool operator==(const Vertex& other) const {
        return position == other.position && color == other.color && tex_coord == other.tex_coord;
    }
};

std::shared_ptr<Buffer> CreateVertexBuffer(Allocator& allocator, CommandPool& command_pool, const std::vector<Vertex> vertices)
{
    uint32_t size = static_cast<uint32_t>(sizeof(Vertex)) * static_cast<uint32_t>(vertices.size());
    std::shared_ptr<Buffer> stage_buffer = allocator.AllocateBuffer({
        .buffer_size = size,
        .buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .resource_desc = {
            .allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        }
    });

    void* data = stage_buffer->MapToCPU();
    memcpy(data, vertices.data(), (size_t) size);
    stage_buffer->UnmapFromCPU();

    std::shared_ptr<Buffer> vertex_buffer = allocator.AllocateBuffer({
        .buffer_size = size,
        .buffer_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    });

    CommandBuffer copy_command = command_pool.AllocateSinglePrimaryCommandBuffer();
    copy_command.Begin(true);

    copy_command.Record([&](VkCommandBuffer command) {
        VkBufferCopy copy_info = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };

        vkCmdCopyBuffer(command, stage_buffer->GetBuffer(), vertex_buffer->GetBuffer(), 1, &copy_info);
    });
    
    copy_command.End();
    copy_command.Submit();

    return vertex_buffer;
}

std::shared_ptr<Buffer> CreateIndexBuffer(Allocator& allocator, CommandPool& command_pool, const std::vector<uint32_t> indices)
{
    uint32_t size = static_cast<uint32_t>(sizeof(uint32_t)) * static_cast<uint32_t>(indices.size());
    std::shared_ptr<Buffer> stage_buffer = allocator.AllocateBuffer({
        .buffer_size = size,
        .buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .resource_desc = {
            .allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        }
    });

    void* data = stage_buffer->MapToCPU();
    memcpy(data, indices.data(), (size_t) size);
    stage_buffer->UnmapFromCPU();

    std::shared_ptr<Buffer> index_buffer = allocator.AllocateBuffer({
        .buffer_size = size,
        .buffer_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    });

    CommandBuffer copy_command = command_pool.AllocateSinglePrimaryCommandBuffer();
    copy_command.Begin(true);

    copy_command.Record([&](VkCommandBuffer command) {
        VkBufferCopy copy_info = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };

        vkCmdCopyBuffer(command, stage_buffer->GetBuffer(), index_buffer->GetBuffer(), 1, &copy_info);
    });
    
    copy_command.End();
    copy_command.Submit(); 

    return index_buffer;
}


int main() {
    Context context{ "Vulkan Rendergraph", 1600, 900 };
    std::shared_ptr<Window> window = context.GetWindow();

    Allocator allocator{ context.GetInstance(), context.GetDevice() };
    DescriptorPool descriptor_pool{ context.GetDevice() };
    CommandPool command_pool{ context.GetDevice(), Device::QueueType::GRAPHICS };

    Fence render_fence{context.GetDevice(), true};
    Semaphore image_available_semaphore{context.GetDevice()};
    Semaphore render_finished_semaphore{context.GetDevice()};

    CommandBuffer main_command = command_pool.AllocateSinglePrimaryCommandBuffer();

    std::cout << "Loading shader compiler" << std::endl;
    ShaderCompiler compiler{context.GetDevice()};
    std::cout << "Getting vertex shader" << std::endl;
    std::shared_ptr<Shader> triangle_vertex_shader = compiler.LoadShader("HelloWorldGraphics", "vertex_main");
    std::cout << "Getting fragment shader" << std::endl;
    std::shared_ptr<Shader> triangle_fragment_shader = compiler.LoadShader("HelloWorldGraphics", "fragment_main");
    GraphicsPipeline triangle_pipeline{context.GetDevice(), 
        {
            .vertex_shader = triangle_vertex_shader, 
            .fragment_shader = triangle_fragment_shader,
        },
        {
            .color_formats = {context.GetSwapchain()->GetImage(0)->GetImageDesc().image_format}
        }
    };

    std::vector<Vertex> quad_vertices = {
        Vertex{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
        Vertex{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f) }
    };
    std::shared_ptr<Buffer> vertex_buffer = CreateVertexBuffer(allocator, command_pool, quad_vertices);

    context.GetDevice()->WaitIdle();
    
    std::vector<uint32_t> quad_indices{0, 1, 2, 0, 2, 3};
    std::shared_ptr<Buffer> index_buffer = CreateIndexBuffer(allocator, command_pool, quad_indices);

    context.GetDevice()->WaitIdle();
    
    while (!window->ShouldClose()) {
        window->PollEvents();

        render_fence.Wait(1000000000);
        render_fence.Reset();

        uint32_t swapchain_index = context.GetSwapchain()->AcquireNextImage(image_available_semaphore);
        std::shared_ptr<Image> swapchain_image = context.GetSwapchain()->GetImage(swapchain_index);
        std::shared_ptr<ImageView> swapchain_image_view = context.GetSwapchain()->GetImageView(swapchain_index);

        main_command.Reset();
        main_command.Begin(true);

        swapchain_image->TransitionImage(main_command, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        triangle_pipeline.Bind(main_command);

        main_command.Record([&](VkCommandBuffer command) {
            VkViewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(swapchain_image_view->GetImageDesc().image_extent.width),
                .height = static_cast<float>(swapchain_image_view->GetImageDesc().image_extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };

            vkCmdSetViewport(command, 0, 1, &viewport);

            VkRect2D scissor = {
                .offset = {0, 0},
                .extent = {
                    .width = static_cast<uint32_t>(swapchain_image_view->GetImageDesc().image_extent.width),
                    .height = static_cast<uint32_t>(swapchain_image_view->GetImageDesc().image_extent.height),
                 },
            };

            vkCmdSetScissor(command, 0, 1, &scissor);

            VkClearValue clear_value = {
                .color = { 0.0f, 0.0f, 0.0f, 1.0f },
            };

            const VkRenderingAttachmentInfoKHR color_attachment_info{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = swapchain_image_view->GetImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clear_value,
            };

            const VkRenderingInfoKHR render_info{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .renderArea = {
                        .offset = {
                            .x = 0,
                            .y = 0,
                        },
                        .extent = {
                            .width = swapchain_image_view->GetImageDesc().image_extent.width,
                            .height = swapchain_image_view->GetImageDesc().image_extent.height,
                        },
                    },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_attachment_info,
            };

            _vkCmdBeginRenderingKHR(command, &render_info);

            VkBuffer buffer = vertex_buffer->GetBuffer();
            VkDeviceSize offset = 0;

            vkCmdBindVertexBuffers(command, 0, 1, &buffer, &offset);
            vkCmdBindIndexBuffer(command, index_buffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(command, 6, 1, 0, 0, 0);

            _vkCmdEndRenderingKHR(command);
        });

        swapchain_image->TransitionImage(main_command, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        main_command.End();

        main_command.InsertWaitSemaphore(image_available_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
        main_command.InsertSignalSemaphore(render_finished_semaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

        main_command.Submit(&render_fence);

        context.GetSwapchain()->Present(swapchain_index, render_finished_semaphore);
    }

    // TODO: handle exit gracefully...
}
