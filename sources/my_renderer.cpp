#include "my_renderer.h"

#include <iostream>


MyRenderer::MyRenderer() :
    window(WindowTitle, WindowWidth, WindowHeight),
    environment(ApplicationName, ApplicationVersion),
    surface(createSurface(environment.instance, window.glfwWindow)),
    deviceContext(environment.instance, surface, deviceExtensions),
    vertexBuffer(createBuffer(Vertex::Size * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer)),
    vertexBufferMemory(createDeviceMemory(vertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)),
    swapchainContext(deviceContext, window, surface),
    renderPipeline(deviceContext.device, swapchainContext),
    commandBufferManager(deviceContext.device, deviceContext.graphicsQueueFamilyIndex, MaxFramesInFlight),
    currentFrame(0),
    imageAvailableSemaphore(createSemaphores(deviceContext.device)),
    renderFinishedSemaphore(createSemaphores(deviceContext.device)),
    inFlightFence(createFences(deviceContext.device, vk::FenceCreateFlagBits::eSignaled))
{
    vertexBuffer.bindMemory(*vertexBufferMemory, 0);

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

    deviceContext.device.waitIdle();
}

vk::raii::SurfaceKHR MyRenderer::createSurface(const vk::raii::Instance& instance, GLFWwindow* window)
{
    VkSurfaceKHR surface;
    if (const VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(*instance), window, nullptr, &surface);
        result == VK_SUCCESS)
    {
        return vk::raii::SurfaceKHR(instance, surface);
    }
    else
    {
        throw std::runtime_error("Failed to create window surface with error code: " + std::to_string(result));
    }
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
        return deviceContext.device.createBuffer(createInfo);
        vk::raii::Buffer(deviceContext.device, createInfo);
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
        return deviceContext.device.allocateMemory(allocateInfo);
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
    const vk::PhysicalDeviceMemoryProperties memoryProperties = deviceContext.physicalDevice.getMemoryProperties();

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
    if (const vk::Result result = deviceContext.device.waitForFences(*inFlightFence[currentFrame], vk::True, std::numeric_limits<uint64_t>::max()); result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence with error code: " + vk::to_string(result));
    }

    const auto& [acquireImageResult, imageIndex] = swapchainContext.getSwapchain().acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphore[currentFrame], nullptr);
    if (acquireImageResult == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (acquireImageResult != vk::Result::eSuccess and acquireImageResult != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire next image with error code: " + vk::to_string(acquireImageResult));
    }

    deviceContext.device.resetFences(*inFlightFence[currentFrame]);

    commandBufferManager.resetCommandBuffer(currentFrame);
    commandBufferManager.recordCommandBuffer(currentFrame, renderPipeline, imageIndex, swapchainContext.extent, { *vertexBuffer }, vertices.size());

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
        deviceContext.graphicsQueue.submit(submitInfo, *inFlightFence[currentFrame]);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to submit queue with error code: " + std::to_string(error.code().value()));
    }

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &(*swapchainContext.getSwapchain()),
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    const vk::Result presentResult = deviceContext.presentQueue.presentKHR(presentInfo);
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
    deviceContext.device.waitIdle();
    renderPipeline.clearSwapchainFramebuffers();
    swapchainContext.recreateSwapchain();
    renderPipeline.recreateSwapchainFramebuffers();
}
