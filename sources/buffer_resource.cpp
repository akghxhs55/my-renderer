#include "buffer_resource.h"


BufferResource::BufferResource(const DeviceContext& deviceContext, const vk::DeviceSize& size, const vk::BufferUsageFlags& usage,
    const vk::MemoryPropertyFlags& properties) :
    deviceContext(deviceContext),
    buffer(createBuffer(size, usage, vk::SharingMode::eExclusive)),
    memory(createDeviceMemory(buffer, properties))
{
    buffer.bindMemory(*memory, 0);
}

BufferResource::~BufferResource() = default;

void BufferResource::copyData(const void* data, const vk::DeviceSize& size) const
{
    void* mappedMemory = memory.mapMemory(0, size);
    std::memcpy(mappedMemory, data, size);
    memory.unmapMemory();
}

vk::raii::Buffer BufferResource::createBuffer(const vk::DeviceSize& size, const vk::BufferUsageFlags& usage,
                                              const vk::SharingMode& sharingMode) const
{
    const vk::BufferCreateInfo createInfo{
        .size = size,
        .usage = usage,
        .sharingMode = sharingMode
    };

    try
    {
        return deviceContext.device.createBuffer(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create buffer with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::DeviceMemory BufferResource::createDeviceMemory(const vk::raii::Buffer& buffer, const vk::MemoryPropertyFlags& properties) const
{
    const vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

    const vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    try
    {
        return deviceContext.device.allocateMemory(allocateInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to allocate memory with error code: " + std::to_string(error.code().value()));
    }
}

uint32_t BufferResource::findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const
{
    const vk::PhysicalDeviceMemoryProperties memoryProperties = deviceContext.physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) and
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}
