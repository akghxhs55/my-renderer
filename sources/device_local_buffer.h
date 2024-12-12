#ifndef DEVICE_LOCAL_BUFFER_H
#define DEVICE_LOCAL_BUFFER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


#include "environment.h"


class DeviceLocalBuffer {
private:
    const Environment& environment;
    const vk::DeviceSize size;
public:
    const vk::raii::Buffer buffer;
private:
    const vk::raii::DeviceMemory bufferMemory;

public:
    DeviceLocalBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage);
    ~DeviceLocalBuffer();

    void copyData(const void* srcData, const vk::DeviceSize dataSize) const;

private:
    vk::raii::Buffer createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage) const;
    vk::raii::DeviceMemory allocateBufferMemory(const vk::raii::Buffer& buffer, const vk::MemoryPropertyFlags properties) const;

    uint32_t findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const;
};


#endif // DEVICE_LOCAL_BUFFER_H
