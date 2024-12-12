#include "device_local_buffer.h"


DeviceLocalBuffer::DeviceLocalBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage) :
    environment(environment),
    size(size),
    buffer(createBuffer(size, usage)),
    bufferMemory(allocateBufferMemory(buffer, vk::MemoryPropertyFlagBits::eDeviceLocal))
{
    buffer.bindMemory(*bufferMemory, 0);
}

DeviceLocalBuffer::~DeviceLocalBuffer() = default;

void DeviceLocalBuffer::copyData(const void* srcData, const vk::DeviceSize dataSize) const
{
    if (dataSize > size)
    {
        throw std::runtime_error("Data size is greater than buffer size.");
    }

    const vk::raii::Buffer stagingBuffer = createBuffer(dataSize, vk::BufferUsageFlagBits::eTransferSrc);
    const vk::raii::DeviceMemory stagingBufferMemory = allocateBufferMemory(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.bindMemory(*stagingBufferMemory, 0);

    void *data = stagingBufferMemory.mapMemory(0, dataSize);
    std::memcpy(data, srcData, dataSize);
    stagingBufferMemory.unmapMemory();

    const vk::raii::CommandBuffer commandBuffer = std::move(environment.createGraphicsCommandBuffers(1)[0]);

    constexpr vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    commandBuffer.begin(beginInfo);

    const vk::BufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = dataSize
    };
    commandBuffer.copyBuffer(*stagingBuffer, *buffer, copyRegion);

    commandBuffer.end();

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    environment.graphicsQueue.submit(submitInfo, nullptr);
    environment.graphicsQueue.waitIdle();
}

vk::raii::Buffer DeviceLocalBuffer::createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage) const
{
    const vk::BufferCreateInfo createInfo{
        .size = size,
        .usage = usage | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    return environment.device.createBuffer(createInfo);
}

vk::raii::DeviceMemory DeviceLocalBuffer::allocateBufferMemory(const vk::raii::Buffer& buffer, const vk::MemoryPropertyFlags properties) const
{
    const vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

    const vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    return environment.device.allocateMemory(allocateInfo);
}

uint32_t DeviceLocalBuffer::findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const
{
    const vk::PhysicalDeviceMemoryProperties memoryProperties = environment.physicalDevice.getMemoryProperties();

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