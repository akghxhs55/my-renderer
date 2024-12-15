#ifndef DEVICE_LOCAL_BUFFER_H
#define DEVICE_LOCAL_BUFFER_H


#include "environment.h"
#include "i_buffer.h"


class DeviceLocalBuffer : public IBuffer {
private:
    std::reference_wrapper<const Environment> environment;
    vk::DeviceSize size;
    vk::raii::Buffer buffer;
    vk::raii::DeviceMemory bufferMemory;

public:
    DeviceLocalBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage);
    ~DeviceLocalBuffer() override;

    DeviceLocalBuffer(const DeviceLocalBuffer&) = delete;
    DeviceLocalBuffer& operator=(const DeviceLocalBuffer&) = delete;

    DeviceLocalBuffer(DeviceLocalBuffer&& other) noexcept;
    DeviceLocalBuffer& operator=(DeviceLocalBuffer&& other) noexcept;

    const vk::raii::Buffer& getBuffer() const override;
    void uploadData(const void* sourceData, const vk::DeviceSize dataSize) const override;

private:
    vk::raii::Buffer createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage) const;
    vk::raii::DeviceMemory allocateBufferMemory(const vk::raii::Buffer& buffer, const vk::MemoryPropertyFlags properties) const;

    uint32_t findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const;
};


#endif // DEVICE_LOCAL_BUFFER_H
