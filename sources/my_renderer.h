#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"
#include "window.h"
#include "physical_device_data.h"
#include "swapchain_data.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run() const;

private:
    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    const std::vector<const char*> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    const Environment environment;
    const Window window;
    const PhysicalDeviceData physicalDeviceData;
    const vk::raii::Device device;
    const vk::raii::Queue graphicsQueue;
    const vk::raii::Queue presentQueue;
    const SwapchainData swapchainData;
    const vk::raii::PipelineLayout pipelineLayout;
    const vk::raii::RenderPass renderPass;
    const vk::raii::Pipeline graphicsPipeline;
    const std::vector<vk::raii::Framebuffer> swapchainFramebuffers;
    const vk::raii::CommandPool commandPool;
    const vk::raii::CommandBuffer commandBuffer;
    const vk::raii::Semaphore imageAvailableSemaphore;
    const vk::raii::Semaphore renderFinishedSemaphore;
    const vk::raii::Fence inFlightFence;

    vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice, const uint32_t& graphicsQueueFamilyIndex) const;

    static vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device& device);
    static vk::raii::RenderPass createRenderPass(const vk::raii::Device& device, const vk::Format& swapchainImageFormat);
    vk::raii::Pipeline createGraphicsPipeline(const vk::raii::Device& device) const;
    std::vector<vk::raii::Framebuffer> createFramebuffers(const vk::raii::Device& device) const;
    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, const uint32_t& queueFamilyIndex);
    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool);
    static vk::raii::Semaphore createSemaphore(const vk::raii::Device& device);
    static vk::raii::Fence createFence(const vk::raii::Device& device, const vk::FenceCreateFlags& flags);

    void drawFrame() const;

    static std::vector<char> readFile(const std::string& filename);
    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);
    static void recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, const uint32_t& imageIndex, const vk::raii::RenderPass& renderPass, const vk::raii::Pipeline& graphicsPipeline, const std::vector<vk::raii::Framebuffer>& swapchainFramebuffers, const vk::Extent2D& swapchainExtent);
};


#endif //MY_RENDERER_H
