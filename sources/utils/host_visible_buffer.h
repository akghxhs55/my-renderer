#ifndef HOST_VISIBLE_BUFFER_H
#define HOST_VISIBLE_BUFFER_H


#include "abstract_buffer.h"


class HostVisibleBuffer : public AbstractBuffer {
private:
    vk::raii::DeviceMemory bufferMemory;
    void* mappedMemory;

public:
    HostVisibleBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage);
    ~HostVisibleBuffer() override;

    HostVisibleBuffer(const HostVisibleBuffer&) = delete;
    HostVisibleBuffer& operator=(const HostVisibleBuffer&) = delete;

    HostVisibleBuffer(HostVisibleBuffer&& other) noexcept;
    HostVisibleBuffer& operator=(HostVisibleBuffer&& other) noexcept;

    void uploadData(const void* sourceData, const vk::DeviceSize dataSize) const override;
};


#endif //HOST_VISIBLE_BUFFER_H
