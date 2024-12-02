#include "my_renderer.h"


#include <fstream>


MyRenderer::MyRenderer() :
    window(WindowTitle, WindowWidth, WindowHeight),
    environment(window, ApplicationName, ApplicationVersion),
    pipelineLayout(createPipelineLayout(environment.device)),
    renderPass(createRenderPass(environment.device, environment.swapchainSurfaceFormat.format)),
    swapchainFramebuffers(environment.createSwapchainFramebuffers(renderPass)),
    graphicsPipeline(createGraphicsPipeline(environment.device, pipelineLayout, renderPass, environment.swapchainExtent))
{
}

MyRenderer::~MyRenderer() = default;

void MyRenderer::run()
{
    while (!window.shouldClose())
    {
        glfwPollEvents();
        drawFrame();
    }
}

void MyRenderer::drawFrame()
{
}

vk::raii::PipelineLayout MyRenderer::createPipelineLayout(const vk::raii::Device& device)
{
    constexpr vk::PipelineLayoutCreateInfo createInfo{
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    try
    {
        return device.createPipelineLayout(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create pipeline layout.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::RenderPass MyRenderer::createRenderPass(const vk::raii::Device& device,
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
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
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
        throw std::runtime_error("Failed to create render pass.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::Pipeline MyRenderer::createGraphicsPipeline(const vk::raii::Device& device,
                                                      const vk::raii::PipelineLayout& pipelineLayout, const vk::raii::RenderPass& renderPass,
                                                      const vk::Extent2D& swapchainExtent)
{
    const vk::raii::ShaderModule vertexShaderModule = createShaderModule(device, readFile("../shaders/vertex.spv"));
    const vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = *vertexShaderModule,
        .pName = "main"
    };

    const vk::raii::ShaderModule fragmentShaderModule = createShaderModule(device, readFile("../shaders/fragment.spv"));
    const vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = *fragmentShaderModule,
        .pName = "main"
    };

    const std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageCreateInfos = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

    constexpr vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };

    const vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    const vk::Rect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainExtent
    };

    const vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    constexpr vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    constexpr vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = vk::False,
        .alphaToOneEnable = vk::False
    };

    constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
        .blendEnable = vk::False,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = {{ 0.0f, 0.0f, 0.0f, 0.0f }}
    };

    constexpr std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    const vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    const vk::GraphicsPipelineCreateInfo createInfo{
        .stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size()),
        .pStages = shaderStageCreateInfos.data(),
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendStateCreateInfo,
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
        throw std::runtime_error("Failed to create graphics pipeline.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

vk::raii::ShaderModule MyRenderer::createShaderModule(const vk::raii::Device& device, const std::vector<char>& code)
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
        throw std::runtime_error("Failed to create shader module.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

std::vector<char> MyRenderer::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
