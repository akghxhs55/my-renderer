#ifndef SWAPCHAIN_CONTEXT_H
#define SWAPCHAIN_CONTEXT_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "device_context.h"


class SwapchainContext {
private:
    const DeviceContext& deviceContext;
    const vk::raii::SurfaceKHR& surface;
    const vk::SurfaceCapabilitiesKHR surfaceCapabilities;
public:
    const vk::SurfaceFormatKHR surfaceFormat;
    const vk::PresentModeKHR presentMode;
    const vk::Extent2D extent;
private:
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;

public:
    SwapchainContext(const DeviceContext& deviceContext, const Window& window, const vk::raii::SurfaceKHR& surface);
    ~SwapchainContext();

    const vk::raii::SwapchainKHR& getSwapchain() const;
    const std::vector<vk::Image>& getImages() const;
    const std::vector<vk::raii::ImageView>& getImageViews() const;

    void recreateSwapchain();

private:
    static vk::SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapchainExtent(const Window& window) const;
    vk::raii::SwapchainKHR createSwapchain() const;
    std::vector<vk::Image> createSwapchainImages() const;
    std::vector<vk::raii::ImageView> createImageViews() const;

};


#endif //SWAPCHAIN_CONTEXT_H
