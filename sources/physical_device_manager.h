#ifndef PHYSICAL_DEVICE_MANAGER_H
#define PHYSICAL_DEVICE_MANAGER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class PhysicalDeviceManager {
public:
    const vk::raii::PhysicalDevice physicalDevice;
    const std::optional<uint32_t> graphicsQueueFamilyIndex;
    const std::optional<uint32_t> presentQueueFamilyIndex;

    PhysicalDeviceManager(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions);
    ~PhysicalDeviceManager();

    std::vector<uint32_t> getQueueFamilyIndices() const;
    vk::SurfaceCapabilitiesKHR getSurfaceCapabilities(const vk::raii::SurfaceKHR& surface) const;
    std::vector<vk::SurfaceFormatKHR> getSurfaceFormats(const vk::raii::SurfaceKHR& surface) const;
    std::vector<vk::PresentModeKHR> getSurfacePresentModes(const vk::raii::SurfaceKHR& surface) const;

private:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;

        bool isComplete() const
        {
            return graphics.has_value();
        }
    };

    static vk::raii::PhysicalDevice createPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions);

    static bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface, const std::vector<const char*>& deviceExtensions);
    static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
    static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions);
};


#endif //PHYSICAL_DEVICE_MANAGER_H
