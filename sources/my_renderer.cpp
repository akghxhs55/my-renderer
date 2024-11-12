#include "my_renderer.h"

#include <set>
#include <fstream>


MyRenderer::MyRenderer() :
    environment(ApplicationName, ApplicationVersion),
    window(environment.instance),
    physicalDeviceData(environment.instance, window.surface, deviceExtensions),
    device(createDevice(physicalDeviceData.physicalDevice, physicalDeviceData.graphicsQueueFamilyIndex.value())),
    graphicsQueue(device.getQueue(physicalDeviceData.graphicsQueueFamilyIndex.value(), 0)),
    presentQueue(device.getQueue(physicalDeviceData.presentQueueFamilyIndex.value(), 0)),
    swapchainData(window, physicalDeviceData, device),
    pipelineLayout(createPipelineLayout(device)),
    renderPass(createRenderPass(device, swapchainData.surfaceFormat.format)),
    graphicsPipeline(createGraphicsPipeline(device)),
    swapchainFramebuffers(createFramebuffers(device)),
    commandPool(createCommandPool(device, physicalDeviceData.graphicsQueueFamilyIndex.value())),
    commandBuffer(createCommandBuffer(device, commandPool)),
    imageAvailableSemaphore(createSemaphore(device)),
    renderFinishedSemaphore(createSemaphore(device)),
    inFlightFence(createFence(device, vk::FenceCreateFlagBits::eSignaled))
{
}

MyRenderer::~MyRenderer() = default;

void MyRenderer::run() const
{
    while (!glfwWindowShouldClose(window.glfwWindow))
    {
        glfwPollEvents();
        drawFrame();
    }

    device.waitIdle();
}

vk::raii::Device MyRenderer::createDevice(const vk::raii::PhysicalDevice& physicalDevice, const uint32_t& graphicsQueueFamilyIndex) const
{
    constexpr float queuePriority = 1.0f;

    const std::vector<uint32_t> queueFamilyIndices = physicalDeviceData.getQueueFamilyIndices();
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for (const auto& queueFamilyIndex : queueFamilyIndices)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo{
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });
    }

    constexpr vk::PhysicalDeviceFeatures physicalDeviceFeatures{};

    const vk::DeviceCreateInfo createInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures
    };

    try
    {
        return physicalDevice.createDevice(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan device with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::PipelineLayout MyRenderer::createPipelineLayout(const vk::raii::Device& device)
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

vk::raii::Pipeline MyRenderer::createGraphicsPipeline(const vk::raii::Device& device) const
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
        .width = static_cast<float>(swapchainData.extent.width),
        .height = static_cast<float>(swapchainData.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    const vk::Rect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainData.extent
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
        .layout = pipelineLayout,
        .renderPass = renderPass,
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

std::vector<vk::raii::Framebuffer> MyRenderer::createFramebuffers(const vk::raii::Device& device) const
{
    std::vector<vk::raii::Framebuffer> framebuffers;
    for (const auto& imageView : swapchainData.imageViews)
    {
        const vk::FramebufferCreateInfo createInfo{
            .renderPass = renderPass,
            .attachmentCount = 1,
            .pAttachments = &(*imageView),
            .width = swapchainData.extent.width,
            .height = swapchainData.extent.height,
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

vk::raii::CommandPool MyRenderer::createCommandPool(const vk::raii::Device& device,
    const uint32_t& queueFamilyIndex)
{
    const vk::CommandPoolCreateInfo createInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueFamilyIndex
    };

    try
    {
        return device.createCommandPool(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create command pool with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::CommandBuffer MyRenderer::createCommandBuffer(const vk::raii::Device& device,
    const vk::raii::CommandPool& commandPool)
{
    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    try
    {
        return std::move(device.allocateCommandBuffers(allocateInfo)[0]);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to allocate command buffers with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::Semaphore MyRenderer::createSemaphore(const vk::raii::Device& device)
{
    try
    {
        return device.createSemaphore({});
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create semaphore with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::Fence MyRenderer::createFence(const vk::raii::Device& device, const vk::FenceCreateFlags& flags)
{
    try
    {
        return device.createFence({ .flags = flags });
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create fence with error code: " + std::to_string(error.code().value()));
    }
}

void MyRenderer::drawFrame() const
{
    if (const vk::Result result = device.waitForFences(*inFlightFence, vk::True, std::numeric_limits<uint64_t>::max()); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence with error code: " + vk::to_string(result));
    }
    device.resetFences(*inFlightFence);

    const auto& [result, imageIndex] = swapchainData.swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to acquire next image with error code: " + vk::to_string(result));
    }

    commandBuffer.reset();
    recordCommandBuffer(commandBuffer, imageIndex, renderPass, graphicsPipeline, swapchainFramebuffers, swapchainData.extent);

    const std::array<vk::Semaphore, 1> waitSemaphores = { *imageAvailableSemaphore };
    constexpr std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    const std::array<vk::Semaphore, 1> signalSemaphores = { *renderFinishedSemaphore };

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &(*commandBuffer),
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data()
    };

    try
    {
        graphicsQueue.submit(submitInfo, *inFlightFence);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to submit queue with error code: " + std::to_string(error.code().value()));
    }

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &(*swapchainData.swapchain),
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    if (const vk::Result result = presentQueue.presentKHR(presentInfo); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present queue with error code: " + vk::to_string(result));
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
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

    file.close();

    return buffer;
}

vk::raii::ShaderModule MyRenderer::createShaderModule(const vk::raii::Device& device, const std::vector<char>& code)
{
    vk::ShaderModuleCreateInfo createInfo{
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

void MyRenderer::recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, const uint32_t& imageIndex,
    const vk::raii::RenderPass& renderPass, const vk::raii::Pipeline& graphicsPipeline,
    const std::vector<vk::raii::Framebuffer>& swapchainFramebuffers, const vk::Extent2D& swapchainExtent)
{
    constexpr vk::CommandBufferBeginInfo beginInfo{
        .pInheritanceInfo = nullptr
    };

    try
    {
        commandBuffer.begin(beginInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to begin recording command buffer with error code: " + std::to_string(error.code().value()));
    }

    constexpr vk::ClearValue clearColor{ std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };

    const vk::RenderPassBeginInfo renderPassBeginInfo{
        .renderPass = *renderPass,
        .framebuffer = *swapchainFramebuffers[imageIndex],
        .renderArea = {
            .offset = { 0, 0 },
            .extent = swapchainExtent
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

    const vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    commandBuffer.setViewport(0, viewport);

    const vk::Rect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainExtent
    };
    commandBuffer.setScissor(0, scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    try
    {
        commandBuffer.end();
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to end recording command buffer with error code: " + std::to_string(error.code().value()));
    }
}
