#ifndef BUFFER_RESOURCE_H
#define BUFFER_RESOURCE_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "device_context.h"


class BufferResource {
private:
    const DeviceContext& deviceContext;
public:
    const vk::raii::Buffer buffer;
private:
    const vk::raii::DeviceMemory memory;

public:
    BufferResource(const DeviceContext& deviceContext, const vk::DeviceSize& size, const vk::BufferUsageFlags& usage, const vk::MemoryPropertyFlags& properties);
    ~BufferResource();

    void copyData(const void* data, const vk::DeviceSize& size) const;

private:
    vk::raii::Buffer createBuffer(const vk::DeviceSize& size, const vk::BufferUsageFlags& usage, const vk::SharingMode& sharingMode) const;
    vk::raii::DeviceMemory createDeviceMemory(const vk::raii::Buffer& buffer, const vk::MemoryPropertyFlags& properties) const;

    uint32_t findMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const;
};


#endif //BUFFER_RESOURCE_H
