#include "environment.h"


#include <iostream>
#include <set>


Environment::Environment(const Window& window, const char* applicationName, const uint32_t applicationVersion) :
    window(window),
    context(),
    instance(createInstance(applicationName, applicationVersion)),
    debugMessenger(createDebugMessenger()),
    surface(window.createSurface(instance)),
    physicalDevice(selectPhysicalDevice()),
    queueFamilyIndices(findQueueFamilies(physicalDevice)),
    device(createDevice()),
    graphicsQueue(device.getQueue(queueFamilyIndices.graphicsFamily.value(), 0)),
    presentQueue(device.getQueue(queueFamilyIndices.presentFamily.value(), 0)),
    graphicsCommandPool(createCommandPool(queueFamilyIndices.graphicsFamily.value())),
    swapchainSurfaceFormat(chooseSwapchainSurfaceFormat(querySwapchainSupport(physicalDevice).formats)),
    swapchainExtent(chooseSwapchainExtent(querySwapchainSupport(physicalDevice).capabilities)),
    swapchain(createSwapchain()),
    swapchainImages(swapchain.getImages()),
    swapchainImageViews(createSwapchainImageViews())
{
}

Environment::~Environment() = default;

std::vector<vk::raii::Framebuffer> Environment::createSwapchainFramebuffers(const vk::raii::RenderPass& renderPass) const
{
    std::vector<vk::raii::Framebuffer> framebuffers;
    framebuffers.reserve(swapchainImageViews.size());
    for (const vk::raii::ImageView& imageView : swapchainImageViews)
    {
        const vk::FramebufferCreateInfo createInfo{
            .renderPass = *renderPass,
            .attachmentCount = 1,
            .pAttachments = &*imageView,
            .width = swapchainExtent.width,
            .height = swapchainExtent.height,
            .layers = 1
        };

        try
        {
            framebuffers.emplace_back(device, createInfo);
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create Vulkan framebuffer.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
        }
    }

    return framebuffers;
}

