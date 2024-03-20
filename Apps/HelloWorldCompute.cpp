#include <iostream>

#include "GraphicsCore/Window.h"
#include "GraphicsCore/Command.h"
#include "GraphicsCore/Context.h"
#include "GraphicsCore/Resources.h"
#include "GraphicsCore/Pipeline.h"
#include "GraphicsCore/Shader.h"
#include "GraphicsCore/Synchronization.h"

int main() {
    Context context{ "Vulkan Rendergraph", 1600, 900 };
    Allocator allocator{context.GetInstance(), context.GetDevice()};
    DescriptorPool descriptor_pool{context.GetDevice()};

    CommandPool command_pool{context.GetDevice(), Device::QueueType::COMPUTE};
    CommandBuffer main_command = command_pool.AllocateSinglePrimaryCommandBuffer();

    ShaderCompiler compiler{context.GetDevice()};
    std::shared_ptr<Shader> compute_shader = compiler.LoadShader("HelloWorldCompute", "compute_main");
    ComputePipeline compute_pipeline{context.GetDevice(), compute_shader};
    
    const uint32_t num_elements = 100;
    Buffer::Desc buffer_desc = {
        .buffer_size = num_elements * sizeof(float),
        .buffer_usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    };

    ResourceDesc memory_desc = {
        .allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        .memory_usage = VMA_MEMORY_USAGE_AUTO,
    };
    
    std::shared_ptr<Buffer> buffer1 = allocator.AllocateBuffer(buffer_desc, memory_desc);
    std::shared_ptr<Buffer> buffer2 = allocator.AllocateBuffer(buffer_desc, memory_desc);
    std::shared_ptr<Buffer> output = allocator.AllocateBuffer(buffer_desc, memory_desc);

    float* data1 = reinterpret_cast<float*>(buffer1->MapToCPU());
    float* data2 = reinterpret_cast<float*>(buffer2->MapToCPU());

    for (uint32_t i = 0; i < num_elements; i++) {
        data1[i] = static_cast<float>(i);
        data2[i] = static_cast<float>(i);
    }

    buffer1->UnmapFromCPU();
    buffer2->UnmapFromCPU();

    Fence compute_fence{ context.GetDevice(), false };

    std::shared_ptr<DescriptorSetLayout> compute_layout = compute_shader->GetParameterLayouts().at(0);
    std::shared_ptr<DescriptorSet> descriptor_set = descriptor_pool.AllocateDescriptorSet(compute_layout);

    descriptor_set->WriteBufferDescriptor(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer1, 0, num_elements * sizeof(float));
    descriptor_set->WriteBufferDescriptor(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer2, 0, num_elements * sizeof(float));
    descriptor_set->WriteBufferDescriptor(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, output, 0, num_elements * sizeof(float));
    descriptor_set->Update();

    main_command.Begin(true);
    compute_pipeline.Bind(main_command);
    compute_pipeline.BindDescriptorSet(main_command, 0, descriptor_set);
    compute_pipeline.DispatchCompute(main_command, num_elements, 1, 1);
    main_command.End();

    main_command.Submit(&compute_fence);
    compute_fence.Wait(100000);

    float* output_data = reinterpret_cast<float*>(output->MapToCPU());

    std::cout << "Output: " << std::endl;
    for (uint32_t i = 0; i < num_elements; i++) {
        std::cout << i << ": " << output_data[i] << std::endl;;
    }
    std::cout << std::endl;

    output->UnmapFromCPU();
    
    // Deleting buffers before the allocator gets destroyed
    // TODO: this should be handled more gracefully
    buffer1 = nullptr;
    buffer2 = nullptr;
    output = nullptr;
}
