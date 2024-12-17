#include "device_local_image.h"


#include "host_visible_buffer.h"


DeviceLocalImage::DeviceLocalImage(const Environment& environment, const vk::Extent2D extent, const vk::Format format,
    const vk::ImageUsageFlags usage) :
    environment(environment),
    extent(extent),
    size(extent.width * extent.height * 4),
    currentLayout(vk::ImageLayout::eUndefined),
    image(createImage(extent, format, usage)),
    imageMemory(allocateImageMemory(vk::MemoryPropertyFlagBits::eDeviceLocal)),
    imageView(createImageView(format))
{

}

DeviceLocalImage::~DeviceLocalImage() = default;

DeviceLocalImage::DeviceLocalImage(DeviceLocalImage&& other) noexcept :
    environment(other.environment),
    extent(other.extent),
    size(other.size),
    currentLayout(other.currentLayout),
    image(std::move(other.image)),
    imageMemory(std::move(other.imageMemory)),
    imageView(std::move(other.imageView))
{
}

DeviceLocalImage& DeviceLocalImage::operator=(DeviceLocalImage&& other) noexcept
{
    if (this != &other)
    {
        environment = other.environment;
        extent = other.extent;
        size = other.size;
        currentLayout = other.currentLayout;
        image = std::move(other.image);
        imageMemory = std::move(other.imageMemory);
        imageView = std::move(other.imageView);
    }

    return *this;
}

const vk::raii::Image& DeviceLocalImage::getImage() const
{
    return image;
}

void DeviceLocalImage::uploadData(const void* sourceData, const vk::DeviceSize dataSize)
{
    if (dataSize > size)
    {
        throw std::runtime_error("Data size is greater than image size.");
    }

    const vk::ImageLayout previousLayout = currentLayout;
    transitionImageLayout(vk::ImageLayout::eTransferDstOptimal);

    const HostVisibleBuffer stagingBuffer(environment.get(), size, vk::BufferUsageFlagBits::eTransferSrc);
    stagingBuffer.uploadData(sourceData, dataSize);

    const vk::BufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = vk::ImageSubresourceLayers{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = vk::Offset3D{ 0, 0, 0 },
        .imageExtent = vk::Extent3D{ extent.width, extent.height, 1 }
    };

    const vk::raii::CommandBuffer commandBuffer = environment.get().beginSingleTimeCommands();
    commandBuffer.copyBufferToImage(*stagingBuffer.getBuffer(), *image, vk::ImageLayout::eTransferDstOptimal, region);
    environment.get().submitSingleTimeCommands(commandBuffer);

    transitionImageLayout(previousLayout);
}

void DeviceLocalImage::transitionImageLayout(const vk::ImageLayout newLayout)
{
    if (newLayout == vk::ImageLayout::eUndefined or newLayout == vk::ImageLayout::ePreinitialized)
    {
        return;
    }

    vk::AccessFlags srcAccessMask;
    vk::AccessFlags dstAccessMask;
    vk::PipelineStageFlags srcStageMask;
    vk::PipelineStageFlags dstStageMask;

    if (currentLayout == vk::ImageLayout::eUndefined and newLayout == vk::ImageLayout::eTransferDstOptimal)
    {

        srcAccessMask = {};
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageMask = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (currentLayout == vk::ImageLayout::eUndefined and newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        srcAccessMask = {};
        dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (currentLayout == vk::ImageLayout::eTransferDstOptimal and newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageMask = vk::PipelineStageFlagBits::eTransfer;
        dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition.");
    }

    const vk::ImageMemoryBarrier barrier{
        .oldLayout = currentLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = *image,
        .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount =  1
        },
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask
    };

    const vk::raii::CommandBuffer commandBuffer = environment.get().beginSingleTimeCommands();
    commandBuffer.pipelineBarrier(srcStageMask, dstStageMask, {}, nullptr, nullptr, barrier);
    environment.get().submitSingleTimeCommands(commandBuffer);

    currentLayout = newLayout;
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

    vk::raii::DeviceMemory imageMemory = environment.get().device.allocateMemory(allocateInfo);
    image.bindMemory(*imageMemory, 0);

    return imageMemory;
}

vk::raii::ImageView DeviceLocalImage::createImageView(const vk::Format format) const
{
    const vk::ImageViewCreateInfo createInfo{
        .image = *image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components = {
            .r = vk::ComponentSwizzle::eIdentity,
            .g = vk::ComponentSwizzle::eIdentity,
            .b = vk::ComponentSwizzle::eIdentity,
            .a = vk::ComponentSwizzle::eIdentity
        },
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    return environment.get().device.createImageView(createInfo);
}
