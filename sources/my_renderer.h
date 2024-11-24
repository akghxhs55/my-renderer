#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "environment.h"
#include "device_context.h"
#include "swapchain_context.h"
#include "render_pipeline.h"
#include "buffer_resource.h"
#include "command_manager.h"
#include "vertex.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run();

private:
    static constexpr auto WindowTitle = "My Renderer";
    static constexpr int WindowWidth = 800;
    static constexpr int WindowHeight = 600;

    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    static constexpr uint32_t MaxFramesInFlight = 2;

    const std::vector<const char*> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    static constexpr std::array<Vertex, 3> vertices = {
        Vertex{ { 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
        Vertex{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
        Vertex{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
    };

    Window window;
    Environment environment;
    vk::raii::SurfaceKHR surface;
    DeviceContext deviceContext;
    SwapchainContext swapchainContext;
    RenderPipeline renderPipeline;
    BufferResource vertexBufferResource;
    CommandManager commandManager;
    uint32_t currentFrameIndex;
    std::vector<vk::raii::Semaphore> imageAvailableSemaphore;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphore;
    std::vector<vk::raii::Fence> inFlightFence;

    static vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance, GLFWwindow* window);
    static std::vector<vk::raii::Semaphore> createSemaphores(const vk::raii::Device& device);
    static std::vector<vk::raii::Fence> createFences(const vk::raii::Device& device, const vk::FenceCreateFlags& flags);

    void drawFrame();

    void recreateSwapchain();
};


#endif //MY_RENDERER_H
