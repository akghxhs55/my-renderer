#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H


#include "environment.h"


class RenderPipeline {
public:
    explicit RenderPipeline(const Environment& environment);
    ~RenderPipeline();

private:
    const Environment& environment;
    const vk::raii::PipelineLayout pipelineLayout;
public:
    const vk::raii::RenderPass renderPass;
    const vk::raii::Pipeline pipeline;

private:
    vk::raii::PipelineLayout createPipelineLayout() const;
    vk::raii::RenderPass createRenderPass() const;
    vk::raii::Pipeline createGraphicsPipeline() const;

    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);
};


#endif //RENDER_PIPELINE_H
