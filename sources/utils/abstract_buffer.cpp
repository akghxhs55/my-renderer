#include "abstract_buffer.h"


AbstractBuffer::AbstractBuffer(const Environment& environment, const vk::DeviceSize size,
                               const vk::BufferUsageFlags usage) :
    environment(environment),
    size(size),
    buffer(createBuffer(size, usage))
{
}

AbstractBuffer::AbstractBuffer(AbstractBuffer&& other) noexcept :
    environment(other.environment),
    size(other.size),
    buffer(std::move(other.buffer))
{
}

AbstractBuffer& AbstractBuffer::operator=(AbstractBuffer&& other) noexcept
{
    if (this != &other)
    {
        environment = other.environment;
        size = other.size;
        buffer = std::move(other.buffer);
    }

    return *this;
}

const vk::raii::Buffer& AbstractBuffer::getBuffer() const
{
    return buffer;
}

vk::raii::Buffer AbstractBuffer::createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage) const
{
    const vk::BufferCreateInfo createInfo{
        .size = size,
        .usage = usage | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    return environment.get().device.createBuffer(createInfo);
}

vk::raii::DeviceMemory AbstractBuffer::bindBufferMemory(const vk::raii::Buffer& buffer, const vk::MemoryPropertyFlags properties) const
{
    const vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

    const vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = environment.get().findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    vk::raii::DeviceMemory bufferMemory = environment.get().device.allocateMemory(allocateInfo);
    buffer.bindMemory(*bufferMemory, 0);

    return bufferMemory;
}
