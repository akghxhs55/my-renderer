#ifndef DEVICE_CONTEXT_H
#define DEVICE_CONTEXT_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class DeviceContext {
public:
    const vk::raii::PhysicalDevice physicalDevice;
    const uint32_t graphicsQueueFamilyIndex;
    const uint32_t presentQueueFamilyIndex;
    const std::vector<uint32_t> queueFamilyIndices;
    const vk::raii::Device device;
    const vk::raii::Queue graphicsQueue;
    const vk::raii::Queue presentQueue;
    const vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    const std::vector<vk::SurfaceFormatKHR> surfaceFormats;
    const std::vector<vk::PresentModeKHR> presentModes;

    explicit DeviceContext(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions);
    ~DeviceContext();

private:
    static vk::raii::PhysicalDevice selectPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions);
    vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions) const;

    static bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions);
    static std::optional<uint32_t> findGraphicsQueueFamilyIndex(const vk::raii::PhysicalDevice& physicalDevice);
    static std::optional<uint32_t> findPresentQueueFamilyIndex(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
    static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions);

    static std::vector<uint32_t> getUniqueValues(const std::vector<uint32_t>& values);
};


#endif //DEVICE_CONTEXT_H
