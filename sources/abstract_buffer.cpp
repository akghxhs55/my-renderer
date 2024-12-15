#include "abstract_buffer.h"


AbstractBuffer::AbstractBuffer(const Environment& environment, const vk::DeviceSize size,
    const vk::BufferUsageFlags usage) :
    environment(environment),
    size(size),
    buffer(createBuffer(size, usage))
{
}

AbstractBuffer::~AbstractBuffer() = default;

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

vk::raii::DeviceMemory AbstractBuffer::allocateBufferMemory(const vk::raii::Buffer& buffer,
    const vk::MemoryPropertyFlags properties) const
{
    const vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

    const vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    return environment.get().device.allocateMemory(allocateInfo);
}

uint32_t AbstractBuffer::findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const
{
    const vk::PhysicalDeviceMemoryProperties memoryProperties = environment.get().physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) and
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type.");
}
