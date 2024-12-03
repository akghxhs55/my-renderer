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

    void run();

private:
    static constexpr auto WindowTitle = "My Renderer";
    static constexpr int WindowWidth = 800;
    static constexpr int WindowHeight = 600;

    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);

    const std::vector<const char*> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    Window window;
    Environment environment;
    const vk::raii::PipelineLayout pipelineLayout;
    const vk::raii::RenderPass renderPass;
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers;
    const vk::raii::Pipeline graphicsPipeline;
    const vk::raii::CommandPool commandPool;
    const vk::raii::CommandBuffer commandBuffer;

    void drawFrame();

    void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, const uint32_t imageIndex) const;

    static vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device& device);
    static vk::raii::RenderPass createRenderPass(const vk::raii::Device& device, const vk::Format& swapchainImageFormat);
    static vk::raii::Pipeline createGraphicsPipeline(const Environment& environment, const vk::raii::PipelineLayout& pipelineLayout,
        const vk::raii::RenderPass& renderPass);
    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, const uint32_t queueFamilyIndex);
    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool);

    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);

    static std::vector<char> readFile(const std::string& filename);
};


#endif //MY_RENDERER_H
