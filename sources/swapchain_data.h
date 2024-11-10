#ifndef SWAPCHAIN_DATA_H
#define SWAPCHAIN_DATA_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "physical_device_data.h"


class SwapchainData {
public:
    const vk::SurfaceCapabilitiesKHR capabilities;
    const vk::SurfaceFormatKHR surfaceFormat;
    const vk::PresentModeKHR presentMode;
    const vk::Extent2D extent;
    const vk::raii::SwapchainKHR swapchain;
    const std::vector<vk::Image> images;
    const std::vector<vk::raii::ImageView> imageViews;

    SwapchainData(const Window& window, const PhysicalDeviceData& physicalDeviceData, const vk::raii::Device& device);
    ~SwapchainData();

private:
    vk::raii::SwapchainKHR createSwapchain(const vk::raii::SurfaceKHR& surface, const std::vector<uint32_t>& queueFamilyIndices,
                                           const vk::raii::Device& device) const;
    std::vector<vk::Image> createSwapchainImages() const;
    std::vector<vk::raii::ImageView> createImageViews(const vk::raii::Device& device) const;

    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    static vk::Extent2D chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const GLFWwindow* glfwWindow);
};


#endif //SWAPCHAIN_DATA_H