std::vector<vk::raii::CommandBuffer> Environment::createGraphicsCommandBuffers(const uint32_t count, const vk::CommandBufferLevel level) const
{
    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *graphicsCommandPool,
        .level = level,
        .commandBufferCount = count
    };

    try
    {
        return device.allocateCommandBuffers(allocateInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to allocate command buffer.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::Semaphore Environment::createSemaphore(const vk::SemaphoreCreateFlags flags) const
{
    const vk::SemaphoreCreateInfo createInfo{
        .flags = flags
    };

    try
    {
        return device.createSemaphore(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan semaphore.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::Fence Environment::createFence(const vk::FenceCreateFlags flags) const
{
    const vk::FenceCreateInfo createInfo{
        .flags = flags
    };

    try {
        return device.createFence(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan fence.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::Buffer Environment::createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage) const
{
    const vk::BufferCreateInfo createInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    try
    {
        return device.createBuffer(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create buffer.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::DeviceMemory Environment::allocateBufferMemory(const vk::raii::Buffer& buffer,
    const vk::MemoryPropertyFlags properties) const
{
    const vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

    const vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    try
    {
        return device.allocateMemory(allocateInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to allocate buffer memory.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::Viewport Environment::getViewport() const
{
    return {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
}

vk::Rect2D Environment::getScissor() const
{
    return {
        .offset = { 0, 0 },
        .extent = swapchainExtent
    };
}

vk::Extent2D Environment::getSwapchainExtent() const
{
    return swapchainExtent;
}

const vk::raii::SwapchainKHR& Environment::getSwapchain() const
{
    return swapchain;
}

void Environment::recreateSwapchain()
{
    swapchainImageViews.clear();
    swapchainImages.clear();
    swapchain.clear();

    swapchainExtent = chooseSwapchainExtent(querySwapchainSupport(physicalDevice).capabilities);

    swapchain = createSwapchain();
    swapchainImages = swapchain.getImages();
    swapchainImageViews = createSwapchainImageViews();
}

vk::raii::Instance Environment::createInstance(const char* applicationName, const uint32_t applicationVersion) const
{
    void* pNext = nullptr;
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = getDebugUtilsMessengerCreateInfo();
    if constexpr (enabledDebug)
    {
        pNext = &debugUtilsMessengerCreateInfo;
    }

    vk::ApplicationInfo applicationInfo{
        .pApplicationName = applicationName,
        .applicationVersion = applicationVersion,
        .pEngineName = EngineName,
        .engineVersion = EngineVersion,
        .apiVersion = vk::ApiVersion13
    };

    std::vector<const char*> enabledLayerNames;
    if constexpr (enabledDebug)
    {
        for (const char* layer : validationLayers)
        {
            enabledLayerNames.push_back(layer);
        }
    }

    std::vector<const char*> enabledExtensions = getRequiredExtensionNames();

    const vk::InstanceCreateInfo createInfo{
        .pNext = pNext,
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size()),
        .ppEnabledLayerNames = enabledLayerNames.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
        .ppEnabledExtensionNames = enabledExtensions.data()
    };

    try
    {
        return context.createInstance(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan instance.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::DebugUtilsMessengerEXT Environment::createDebugMessenger() const
{
    if constexpr (!enabledDebug)
    {
        return nullptr;
    }

    try
    {
        return instance.createDebugUtilsMessengerEXT(getDebugUtilsMessengerCreateInfo());
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan debug utils messenger.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::PhysicalDevice Environment::selectPhysicalDevice() const
{
    for (const vk::raii::PhysicalDevice& physicalDevice : instance.enumeratePhysicalDevices())
    {
        if (isPhysicalDeviceSuitable(physicalDevice))
        {
            return physicalDevice;
        }
    }

    throw std::runtime_error("No suitable physical device found");
}

vk::raii::Device Environment::createDevice() const
{
    constexpr float queuePriority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for (const uint32_t queueFamilyIndex : queueFamilyIndices.getUniqueIndices())
    {
        queueCreateInfos.push_back({
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });
    }

    std::vector<const char*> enabledLayerNames;
    if constexpr (enabledDebug)
    {
        for (const char* layer : validationLayers)
        {
            enabledLayerNames.push_back(layer);
        }
    }

    constexpr vk::PhysicalDeviceFeatures enabledFeatures{};

    const vk::DeviceCreateInfo createInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size()),
        .ppEnabledLayerNames = enabledLayerNames.data(),
        .pEnabledFeatures = &enabledFeatures
    };

    try
    {
        return physicalDevice.createDevice(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan logical device.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::CommandPool Environment::createCommandPool(const uint32_t queueFamilyIndex) const
{
    const vk::CommandPoolCreateInfo createInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueFamilyIndex
    };

    try
    {
        return device.createCommandPool(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create command pool.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::SwapchainKHR Environment::createSwapchain() const
{
    const SwapchainDetails swapchainDetails = querySwapchainSupport(physicalDevice);
    const vk::PresentModeKHR presentMode = chooseSwapchainPresentMode(swapchainDetails.presentModes);

    uint32_t imageCount = swapchainDetails.capabilities.minImageCount + 1;
    if (swapchainDetails.capabilities.maxImageCount > 0 and imageCount > swapchainDetails.capabilities.maxImageCount)
    {
        imageCount = swapchainDetails.capabilities.maxImageCount;
    }

    const std::vector<uint32_t> uniqueQueueFamilyIndices = this->queueFamilyIndices.getUniqueIndices();

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = *surface,
        .minImageCount = imageCount,
        .imageFormat = swapchainSurfaceFormat.format,
        .imageColorSpace = swapchainSurfaceFormat.colorSpace,
        .imageExtent = swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = uniqueQueueFamilyIndices.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = static_cast<uint32_t>(uniqueQueueFamilyIndices.size()),
        .pQueueFamilyIndices = uniqueQueueFamilyIndices.data(),
        .preTransform = swapchainDetails.capabilities.currentTransform,
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
        throw std::runtime_error("Failed to create Vulkan swapchain.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

std::vector<vk::raii::ImageView> Environment::createSwapchainImageViews() const
{
    std::vector<vk::raii::ImageView> swapchainImageViews;
    for (const vk::Image& image : swapchainImages)
    {
        const vk::ImageViewCreateInfo createInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = swapchainSurfaceFormat.format,
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
            swapchainImageViews.emplace_back(device.createImageView(createInfo));
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create Vulkan image view.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
        }
    }

    return swapchainImageViews;
}

std::vector<const char*> Environment::getRequiredExtensionNames()
{
    std::vector<const char*> extensionNames;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        extensionNames.emplace_back(glfwExtensions[i]);
    }

    if constexpr (enabledDebug)
    {
        extensionNames.emplace_back(vk::EXTDebugUtilsExtensionName);
    }

    extensionNames.emplace_back(vk::KHRPortabilityEnumerationExtensionName);

    return extensionNames;
}

vk::DebugUtilsMessengerCreateInfoEXT Environment::getDebugUtilsMessengerCreateInfo()
{
    return vk::DebugUtilsMessengerCreateInfoEXT{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr
    };
}

bool Environment::isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice) const
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    const SwapchainDetails swapchainDetails = querySwapchainSupport(physicalDevice);
    const bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    return indices.isComplete() and
            swapchainDetails.isComplete() and
            extensionsSupported;
}

bool Environment::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const vk::ExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

Environment::QueueFamilyIndices Environment::findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice) const
{
    std::vector<uint32_t> graphicsFamilyIndices;
    std::vector<uint32_t> presentFamilyIndices;

    const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            graphicsFamilyIndices.push_back(i);
        }

        if (physicalDevice.getSurfaceSupportKHR(i, *surface))
        {
            presentFamilyIndices.push_back(i);
        }
    }

    for (uint32_t graphicsFamilyIndex : graphicsFamilyIndices)
    {
        for (uint32_t presentFamilyIndex : presentFamilyIndices)
        {
            if (graphicsFamilyIndex != presentFamilyIndex)
            {
                return QueueFamilyIndices{
                    .graphicsFamily = graphicsFamilyIndex,
                    .presentFamily = presentFamilyIndex
                };
            }
        }
    }

    return QueueFamilyIndices{
        .graphicsFamily = graphicsFamilyIndices.empty() ? std::nullopt : std::optional<uint32_t>(graphicsFamilyIndices[0]),
        .presentFamily = presentFamilyIndices.empty() ? std::nullopt : std::optional<uint32_t>(presentFamilyIndices[0])
    };
}

Environment::SwapchainDetails Environment::querySwapchainSupport(const vk::raii::PhysicalDevice& physicalDevice) const
{
    return SwapchainDetails{
        .capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface),
        .formats = physicalDevice.getSurfaceFormatsKHR(*surface),
        .presentModes = physicalDevice.getSurfacePresentModesKHR(*surface)
    };
}

vk::SurfaceFormatKHR Environment::chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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

vk::PresentModeKHR Environment::chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

vk::Extent2D Environment::chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    const auto [width, height] = window.getFramebufferSize();

    return {
        .width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        .height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

vk::Bool32 Environment::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "[Validation layer] " << pCallbackData->pMessage << std::endl;
    return vk::False;
}

uint32_t Environment::findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const
{
    const vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

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
