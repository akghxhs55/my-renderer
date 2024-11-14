#include "swapchain_data.h"


#include <iostream>


SwapchainData::SwapchainData(const Window& window, const PhysicalDeviceManager& physicalDeviceData, const vk::raii::Device& device) :
    capabilities(physicalDeviceData.getSurfaceCapabilities(window.surface)),
    surfaceFormat(chooseSwapSurfaceFormat(physicalDeviceData.getSurfaceFormats(window.surface))),
    presentMode(chooseSwapPresentMode(physicalDeviceData.getSurfacePresentModes(window.surface))),
    extent(chooseSwapchainExtent(capabilities, window.glfwWindow)),
    swapchain(createSwapchain(window.surface, physicalDeviceData.getQueueFamilyIndices(), device)),
    images(createSwapchainImages()),
    imageViews(createImageViews(device))
{
}

SwapchainData::~SwapchainData() = default;

vk::raii::SwapchainKHR SwapchainData::createSwapchain(const vk::raii::SurfaceKHR& surface,
                                                      const std::vector<uint32_t>& queueFamilyIndices,
                                                      const vk::raii::Device& device) const
{
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 and
        capabilities.maxImageCount < imageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = queueFamilyIndices.size() <= 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
        .queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
        .pQueueFamilyIndices = queueFamilyIndices.data(),
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = vk::True,
        .oldSwapchain = nullptr
    };

    try
    {
        return device.createSwapchainKHR(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan swapchain with error code: " + std::to_string(error.code().value()));
    }
}

std::vector<vk::Image> SwapchainData::createSwapchainImages() const
{
    return swapchain.getImages();
}

std::vector<vk::raii::ImageView> SwapchainData::createImageViews(const vk::raii::Device& device) const
{
    std::vector<vk::raii::ImageView> imageViews;
    for (const auto image : images)
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
            imageViews.emplace_back(device.createImageView(createInfo));
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create image view with error code: " + std::to_string(error.code().value()));
        }
    }

    return imageViews;
}

vk::SurfaceFormatKHR SwapchainData::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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

vk::PresentModeKHR SwapchainData::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

vk::Extent2D SwapchainData::chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
    const GLFWwindow* glfwWindow)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(const_cast<GLFWwindow*>(glfwWindow), &width, &height);

    vk::Extent2D extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}
