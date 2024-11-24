#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "swapchain_context.h"


class RenderPipeline {
private:
    const vk::raii::Device& device;
    const SwapchainContext& swapchainContext;
    const vk::raii::PipelineLayout pipelineLayout;
public:
    const vk::raii::RenderPass renderPass;
    const vk::raii::Pipeline pipeline;
private:
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers;

public:
    RenderPipeline(const vk::raii::Device& device, const SwapchainContext& swapchainManager);
    ~RenderPipeline();

    const vk::raii::Framebuffer& getSwapchainFramebuffer(const uint32_t& index) const;
    const std::vector<vk::raii::Framebuffer>& getSwapchainFramebuffers() const;

    void clearSwapchainFramebuffers();
    void recreateSwapchainFramebuffers();

private:
    vk::raii::PipelineLayout createPipelineLayout() const;
    vk::raii::RenderPass createRenderPass() const;
    vk::raii::Pipeline createPipeline() const;
    std::vector<vk::raii::Framebuffer> createSwapchainFramebuffers() const;

    static std::vector<char> readFile(const std::string& filename);
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
};


#endif //RENDER_PIPELINE_H
