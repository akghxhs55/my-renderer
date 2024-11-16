#include "render_pipeline.h"


#include <fstream>


RenderPipeline::RenderPipeline(const vk::raii::Device& device, const SwapchainManager& swapchainData) :
    pipelineLayout(createPipelineLayout(device)),
    renderPass(createRenderPass(device, swapchainData.surfaceFormat.format)),
    pipeline(createPipeline(device, swapchainData.extent)),
    swapchainFramebuffers(createFramebuffers(device, swapchainData))
{
}

RenderPipeline::~RenderPipeline() = default;

vk::raii::PipelineLayout RenderPipeline::createPipelineLayout(const vk::raii::Device& device)
{
    constexpr vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    try
    {
        return device.createPipelineLayout(pipelineLayoutCreateInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create pipeline layout with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::RenderPass RenderPipeline::createRenderPass(const vk::raii::Device& device,
    const vk::Format& swapchainImageFormat)
{
    const vk::AttachmentDescription colorAttachmentDescription{
        .format = swapchainImageFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    constexpr vk::AttachmentReference colorAttachmentReference{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    const vk::SubpassDescription subpassDescription{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference
    };

    constexpr vk::SubpassDependency subpassDependency{
        .srcSubpass = vk::SubpassExternal,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        .dependencyFlags = vk::DependencyFlagBits::eByRegion
    };

    const vk::RenderPassCreateInfo createInfo{
        .attachmentCount = 1,
        .pAttachments = &colorAttachmentDescription,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    try
    {
        return device.createRenderPass(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create render pass with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::Pipeline RenderPipeline::createPipeline(const vk::raii::Device& device, const vk::Extent2D& extent) const
{
    const auto vertShaderCode = readFile("../shaders/vertex.spv");
    const vk::raii::ShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
    const vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = *vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    const auto fragShaderCode = readFile("../shaders/fragment.spv");
    const vk::raii::ShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);
    const vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = *fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    const std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

    constexpr vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };

    const vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    const vk::Rect2D scissor{
        .offset = { 0, 0 },
        .extent = extent
    };

    const vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    constexpr vk::PipelineRasterizationStateCreateInfo rasterizationCreateInfo{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    constexpr vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = vk::False,
        .alphaToOneEnable = vk::False
    };

    const vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
        .blendEnable = vk::False,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendCreateInfo{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = {{ 0.0f, 0.0f, 0.0f, 0.0f }}
    };

    constexpr std::array<vk::DynamicState, 2> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    const vk::GraphicsPipelineCreateInfo createInfo{
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationCreateInfo,
        .pMultisampleState = &multisampleCreateInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = *pipelineLayout,
        .renderPass = *renderPass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1
    };

    try
    {
        return device.createGraphicsPipeline(nullptr, createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create graphics pipeline with error code: " + std::to_string(error.code().value()));
    }
}

std::vector<vk::raii::Framebuffer> RenderPipeline::createFramebuffers(const vk::raii::Device& device, const SwapchainManager& swapchainManager) const
{
    std::vector<vk::raii::Framebuffer> framebuffers;
    for (const auto& imageView : swapchainManager.imageViews)
    {
        const vk::FramebufferCreateInfo createInfo{
            .renderPass = *renderPass,
            .attachmentCount = 1,
            .pAttachments = &(*imageView),
            .width = swapchainManager.extent.width,
            .height = swapchainManager.extent.height,
            .layers = 1
        };

        try
        {
            framebuffers.emplace_back(device.createFramebuffer(createInfo));
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create framebuffer with error code: " + std::to_string(error.code().value()));
        }
    }

    return framebuffers;
}

std::vector<char> RenderPipeline::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

    file.close();

    return buffer;
}

vk::raii::ShaderModule RenderPipeline::createShaderModule(const vk::raii::Device& device, const std::vector<char>& code)
{
    const vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    try
    {
        return device.createShaderModule(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create shader module with error code: " + std::to_string(error.code().value()));
    }
}
