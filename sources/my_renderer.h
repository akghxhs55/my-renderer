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
    void run() const;

private:
    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);

    const Environment environment;
    const Window window;
    const PhysicalDeviceManager physicalDeviceManager;
    const DeviceManager deviceManager;
    const SwapchainManager swapchainManager;
    const RenderPipeline renderPipeline;
    const CommandBufferManager commandBufferManager;
    const vk::raii::Semaphore imageAvailableSemaphore;
    const vk::raii::Semaphore renderFinishedSemaphore;
    const vk::raii::Fence inFlightFence;

    static vk::raii::Semaphore createSemaphore(const vk::raii::Device& device);
    static vk::raii::Fence createFence(const vk::raii::Device& device, const vk::FenceCreateFlags& flags);

    void drawFrame() const;
};


#endif //MY_RENDERER_H
