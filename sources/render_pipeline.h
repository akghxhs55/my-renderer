#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "swapchain_manager.h"


class RenderPipeline {
public:
    const vk::raii::PipelineLayout pipelineLayout;
    const vk::raii::RenderPass renderPass;
    const vk::raii::Pipeline pipeline;
private:
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers;

public:
    RenderPipeline(const vk::raii::Device& device, const SwapchainManager& swapchainData);
    ~RenderPipeline();

    const std::vector<vk::raii::Framebuffer>& getSwapchainFramebuffers() const;

    void clearSwapchainFramebuffers();
    void resetSwapchainFramebuffers(const vk::raii::Device& device, const SwapchainManager& swapchainManager);

private:

    static vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device& device);
    static vk::raii::RenderPass createRenderPass(const vk::raii::Device& device, const vk::Format& swapchainImageFormat);
    vk::raii::Pipeline createPipeline(const vk::raii::Device& device, const vk::Extent2D& extent) const;
    std::vector<vk::raii::Framebuffer> createSwapchainFramebuffers(const vk::raii::Device& device, const SwapchainManager& swapchainManager) const;

    static std::vector<char> readFile(const std::string& filename);
    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);
};


#endif //RENDER_PIPELINE_H
