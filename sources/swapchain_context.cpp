#include "swapchain_context.h"


SwapchainContext::SwapchainContext(const DeviceContext& deviceContext, const Window& window, const vk::raii::SurfaceKHR& surface) :
    deviceContext(deviceContext),
    surface(surface),
    surfaceCapabilities(deviceContext.surfaceCapabilities),
    surfaceFormat(chooseSwapchainSurfaceFormat(deviceContext.surfaceFormats)),
    presentMode(chooseSwapchainPresentMode(deviceContext.presentModes)),
    extent(chooseSwapchainExtent(window)),
    swapchain(createSwapchain()),
    images(createSwapchainImages()),
    imageViews(createImageViews())
{
}

SwapchainContext::~SwapchainContext() = default;

const vk::raii::SwapchainKHR& SwapchainContext::getSwapchain() const
{
    return swapchain;
}

const std::vector<vk::Image>& SwapchainContext::getImages() const
{
    return images;
}

const std::vector<vk::raii::ImageView>& SwapchainContext::getImageViews() const
{
    return imageViews;
}

void SwapchainContext::recreateSwapchain()
{
    swapchain.clear();
    images.clear();
    imageViews.clear();

    swapchain = createSwapchain();
    images = createSwapchainImages();
    imageViews = createImageViews();
}

vk::SurfaceFormatKHR SwapchainContext::chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    for (const vk::SurfaceFormatKHR& availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb and
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR SwapchainContext::chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    for (const vk::PresentModeKHR& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapchainContext::chooseSwapchainExtent(const Window& window) const
{
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return surfaceCapabilities.currentExtent;
    }

    const auto [width, height] = window.getFramebufferSize();

    vk::Extent2D extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

    return extent;
}

vk::raii::SwapchainKHR SwapchainContext::createSwapchain() const
{
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 and
        surfaceCapabilities.maxImageCount < imageCount)
    {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = *surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = deviceContext.queueFamilyIndices.size() <= 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
        .queueFamilyIndexCount = static_cast<uint32_t>(deviceContext.queueFamilyIndices.size()),
        .pQueueFamilyIndices = deviceContext.queueFamilyIndices.data(),
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = vk::True,
        .oldSwapchain = nullptr
    };

    try
    {
        return deviceContext.device.createSwapchainKHR(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan swapchain with error code: " + std::to_string(error.code().value()));
    }
}

std::vector<vk::Image> SwapchainContext::createSwapchainImages() const
{
    try
    {
        return swapchain.getImages();
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to get swapchain images with error code: " + std::to_string(error.code().value()));
    }
}

std::vector<vk::raii::ImageView> SwapchainContext::createImageViews() const
{
    std::vector<vk::raii::ImageView> imageViews;
    imageViews.reserve(images.size());

    for (const auto& image : images)
    {
        const vk::ImageViewCreateInfo createInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = surfaceFormat.format,
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

        try
        {
            imageViews.emplace_back(deviceContext.device.createImageView(createInfo));
            const vk::raii::ImageView imageView = deviceContext.device.createImageView(createInfo);
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create image view with error code: " + std::to_string(error.code().value()));
        }
    }

    return imageViews;
}
