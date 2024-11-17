#include "swapchain_manager.h"


#include <iostream>


SwapchainManager::SwapchainManager(const Window& window, const PhysicalDeviceManager& physicalDeviceManager, const vk::raii::Device& device) :
    device(device),
    surface(window.surface),
    queueFamilyIndices(std::move(physicalDeviceManager.getQueueFamilyIndices())),
    capabilities(physicalDeviceManager.getSurfaceCapabilities(surface)),
    surfaceFormat(chooseSwapSurfaceFormat(physicalDeviceManager.getSurfaceFormats(surface))),
    presentMode(chooseSwapPresentMode(physicalDeviceManager.getSurfacePresentModes(surface))),
    extent(chooseSwapchainExtent(window.glfwWindow)),
    swapchain(createSwapchain()),
    images(createSwapchainImages()),
    imageViews(createImageViews())
{
}

SwapchainManager::~SwapchainManager() = default;

const vk::raii::SwapchainKHR& SwapchainManager::getSwapchain() const
{
    return swapchain;
}

const std::vector<vk::Image>& SwapchainManager::getImages() const
{
    return images;
}

const std::vector<vk::raii::ImageView>& SwapchainManager::getImageViews() const
{
    return imageViews;
}

void SwapchainManager::recreateSwapchain()
{
    swapchain.clear();
    images.clear();
    imageViews.clear();

    swapchain = createSwapchain();
    images = createSwapchainImages();
    imageViews = createImageViews();
}

vk::SurfaceFormatKHR SwapchainManager::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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

vk::PresentModeKHR SwapchainManager::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

vk::Extent2D SwapchainManager::chooseSwapchainExtent(const GLFWwindow* glfwWindow) const
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

vk::raii::SwapchainKHR SwapchainManager::createSwapchain() const
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

std::vector<vk::Image> SwapchainManager::createSwapchainImages() const
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

std::vector<vk::raii::ImageView> SwapchainManager::createImageViews() const
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
            imageViews.emplace_back(device.createImageView(createInfo));
            const vk::raii::ImageView imageView = device.createImageView(createInfo);
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create image view with error code: " + std::to_string(error.code().value()));
        }
    }

    return imageViews;
}
