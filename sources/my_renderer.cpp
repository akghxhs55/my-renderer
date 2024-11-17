#include "my_renderer.h"

#include <iostream>


MyRenderer::MyRenderer() :
    environment(ApplicationName, ApplicationVersion),
    window(environment.instance),
    physicalDeviceManager(environment.instance, window.surface),
    deviceManager(physicalDeviceManager),
    vertexBuffer(createBuffer(Vertex::Size * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer)),
    vertexBufferMemory(createDeviceMemory(vertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)),
    swapchainManager(window, physicalDeviceManager, deviceManager.device),
    renderPipeline(deviceManager.device, swapchainManager),
    commandBufferManager(deviceManager.device, physicalDeviceManager.graphicsQueueFamilyIndex, MaxFramesInFlight),
    currentFrame(0),
    imageAvailableSemaphore(createSemaphores(deviceManager.device)),
    renderFinishedSemaphore(createSemaphores(deviceManager.device)),
    inFlightFence(createFences(deviceManager.device, vk::FenceCreateFlagBits::eSignaled))
{
    vertexBuffer.bindMemory(vertexBufferMemory, 0);

    void* data = vertexBufferMemory.mapMemory(0, Vertex::Size * vertices.size());
    std::memcpy(data, vertices.data(), Vertex::Size * vertices.size());

    vertexBufferMemory.unmapMemory();
}

MyRenderer::~MyRenderer() = default;

void MyRenderer::run()
{
    while (!window.shouldClose())
    {
        glfwPollEvents();
        drawFrame();
    }

    deviceManager.device.waitIdle();
}

vk::raii::Buffer MyRenderer::createBuffer(const vk::DeviceSize& size, const vk::BufferUsageFlags& usage) const
{
    const vk::BufferCreateInfo createInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    try
    {
        return deviceManager.device.createBuffer(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create buffer with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::DeviceMemory MyRenderer::createDeviceMemory(const vk::raii::Buffer& buffer, const vk::MemoryPropertyFlags& properties) const
{
    const vk::MemoryRequirements memoryRequirements = buffer.getMemoryRequirements();

    const vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    try
    {
        return deviceManager.device.allocateMemory(allocateInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to allocate memory with error code: " + std::to_string(error.code().value()));
    }
}

std::vector<vk::raii::Semaphore> MyRenderer::createSemaphores(const vk::raii::Device& device)
{
    std::vector<vk::raii::Semaphore> semaphores;
    semaphores.reserve(MaxFramesInFlight);

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
    fences.reserve(MaxFramesInFlight);

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

uint32_t MyRenderer::findMemoryType(uint32_t typeFilter, const vk::MemoryPropertyFlags& properties) const
{
    const vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDeviceManager.physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) and
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

void MyRenderer::drawFrame()
{
    if (const vk::Result result = deviceManager.device.waitForFences(*inFlightFence[currentFrame], vk::True, std::numeric_limits<uint64_t>::max()); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence with error code: " + vk::to_string(result));
    }

    const auto& [acquireImageResult, imageIndex] = swapchainManager.getSwapchain().acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore[currentFrame], nullptr);
    if (acquireImageResult == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (acquireImageResult != vk::Result::eSuccess and acquireImageResult != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire next image with error code: " + vk::to_string(acquireImageResult));
    }

    deviceManager.device.resetFences(*inFlightFence[currentFrame]);

    commandBufferManager.resetCommandBuffer(currentFrame);
    commandBufferManager.recordCommandBuffer(currentFrame, renderPipeline, imageIndex, swapchainManager.extent, { *vertexBuffer }, vertices.size());

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
        .pSwapchains = &(*swapchainManager.getSwapchain()),
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    const vk::Result presentResult = deviceManager.presentQueue.presentKHR(presentInfo);
    if (presentResult == vk::Result::eErrorOutOfDateKHR or presentResult == vk::Result::eSuboptimalKHR or window.wasFramebufferResized())
    {
        window.resetFramebufferResized();
        recreateSwapchain();
    }
    else if (presentResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present queue with error code: " + vk::to_string(presentResult));
    }

    currentFrame = (currentFrame + 1) % MaxFramesInFlight;
}

void MyRenderer::recreateSwapchain()
{
    deviceManager.device.waitIdle();
    renderPipeline.clearSwapchainFramebuffers();
    swapchainManager.recreateSwapchain();
    renderPipeline.recreateSwapchainFramebuffers();
}
