#include "my_renderer.h"


MyRenderer::MyRenderer() :
    environment(ApplicationName, ApplicationVersion),
    window(environment.instance),
    physicalDeviceManager(environment.instance, window.surface, deviceExtensions),
    deviceManager(physicalDeviceManager, deviceExtensions),
    swapchainData(window, physicalDeviceManager, deviceManager.device),
    renderPipeline(deviceManager.device, swapchainData),
    commandPool(createCommandPool(deviceManager.device, physicalDeviceManager.graphicsQueueFamilyIndex.value())),
    commandBuffer(createCommandBuffer(deviceManager.device, commandPool)),
    imageAvailableSemaphore(createSemaphore(deviceManager.device)),
    renderFinishedSemaphore(createSemaphore(deviceManager.device)),
    inFlightFence(createFence(deviceManager.device, vk::FenceCreateFlagBits::eSignaled))
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

    deviceManager.device.waitIdle();
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
    if (const vk::Result result = deviceManager.device.waitForFences(*inFlightFence, vk::True, std::numeric_limits<uint64_t>::max()); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence with error code: " + vk::to_string(result));
    }
    deviceManager.device.resetFences(*inFlightFence);

    const auto& [result, imageIndex] = swapchainData.swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to acquire next image with error code: " + vk::to_string(result));
    }

    commandBuffer.reset();
    recordCommandBuffer(commandBuffer, imageIndex, renderPipeline, swapchainData.extent);

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
        deviceManager.graphicsQueue.submit(submitInfo, *inFlightFence);
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

    if (const vk::Result result = deviceManager.presentQueue.presentKHR(presentInfo); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present queue with error code: " + vk::to_string(result));
    }
}

void MyRenderer::recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, const uint32_t& imageIndex, const RenderPipeline& renderPipeline, const vk::Extent2D& swapchainExtent)
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
        .renderPass = *renderPipeline.renderPass,
        .framebuffer = *renderPipeline.swapchainFramebuffers[imageIndex],
        .renderArea = {
            .offset = { 0, 0 },
            .extent = swapchainExtent
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPipeline.pipeline);

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
