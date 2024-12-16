#include "host_visible_buffer.h"

HostVisibleBuffer::HostVisibleBuffer(const Environment& environment, const vk::DeviceSize size,
    const vk::BufferUsageFlags usage) :
    AbstractBuffer(environment, size, usage),
    bufferMemory(allocateBufferMemory(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)),
    mappedMemory(nullptr)
{
    buffer.bindMemory(*bufferMemory, 0);
    mappedMemory = bufferMemory.mapMemory(0, size);
}

HostVisibleBuffer::~HostVisibleBuffer()
{
    bufferMemory.unmapMemory();
}

HostVisibleBuffer::HostVisibleBuffer(HostVisibleBuffer&& other) noexcept :
    AbstractBuffer(std::move(other)),
    bufferMemory(std::move(other.bufferMemory)),
    mappedMemory(other.mappedMemory)
{
    other.mappedMemory = nullptr;
}

HostVisibleBuffer& HostVisibleBuffer::operator=(HostVisibleBuffer&& other) noexcept
{
    if (this != &other)
    {
        AbstractBuffer::operator=(std::move(other));
        bufferMemory = std::move(other.bufferMemory);

        other.mappedMemory = nullptr;
    }

    return *this;
}

void HostVisibleBuffer::uploadData(const void* sourceData, const vk::DeviceSize dataSize) const
{
    if (dataSize > size)
    {
        throw std::runtime_error("Data size is greater than buffer size.");
    }

    std::memcpy(mappedMemory, sourceData, dataSize);
}

