#pragma once

#include <memory>

#include <vk_mem_alloc.h>

#include "Device.h"
#include "Instance.h"

class Allocator {
public:
    Allocator(std::shared_ptr<Instance> instance, std::shared_ptr<Device> device);
    ~Allocator();

private:
    std::shared_ptr<Instance> instance_;
    std::shared_ptr<Device> device_;

    VmaAllocator allocator_; 

    void CreateAllocator();
};