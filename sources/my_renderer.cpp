#include "my_renderer.h"


MyRenderer::MyRenderer() :
    window(WindowTitle, WindowWidth, WindowHeight),
    environment(window, ApplicationName, ApplicationVersion),
    renderPipeline(environment),
    swapchainFramebuffers(environment.createSwapchainFramebuffers(renderPipeline.renderPass)),
    graphicsCommandBuffers(environment.createGraphicsCommandBuffers(MaxFramesInFlight)),
    syncObjects(createSyncObjects(MaxFramesInFlight, environment))
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

    environment.device.waitIdle();
}

void MyRenderer::drawFrame()
{
    const vk::raii::CommandBuffer& graphicsCommandBuffer = graphicsCommandBuffers[currentFrame];
    const auto& [imageAvailableSemaphore, renderFinishedSemaphore, inFlightFence] = syncObjects[currentFrame];

    if (environment.device.waitForFences(*inFlightFence, true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence.");
    }

    const auto& [acquireImageResult, imageIndex] = environment.getSwapchain().acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore, nullptr);
    if (acquireImageResult == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (acquireImageResult != vk::Result::eSuccess and acquireImageResult != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image.");
    }

    environment.device.resetFences(*inFlightFence);

    graphicsCommandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    recordRenderCommand(*graphicsCommandBuffer, imageIndex);

    constexpr std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*imageAvailableSemaphore,
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &*graphicsCommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphore
    };

    try {
        environment.graphicsQueue.submit(submitInfo, *inFlightFence);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to submit draw command buffer.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*environment.getSwapchain(),
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    if (const vk::Result result = environment.presentQueue.presentKHR(presentInfo);
        result == vk::Result::eErrorOutOfDateKHR or
        result == vk::Result::eSuboptimalKHR or
        window.wasFramebufferResized())
    {
        window.resetFramebufferResized();
        recreateSwapchain();
    }
    else if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present swapchain image.");
    }

    currentFrame = (currentFrame + 1) % MaxFramesInFlight;
}

void MyRenderer::recordRenderCommand(const vk::CommandBuffer& commandBuffer, const uint32_t imageIndex) const
{
    constexpr vk::CommandBufferBeginInfo beginInfo{
        .pInheritanceInfo = nullptr
    };

    commandBuffer.begin(beginInfo);

    constexpr vk::ClearValue clearColor{ std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };
    const vk::RenderPassBeginInfo renderPassBeginInfo{
        .renderPass = *renderPipeline.renderPass,
        .framebuffer = *swapchainFramebuffers[imageIndex],
        .renderArea = {
            .offset = { 0, 0 },
            .extent = environment.getSwapchainExtent()
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPipeline.pipeline);

    commandBuffer.setViewport(0, environment.getViewport());
    commandBuffer.setScissor(0, environment.getScissor());

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void MyRenderer::recreateSwapchain()
{
    if (window.getFramebufferSize().first == 0 or
        window.getFramebufferSize().second == 0)
    {
        return;
    }

    environment.device.waitIdle();

    swapchainFramebuffers.clear();
    environment.recreateSwapchain();
    swapchainFramebuffers = environment.createSwapchainFramebuffers(renderPipeline.renderPass);
}

std::vector<MyRenderer::SyncObjects> MyRenderer::createSyncObjects(const uint32_t count, const Environment& environment)
{
    std::vector<SyncObjects> syncObjects;
    syncObjects.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        syncObjects.push_back({
            .imageAvailableSemaphore = environment.createSemaphore(),
            .renderFinishedSemaphore = environment.createSemaphore(),
            .inFlightFence = environment.createFence(vk::FenceCreateFlagBits::eSignaled)
        });
    }

    return syncObjects;
}
