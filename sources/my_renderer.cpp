#include "my_renderer.h"


MyRenderer::MyRenderer() :
    environment(ApplicationName, ApplicationVersion),
    window(environment.instance),
    physicalDeviceManager(environment.instance, window.surface),
    deviceManager(physicalDeviceManager),
    swapchainManager(window, physicalDeviceManager, deviceManager.device),
    renderPipeline(deviceManager.device, swapchainManager),
    commandBufferManager(deviceManager.device, physicalDeviceManager.graphicsQueueFamilyIndex),
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

    const auto& [acquireImageResult, imageIndex] = swapchainManager.swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, nullptr);
    if (acquireImageResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to acquire next image with error code: " + vk::to_string(acquireImageResult));
    }

    commandBufferManager.commandBuffer.reset();
    commandBufferManager.recordCommandBuffer(imageIndex, renderPipeline, swapchainManager.extent);

    const std::array<vk::Semaphore, 1> waitSemaphores = { *imageAvailableSemaphore };
    constexpr std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    const std::array<vk::Semaphore, 1> signalSemaphores = { *renderFinishedSemaphore };

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &(*commandBufferManager.commandBuffer),
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
        .pSwapchains = &(*swapchainManager.swapchain),
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    if (const vk::Result result = deviceManager.presentQueue.presentKHR(presentInfo); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present queue with error code: " + vk::to_string(result));
    }
}
