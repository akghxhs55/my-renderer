#ifndef SWAPCHAIN_MANAGER_H
#define SWAPCHAIN_MANAGER_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "physical_device_manager.h"


class SwapchainManager {
private:
    const vk::raii::Device& device;
    const vk::raii::SurfaceKHR& surface;
    const std::vector<uint32_t>& queueFamilyIndices;
public:
    const vk::SurfaceCapabilitiesKHR capabilities;
    const vk::SurfaceFormatKHR surfaceFormat;
    const vk::PresentModeKHR presentMode;
    const vk::Extent2D extent;
private:
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;

public:
    SwapchainManager(const Window& window, const PhysicalDeviceManager& physicalDeviceManager, const vk::raii::Device& device);
    ~SwapchainManager();

    const vk::raii::SwapchainKHR& getSwapchain() const;
    const std::vector<vk::Image>& getImages() const;
    const std::vector<vk::raii::ImageView>& getImageViews() const;

    void recreateSwapchain();

private:
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapchainExtent(const GLFWwindow* glfwWindow) const;
    vk::raii::SwapchainKHR createSwapchain() const;
    std::vector<vk::Image> createSwapchainImages() const;
    std::vector<vk::raii::ImageView> createImageViews() const;

};


#endif //SWAPCHAIN_MANAGER_H
