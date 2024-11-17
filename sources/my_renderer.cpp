#include "my_renderer.h"


MyRenderer::MyRenderer() :
    environment(ApplicationName, ApplicationVersion),
    window(environment.instance),
    physicalDeviceManager(environment.instance, window.surface),
    deviceManager(physicalDeviceManager),
    swapchainManager(window, physicalDeviceManager, deviceManager.device),
    renderPipeline(deviceManager.device, swapchainManager),
    commandBufferManager(MaxFramesInFlight, deviceManager.device, physicalDeviceManager.graphicsQueueFamilyIndex),
    imageAvailableSemaphore(createSemaphores(deviceManager.device)),
    renderFinishedSemaphore(createSemaphores(deviceManager.device)),
    inFlightFence(createFences(deviceManager.device, vk::FenceCreateFlagBits::eSignaled)),
    currentFrame(0)
{
}

MyRenderer::~MyRenderer() = default;

void MyRenderer::run()
{
    while (!glfwWindowShouldClose(window.glfwWindow))
    {
        glfwPollEvents();
        drawFrame();
    }

    deviceManager.device.waitIdle();
}

std::vector<vk::raii::Semaphore> MyRenderer::createSemaphores(const vk::raii::Device& device)
{
    std::vector<vk::raii::Semaphore> semaphores;
    for (size_t i = 0; i < MaxFramesInFlight; ++i)
    {
        try
        {
            semaphores.emplace_back(device.createSemaphore({}));
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create semaphore with error code: " + std::to_string(error.code().value()));
        }
    }
    return semaphores;
}

std::vector<vk::raii::Fence> MyRenderer::createFences(const vk::raii::Device& device, const vk::FenceCreateFlags& flags)
{
    std::vector<vk::raii::Fence> fences;
    for (size_t i = 0; i < MaxFramesInFlight; ++i)
    {
        try
        {
            fences.emplace_back(device.createFence({ .flags = flags }));
        }
        catch (const vk::SystemError& error)
        {
            throw std::runtime_error("Failed to create fence with error code: " + std::to_string(error.code().value()));
        }
    }
    return fences;
}

void MyRenderer::drawFrame()
{
    if (const vk::Result result = deviceManager.device.waitForFences(*inFlightFence[currentFrame], vk::True, std::numeric_limits<uint64_t>::max()); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence with error code: " + vk::to_string(result));
    }
    deviceManager.device.resetFences(*inFlightFence[currentFrame]);

    const auto& [acquireImageResult, imageIndex] = swapchainManager.swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore[currentFrame], nullptr);
    if (acquireImageResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to acquire next image with error code: " + vk::to_string(acquireImageResult));
    }

    commandBufferManager.commandBuffers[currentFrame].reset();
    commandBufferManager.recordCommandBuffer(currentFrame, imageIndex, renderPipeline, swapchainManager.extent);

    const std::array<vk::Semaphore, 1> waitSemaphores = { *imageAvailableSemaphore[currentFrame] };
    constexpr std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    const std::array<vk::Semaphore, 1> signalSemaphores = { *renderFinishedSemaphore[currentFrame] };

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &(*commandBufferManager.commandBuffers[currentFrame]),
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data()
    };

    try
    {
        deviceManager.graphicsQueue.submit(submitInfo, *inFlightFence[currentFrame]);
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

    currentFrame = (currentFrame + 1) % MaxFramesInFlight;
}
