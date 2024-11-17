#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"
#include "window.h"
#include "physical_device_manager.h"
#include "device_manager.h"
#include "swapchain_manager.h"
#include "render_pipeline.h"
#include "command_buffer_manager.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run();

private:
    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    static constexpr uint32_t MaxFramesInFlight = 2;

    Environment environment;
    Window window;
    PhysicalDeviceManager physicalDeviceManager;
    DeviceManager deviceManager;
    SwapchainManager swapchainManager;
    RenderPipeline renderPipeline;
    CommandBufferManager commandBufferManager;
    uint32_t currentFrame;
    std::vector<vk::raii::Semaphore> imageAvailableSemaphore;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphore;
    std::vector<vk::raii::Fence> inFlightFence;

    static std::vector<vk::raii::Semaphore> createSemaphores(const vk::raii::Device& device);
    static std::vector<vk::raii::Fence> createFences(const vk::raii::Device& device, const vk::FenceCreateFlags& flags);

    void drawFrame();

    void recreateSwapchain();
};


#endif //MY_RENDERER_H
