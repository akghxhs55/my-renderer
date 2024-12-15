#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "environment.h"
#include "render_pipeline.h"
#include "vertex.h"
#include "device_local_buffer.h"
#include "host_visible_buffer.h"


class MyRenderer {
private:
    struct SyncObjects {
        vk::raii::Semaphore imageAvailableSemaphore;
        vk::raii::Semaphore renderFinishedSemaphore;
        vk::raii::Fence inFlightFence;
    };
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
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

    static constexpr std::array<Vertex, 4> vertices = {
        Vertex{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
        Vertex{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        Vertex{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
        Vertex{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
    };
    static constexpr std::array<uint16_t, 6> indices = {
        0, 1, 2, 2, 3, 0
    };

    Window window;
    Environment environment;
    RenderPipeline renderPipeline;
    std::unique_ptr<AbstractBuffer> vertexBuffer;
    std::unique_ptr<AbstractBuffer> indexBuffer;
    std::vector<std::unique_ptr<AbstractBuffer>> uniformBuffers;
    std::vector<vk::raii::DescriptorSet> descriptorSets;
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers;
    std::vector<vk::raii::CommandBuffer> graphicsCommandBuffers;
    std::vector<SyncObjects> syncObjects;
    uint32_t currentFrame;

    void update() const;
    void drawFrame();

    void recordRenderCommand(const vk::CommandBuffer& commandBuffer, const uint32_t imageIndex) const;
    void recreateSwapchain();
    vk::raii::CommandBuffer beginSingleTimeCommands() const;
    void submitSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

    static std::vector<std::unique_ptr<AbstractBuffer>> createUniformBuffers(const Environment& environment, const uint32_t count);
    static std::vector<SyncObjects> createSyncObjects(const Environment& environment, const uint32_t count);
};


#endif //MY_RENDERER_H
