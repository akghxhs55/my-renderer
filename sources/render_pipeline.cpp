#include "render_pipeline.h"


#include "vertex.h"

#include <fstream>


RenderPipeline::RenderPipeline(const Environment& environment) :
    descriptorSetLayout(createDescriptorSetLayout(environment)),
    pipelineLayout(createPipelineLayout(environment)),
    renderPass(createRenderPass(environment)),
    pipeline(createGraphicsPipeline(environment))
{
}

RenderPipeline::~RenderPipeline() = default;

vk::raii::DescriptorSetLayout RenderPipeline::createDescriptorSetLayout(const Environment& environment)
{
    constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };
    constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding{
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr
    };
    constexpr std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

    const vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    return environment.device.createDescriptorSetLayout(createInfo);
}

vk::raii::PipelineLayout RenderPipeline::createPipelineLayout(const Environment& environment) const
{
    const vk::PipelineLayoutCreateInfo createInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    return environment.device.createPipelineLayout(createInfo);
}

vk::raii::RenderPass RenderPipeline::createRenderPass(const Environment& environment)
{
    const vk::AttachmentDescription colorAttachmentDescription{
        .format = environment.swapchainSurfaceFormat.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };
    const vk::AttachmentDescription depthAttachmentDescription{
        .format = environment.depthFormat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };
    const std::array<vk::AttachmentDescription, 2> attachments = { colorAttachmentDescription, depthAttachmentDescription };

    constexpr vk::AttachmentReference colorAttachmentReference{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    constexpr vk::AttachmentReference depthAttachmentReference{
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    const vk::SubpassDescription subpassDescription{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depthAttachmentReference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr
    };

    constexpr vk::SubpassDependency subpassDependency{
        .srcSubpass = vk::SubpassExternal,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
    };

    const vk::RenderPassCreateInfo createInfo{
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    return environment.device.createRenderPass(createInfo);
}

vk::raii::Pipeline RenderPipeline::createGraphicsPipeline(const Environment& environment) const
{
    const vk::raii::ShaderModule vertexShaderModule = createShaderModule(environment.device, readFile(ShaderPath + VertexShaderFilename));
    const vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = *vertexShaderModule,
        .pName = "main"
    };

    const vk::raii::ShaderModule fragmentShaderModule = createShaderModule(environment.device, readFile(ShaderPath + FragmentShaderFilename));
    const vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = *fragmentShaderModule,
        .pName = "main"
    };

    const std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageCreateInfos = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

    const vk::VertexInputBindingDescription vertexInputBindingDescription = Vertex::getBindingDescription();
    const std::array<vk::VertexInputAttributeDescription, 3> vertexInputAttributeDescriptions = Vertex::getAttributeDescriptions();
    const vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexInputBindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()
    };

    constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };

    constexpr vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
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

    constexpr vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
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
        .pDepthStencilState = &depthStencilStateCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = *pipelineLayout,
        .renderPass = *renderPass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1
    };

    return environment.device.createGraphicsPipeline(nullptr, createInfo);
}

vk::raii::ShaderModule RenderPipeline::createShaderModule(const vk::raii::Device& device, const std::vector<char>& code)
{
    const vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    return device.createShaderModule(createInfo);
}

std::vector<char> RenderPipeline::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    const std::streamsize fileSize = static_cast<std::streamsize>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


