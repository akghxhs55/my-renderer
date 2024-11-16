#ifndef SWAPCHAIN_MANAGER_H
#define SWAPCHAIN_MANAGER_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "physical_device_manager.h"


class SwapchainManager {
public:
    const vk::SurfaceCapabilitiesKHR capabilities;
    const vk::SurfaceFormatKHR surfaceFormat;
    const vk::PresentModeKHR presentMode;
    const vk::Extent2D extent;
    const vk::raii::SwapchainKHR swapchain;
    const std::vector<vk::Image> images;
    const std::vector<vk::raii::ImageView> imageViews;

    SwapchainManager(const Window& window, const PhysicalDeviceManager& physicalDeviceManager, const vk::raii::Device& device);
    ~SwapchainManager();

private:
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapchainExtent(const GLFWwindow* glfwWindow) const;
    vk::raii::SwapchainKHR createSwapchain(const vk::raii::SurfaceKHR& surface, const std::vector<uint32_t>& queueFamilyIndices,
                                           const vk::raii::Device& device) const;
    std::vector<vk::Image> createSwapchainImages() const;
    std::vector<vk::raii::ImageView> createImageViews(const vk::raii::Device& device) const;

};


#endif //SWAPCHAIN_MANAGER_H
