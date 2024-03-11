#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "Device.h"

class Fence {
public:
    Fence(std::shared_ptr<Device> device, bool start_signaled = true);
    ~Fence();

    void Wait(uint64_t ns) const;
    void Reset();

    const VkFence& GetFence() const {return fence_;}

private:
    std::shared_ptr<Device> device_;

    VkFence fence_;

    void CreateFence(bool start_signaled);
};

class Semaphore {
public:
    Semaphore(std::shared_ptr<Device> device);
    ~Semaphore();

    const VkSemaphore& GetSemaphore() const {return semaphore_;}

private:
    std::shared_ptr<Device> device_;
    
    VkSemaphore semaphore_;

    void CreateSemaphore();
};