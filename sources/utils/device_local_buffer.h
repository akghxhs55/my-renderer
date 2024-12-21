#ifndef DEVICE_LOCAL_BUFFER_H
#define DEVICE_LOCAL_BUFFER_H


#include "abstract_buffer.h"


class DeviceLocalBuffer : public AbstractBuffer {
private:
    vk::raii::DeviceMemory bufferMemory;

public:
    DeviceLocalBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage);
    ~DeviceLocalBuffer() override;

    DeviceLocalBuffer(const DeviceLocalBuffer&) = delete;
    DeviceLocalBuffer& operator=(const DeviceLocalBuffer&) = delete;

    DeviceLocalBuffer(DeviceLocalBuffer&& other) noexcept;
    DeviceLocalBuffer& operator=(DeviceLocalBuffer&& other) noexcept;

    void uploadData(const void* sourceData, const vk::DeviceSize dataSize) const override;
};


#endif // DEVICE_LOCAL_BUFFER_H
