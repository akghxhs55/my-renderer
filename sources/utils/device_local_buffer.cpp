#include "device_local_buffer.h"


DeviceLocalBuffer::DeviceLocalBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage) :
    AbstractBuffer(environment, size, usage),
    bufferMemory(bindBufferMemory(buffer, vk::MemoryPropertyFlagBits::eDeviceLocal))
{
}

DeviceLocalBuffer::~DeviceLocalBuffer() = default;

DeviceLocalBuffer::DeviceLocalBuffer(DeviceLocalBuffer&& other) noexcept :
    AbstractBuffer(std::move(other)),
    bufferMemory(std::move(other.bufferMemory))
{
}

DeviceLocalBuffer& DeviceLocalBuffer::operator=(DeviceLocalBuffer&& other) noexcept
{
    if (this != &other)
    {
        AbstractBuffer::operator=(std::move(other));
        bufferMemory = std::move(other.bufferMemory);
    }

    return *this;
}

void DeviceLocalBuffer::uploadData(const void* sourceData, const vk::DeviceSize dataSize) const
{
    if (dataSize > size)
    {
        throw std::runtime_error("Data size is greater than buffer size.");
    }

    const vk::raii::Buffer stagingBuffer = createBuffer(dataSize, vk::BufferUsageFlagBits::eTransferSrc);
    const vk::raii::DeviceMemory stagingBufferMemory = bindBufferMemory(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *data = stagingBufferMemory.mapMemory(0, dataSize);
    std::memcpy(data, sourceData, dataSize);
    stagingBufferMemory.unmapMemory();

    const vk::raii::CommandBuffer commandBuffer = environment.get().beginSingleTimeCommands();

    const vk::BufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = dataSize
    };
    commandBuffer.copyBuffer(*stagingBuffer, *buffer, copyRegion);

    environment.get().submitSingleTimeCommands(commandBuffer);
}
