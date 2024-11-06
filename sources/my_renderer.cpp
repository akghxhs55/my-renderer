#include "my_renderer.h"

#include <iostream>


MyShader::MyShader() :
    window(initializeWindow()),
    context(),
    instance(initializeInstance()),
    debugMessenger(initializeDebugMessenger())
{
}

MyShader::~MyShader()
{
    glfwTerminate();
}

void MyShader::run()
{
    while (!glfwWindowShouldClose(window.get()))
    {
        glfwPollEvents();
        drawFrame();
    }
}

MyShader::WindowPtr MyShader::initializeWindow()
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

vk::raii::Instance MyShader::initializeInstance() const
{
    void* pNext = nullptr;
    if constexpr (enableValidationLayers)
    {
        const vk::DebugUtilsMessengerCreateInfoEXT createInfo{
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr
        };

        pNext = const_cast<vk::DebugUtilsMessengerCreateInfoEXT*>(&createInfo);
    }

    vk::ApplicationInfo applicationInfo{
        .pApplicationName = ApplicationName,
        .applicationVersion = ApplicationVersion,
        .pEngineName = "No Engine",
        .engineVersion = vk::makeApiVersion(0, 1, 0, 0),
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
        return vk::raii::Instance(context, createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan instance with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::DebugUtilsMessengerEXT MyShader::initializeDebugMessenger() const
{
    if constexpr (!enableValidationLayers)
    {
        return nullptr;
    }

    const vk::DebugUtilsMessengerCreateInfoEXT createInfo{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr
    };

    try
    {
        return vk::raii::DebugUtilsMessengerEXT(instance, createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan debug utils messenger with error code: " + std::to_string(error.code().value()));
    }
}

void MyShader::drawFrame()
{

}

std::vector<const char*> MyShader::getRequiredExtensionNames()
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

vk::Bool32 MyShader::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
