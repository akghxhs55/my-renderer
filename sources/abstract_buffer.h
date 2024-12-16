#ifndef ABSTRACT_BUFFER_H
#define ABSTRACT_BUFFER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"


class AbstractBuffer
{
protected:
    std::reference_wrapper<const Environment> environment;
    vk::DeviceSize size;
    vk::raii::Buffer buffer;

public:
    AbstractBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage);
    virtual ~AbstractBuffer();

    AbstractBuffer(const AbstractBuffer&) = delete;
    AbstractBuffer& operator=(const AbstractBuffer&) = delete;

    AbstractBuffer(AbstractBuffer&&) noexcept = default;
    AbstractBuffer& operator=(AbstractBuffer&&) noexcept = default;

    const vk::raii::Buffer& getBuffer() const;
    virtual void uploadData(const void* sourceData, const vk::DeviceSize dataSize) const = 0;

protected:
    vk::raii::Buffer createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage) const;
    vk::raii::DeviceMemory allocateBufferMemory(const vk::MemoryPropertyFlags properties) const;
};


#endif //ABSTRACT_BUFFER_H
