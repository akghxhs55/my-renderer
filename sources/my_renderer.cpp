#include "my_renderer.h"

#include <iostream>
#include <set>


MyRenderer::MyRenderer() :
    window(initializeWindow()),
    context(),
    instance(initializeInstance()),
    debugMessenger(initializeDebugMessenger()),
    surface(initializeSurface()),
    physicalDevice(initializePhysicalDevice()),
    device(initializeDevice()),
    presentQueue(initializePresentQueue()),
    swapchain(initializeSwapchain()),
    swapchainImages(initializeSwapchainImages()),
    swapchainImageFormat(initializeSwapchainImageFormat()),
    swapchainExtent(initializeSwapchainExtent())
{
}

MyRenderer::~MyRenderer()
{
    glfwTerminate();
}

void MyRenderer::run()
{
    while (!glfwWindowShouldClose(window.get()))
    {
        glfwPollEvents();
        drawFrame();
    }
}

MyRenderer::WindowPtr MyRenderer::initializeWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(Width, Height, ApplicationName, nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Failed to create GLFW window.");
    }

    return WindowPtr(window, glfwDestroyWindow);
}

vk::raii::Instance MyRenderer::initializeInstance() const
{
    void* pNext = nullptr;

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = getDebugUtilsMessengerCreateInfo();
    if constexpr (enableValidationLayers)
    {
        pNext = &debugUtilsMessengerCreateInfo;
    }

    vk::ApplicationInfo applicationInfo{
        .pApplicationName = ApplicationName,
        .applicationVersion = ApplicationVersion,
        .pEngineName = EngineName,
        .engineVersion = EngineVersion,
        .apiVersion = vk::ApiVersion13
    };

    std::vector<const char*> instanceLayers;
    if (enableValidationLayers)
    {
        for (const char* layer : validationLayers)
        {
            instanceLayers.push_back(layer);
        }
    }

    std::vector<const char*> instanceExtensions = getRequiredExtensionNames();

    const vk::InstanceCreateInfo createInfo{
        .pNext = pNext,
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data()
    };

    try
    {
        return context.createInstance(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan instance with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::DebugUtilsMessengerEXT MyRenderer::initializeDebugMessenger() const
{
    if constexpr (!enableValidationLayers)
    {
        return nullptr;
    }

    try
    {
        return vk::raii::DebugUtilsMessengerEXT(instance, getDebugUtilsMessengerCreateInfo());
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan debug utils messenger with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::SurfaceKHR MyRenderer::initializeSurface() const
{
    VkSurfaceKHR _surface;
    if (const VkResult result = glfwCreateWindowSurface(static_cast<vk::Instance>(instance), window.get(), nullptr, &_surface);
        result == VK_SUCCESS)
    {
        return vk::raii::SurfaceKHR(instance, _surface);
    }
    else
    {
        throw std::runtime_error("Failed to create window surface with error code: " + std::to_string(result));
    }

}

vk::raii::PhysicalDevice MyRenderer::initializePhysicalDevice() const
{
    const std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    for (const vk::raii::PhysicalDevice& physicalDevice : physicalDevices)
    {
        if (isPhysicalDeviceSuitable(physicalDevice, surface))
        {
            return physicalDevice;
        }
    }

    throw std::runtime_error("Failed to find a suitable GPU.");
}

vk::raii::Device MyRenderer::initializeDevice() const
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    constexpr float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = indices.graphicsFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::PhysicalDeviceFeatures physicalDeviceFeatures{};

    const vk::DeviceCreateInfo createInfo{
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures
    };

    try
    {
        return physicalDevice.createDevice(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan device with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::Queue MyRenderer::initializePresentQueue() const
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    return device.getQueue(indices.presentFamily.value(), 0);
}

vk::raii::SwapchainKHR MyRenderer::initializeSwapchain() const
{
    const SwapChainSupportDetails swapchainSupportDetails = querySwapChainSupport(physicalDevice, surface);

    const vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupportDetails.formats);
    const vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupportDetails.presentModes);
    const vk::Extent2D extent = chooseSwapExtent(swapchainSupportDetails.capabilities, window.get());

    uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
    if (swapchainSupportDetails.capabilities.maxImageCount > 0 and
        swapchainSupportDetails.capabilities.maxImageCount < imageCount)
    {
        imageCount = swapchainSupportDetails.capabilities.maxImageCount;
    }

    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    std::vector<uint32_t> queueFamilyIndices;
    vk::SharingMode imageSharingMode = vk::SharingMode::eExclusive;
    if (indices.graphicsFamily.value() != indices.presentFamily.value())
    {
        imageSharingMode = vk::SharingMode::eConcurrent;
        queueFamilyIndices = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    }

    const vk::SwapchainCreateInfoKHR createInfo{
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = imageSharingMode,
        .queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
        .pQueueFamilyIndices = queueFamilyIndices.data(),
        .preTransform = swapchainSupportDetails.capabilities.currentTransform,
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

std::vector<vk::Image> MyRenderer::initializeSwapchainImages() const
{
    return swapchain.getImages();
}

vk::Format MyRenderer::initializeSwapchainImageFormat() const
{
    const SwapChainSupportDetails swapchainSupportDetails = querySwapChainSupport(physicalDevice, surface);
    return chooseSwapSurfaceFormat(swapchainSupportDetails.formats).format;
}

vk::Extent2D MyRenderer::initializeSwapchainExtent() const
{
    const SwapChainSupportDetails swapchainSupportDetails = querySwapChainSupport(physicalDevice, surface);
    return chooseSwapExtent(swapchainSupportDetails.capabilities, window.get());
}

void MyRenderer::drawFrame()
{
}

std::vector<const char*> MyRenderer::getRequiredExtensionNames()
{
    std::vector<const char*> extensionNames;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        extensionNames.emplace_back(glfwExtensions[i]);
    }

    if constexpr (enableValidationLayers)
    {
        extensionNames.emplace_back(vk::EXTDebugUtilsExtensionName);
    }

    extensionNames.emplace_back(vk::KHRPortabilityEnumerationExtensionName);

    return extensionNames;
}

vk::DebugUtilsMessengerCreateInfoEXT MyRenderer::getDebugUtilsMessengerCreateInfo()
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

vk::Bool32 MyRenderer::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

bool MyRenderer::isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface)
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    const bool extensionSupported = checkDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionSupported)
    {
        const SwapChainSupportDetails swapchainDetails = querySwapChainSupport(physicalDevice, surface);
        swapChainAdequate = !swapchainDetails.formats.empty() and
                            !swapchainDetails.presentModes.empty();
    }

    // const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    // const vk::PhysicalDeviceFeatures features = physicalDevice.getFeatures();

    return indices.isComplete() and
           extensionSupported and
           swapChainAdequate;
}

bool MyRenderer::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const vk::ExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

MyRenderer::QueueFamilyIndices MyRenderer::findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface)
{
    QueueFamilyIndices indices;

    const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }

        if (physicalDevice.getSurfaceSupportKHR(i, surface))
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    return indices;
}

MyRenderer::SwapChainSupportDetails MyRenderer::querySwapChainSupport(const vk::raii::PhysicalDevice& device,
    const vk::raii::SurfaceKHR& surface)
{
    return SwapChainSupportDetails{
        .capabilities = device.getSurfaceCapabilitiesKHR(surface),
        .formats = device.getSurfaceFormatsKHR(surface),
        .presentModes = device.getSurfacePresentModesKHR(surface)
    };
}

vk::SurfaceFormatKHR MyRenderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
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

vk::PresentModeKHR MyRenderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

vk::Extent2D MyRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    vk::Extent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}
