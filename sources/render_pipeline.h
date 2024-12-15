#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H


#include "environment.h"


class RenderPipeline {
public:
    explicit RenderPipeline(const Environment& environment, const vk::raii::DescriptorSetLayout& descriptorSetLayout);
    ~RenderPipeline();

private:
    const vk::raii::PipelineLayout pipelineLayout;
public:
    const vk::raii::RenderPass renderPass;
    const vk::raii::Pipeline pipeline;

private:
    static vk::raii::PipelineLayout createPipelineLayout(const Environment& environment, const vk::raii::DescriptorSetLayout& descriptorSetLayout);
    static vk::raii::RenderPass createRenderPass(const Environment& environment);
    vk::raii::Pipeline createGraphicsPipeline(const Environment& environment) const;

    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);
};


#endif //RENDER_PIPELINE_H
