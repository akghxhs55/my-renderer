#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "environment.h"
#include "render_pipeline.h"


class MyRenderer {
private:
    struct SyncObjects {
        vk::raii::Semaphore imageAvailableSemaphore;
        vk::raii::Semaphore renderFinishedSemaphore;
        vk::raii::Fence inFlightFence;
    };

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

    Window window;
    Environment environment;
    RenderPipeline renderPipeline;
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers;
    const std::vector<vk::raii::CommandBuffer> graphicsCommandBuffers;
    const std::vector<SyncObjects> syncObjects;
    uint32_t currentFrame = 0;

    void drawFrame();

    void recordRenderCommand(const vk::CommandBuffer& commandBuffer, const uint32_t imageIndex) const;

    static std::vector<SyncObjects> createSyncObjects(const uint32_t count, const Environment& environment);
};


#endif //MY_RENDERER_H
