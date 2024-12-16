#include "device_local_image.h"


#include "host_visible_buffer.h"


DeviceLocalImage::DeviceLocalImage(const Environment& environment, const vk::Extent2D extent, const vk::Format format,
    const vk::ImageUsageFlags usage) :
    environment(environment),
    extent(extent),
    size(extent.width * extent.height * 4),
    image(createImage(extent, format, usage)),
    imageMemory(allocateImageMemory(vk::MemoryPropertyFlagBits::eDeviceLocal))
{
    image.bindMemory(*imageMemory, 0);
}

DeviceLocalImage::~DeviceLocalImage() = default;

DeviceLocalImage::DeviceLocalImage(DeviceLocalImage&& other) noexcept :
    environment(other.environment),
    extent(other.extent),
    size(other.size),
    image(std::move(other.image)),
    imageMemory(std::move(other.imageMemory))
{
}

DeviceLocalImage& DeviceLocalImage::operator=(DeviceLocalImage&& other) noexcept
{
    if (this != &other)
    {
        environment = other.environment;
        extent = other.extent;
        size = other.size;
        image = std::move(other.image);
        imageMemory = std::move(other.imageMemory);
    }

    return *this;
}

const vk::raii::Image& DeviceLocalImage::getImage() const
{
    return image;
}

void DeviceLocalImage::uploadData(const void* sourceData, const vk::DeviceSize dataSize) const
{
    if (dataSize > size)
    {
        throw std::runtime_error("Data size is greater than image size.");
    }

    const HostVisibleBuffer stagingBuffer(environment.get(), size, vk::BufferUsageFlagBits::eTransferSrc);
    stagingBuffer.uploadData(sourceData, dataSize);

    const vk::raii::CommandBuffer commandBuffer = environment.get().beginSingleTimeCommands();


}

vk::raii::Image DeviceLocalImage::createImage(const vk::Extent2D extent, const vk::Format format,
    const vk::ImageUsageFlags usage) const
{
    const vk::ImageCreateInfo createInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = vk::Extent3D{ extent.width, extent.height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = usage | vk::ImageUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    return environment.get().device.createImage(createInfo);
}

vk::raii::DeviceMemory DeviceLocalImage::allocateImageMemory(const vk::MemoryPropertyFlags properties) const
{
    const vk::MemoryRequirements memoryRequirements = image.getMemoryRequirements();

    const vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = environment.get().findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    return environment.get().device.allocateMemory(allocateInfo);
}
