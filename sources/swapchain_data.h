#ifndef SWAPCHAIN_DATA_H
#define SWAPCHAIN_DATA_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <iostream>
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "physical_device_data.h"


class SwapchainData {
public:
    const vk::raii::SwapchainKHR swapchain;

    SwapchainData(const Window& window, const PhysicalDeviceData& physicalDeviceData, const vk::raii::Device& device);
    ~SwapchainData();

private:
    struct SwapchainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    vk::Format format;
    vk::Extent2D extent;
    const std::vector<vk::Image> images;
    const std::vector<vk::raii::ImageView> imageViews;

    vk::raii::SwapchainKHR createSwapchain(const GLFWwindow* glfwWindow, const vk::raii::SurfaceKHR& surface,
                                           const vk::raii::PhysicalDevice& physicalDevice, const std::vector<uint32_t>& queueFamilyIndices,
                                           const vk::raii::Device& device);
    std::vector<vk::Image> createSwapchainImages() const;
    std::vector<vk::raii::ImageView> createImageViews(const vk::raii::Device& device) const;

    static SwapchainSupportDetails querySwapChainSupport(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    static vk::Extent2D chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const GLFWwindow* glfwWindow);
};


#endif //SWAPCHAIN_DATA_H
