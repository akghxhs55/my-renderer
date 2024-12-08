#include "my_renderer.h"


MyRenderer::MyRenderer() :
    window(WindowTitle, WindowWidth, WindowHeight),
    environment(window, ApplicationName, ApplicationVersion),
    renderPipeline(environment),
    swapchainFramebuffers(environment.createSwapchainFramebuffers(renderPipeline.renderPass)),
    graphicsCommandBuffers(environment.createGraphicsCommandBuffers(MaxFramesInFlight)),
    syncObjects(createSyncObjects(environment, MaxFramesInFlight)),
    vertexBuffer(environment.createBuffer(Vertex::Size * sizeof(vertices), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer)),
    vertexBufferMemory(environment.allocateBufferMemory(vertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
{
    vertexBuffer.bindMemory(*vertexBufferMemory, 0);
    copyData(vertexBuffer, vertices.data(), Vertex::Size * sizeof(vertices));
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

    environment.graphicsQueue.submit(submitInfo, *inFlightFence);

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

    commandBuffer.bindVertexBuffers(0, *vertexBuffer, { 0 });

    commandBuffer.draw(vertices.size(), 1, 0, 0);

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

void MyRenderer::copyData(const vk::raii::Buffer& dstBuffer, const void* srcData, const vk::DeviceSize size) const
{
    const vk::raii::Buffer stagingBuffer = environment.createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc);
    const vk::raii::DeviceMemory stagingBufferMemory = environment.allocateBufferMemory(stagingBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    stagingBuffer.bindMemory(*stagingBufferMemory, 0);

    void* dstData = stagingBufferMemory.mapMemory(0, size);
    std::memcpy(dstData, srcData, size);
    stagingBufferMemory.unmapMemory();

    const vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();

    const vk::BufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    commandBuffer.copyBuffer(*stagingBuffer, *dstBuffer, copyRegion);

    submitSingleTimeCommands(commandBuffer);
}

vk::raii::CommandBuffer MyRenderer::beginSingleTimeCommands() const
{
    vk::raii::CommandBuffer commandBuffer = std::move(environment.createGraphicsCommandBuffers(1)[0]);

    constexpr vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void MyRenderer::submitSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const
{
    commandBuffer.end();

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    environment.graphicsQueue.submit(submitInfo, nullptr);
    environment.graphicsQueue.waitIdle();
}

std::vector<MyRenderer::SyncObjects> MyRenderer::createSyncObjects(const Environment& environment, const uint32_t count)
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
