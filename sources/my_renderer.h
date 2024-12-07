#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"
#include "environment.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();

    void run() const;

private:
    static constexpr auto WindowTitle = "My Renderer";
    static constexpr int WindowWidth = 800;
    static constexpr int WindowHeight = 600;

    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);

    Window window;
    Environment environment;
    const vk::raii::PipelineLayout pipelineLayout;
    const vk::raii::RenderPass renderPass;
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers;
    const vk::raii::Pipeline graphicsPipeline;
    const vk::raii::CommandBuffer graphicsCommandBuffer;
    const vk::raii::Semaphore imageAvailableSemaphore;
    const vk::raii::Semaphore renderFinishedSemaphore;
    const vk::raii::Fence inFlightFence;

    void drawFrame() const;

    void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, const uint32_t imageIndex) const;

    static vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device& device);
    static vk::raii::RenderPass createRenderPass(const vk::raii::Device& device, const vk::Format& swapchainImageFormat);
    static vk::raii::Pipeline createGraphicsPipeline(const Environment& environment, const vk::raii::PipelineLayout& pipelineLayout,
        const vk::raii::RenderPass& renderPass);
    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);

    static std::vector<char> readFile(const std::string& filename);
};


#endif //MY_RENDERER_H
