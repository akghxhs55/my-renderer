#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "swapchain_data.h"


class RenderPipeline {
public:
    const vk::raii::PipelineLayout pipelineLayout;
    const vk::raii::RenderPass renderPass;
    const vk::raii::Pipeline pipeline;
    const std::vector<vk::raii::Framebuffer> swapchainFramebuffers;

    RenderPipeline(const vk::raii::Device& device, const SwapchainData& swapchainData);
    ~RenderPipeline();

private:
    static vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device& device);
    static vk::raii::RenderPass createRenderPass(const vk::raii::Device& device, const vk::Format& swapchainImageFormat);
    vk::raii::Pipeline createPipeline(const vk::raii::Device& device, const vk::Extent2D& extent) const;
    std::vector<vk::raii::Framebuffer> createFramebuffers(const vk::raii::Device& device, const std::vector<vk::raii::ImageView>& imageViews, const vk::Extent2D& extent) const;

    static std::vector<char> readFile(const std::string& filename);
    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);
};


#endif //RENDER_PIPELINE_H
