#include "my_renderer.h"


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>


MyRenderer::MyRenderer() :
    window(WindowTitle, WindowWidth, WindowHeight),
    environment(window, ApplicationName, ApplicationVersion),
    vertexBuffer(std::make_unique<DeviceLocalBuffer>(environment, Vertex::Size * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer)),
    indexBuffer(std::make_unique<DeviceLocalBuffer>(environment, sizeof(indices), vk::BufferUsageFlagBits::eIndexBuffer)),
    descriptorSetLayout(createDescriptorSetLayout(environment.device)),
    descriptorPool(createDescriptorPool(environment, MaxFramesInFlight)),
    descriptorSets(createDescriptorSets(environment.device, descriptorPool, descriptorSetLayout, MaxFramesInFlight)),
    uniformBuffers(createUniformBuffers(environment, MaxFramesInFlight)),
    renderPipeline(environment, descriptorSetLayout),
    swapchainFramebuffers(environment.createSwapchainFramebuffers(renderPipeline.renderPass)),
    graphicsCommandBuffers(environment.createGraphicsCommandBuffers(MaxFramesInFlight)),
    syncObjects(createSyncObjects(environment, MaxFramesInFlight)),
    currentFrame(0)
{
    vertexBuffer->uploadData(vertices.data(), Vertex::Size * vertices.size());
    indexBuffer->uploadData(indices.data(), sizeof(indices));

    for (uint32_t i = 0; i < MaxFramesInFlight; ++i)
    {
        const vk::DescriptorBufferInfo bufferInfo{
            .buffer = *uniformBuffers[i]->getBuffer(),
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };

        const vk::WriteDescriptorSet descriptorWrite{
            .dstSet = *descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo,
            .pImageInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        environment.device.updateDescriptorSets({ descriptorWrite }, nullptr);
    }
}

MyRenderer::~MyRenderer() = default;

void MyRenderer::run()
{
    while (!window.shouldClose())
    {
        glfwPollEvents();
        update();
        drawFrame();
    }

    environment.device.waitIdle();
}

void MyRenderer::update() const
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto currentTime = std::chrono::high_resolution_clock::now();
    const float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{
        .model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .projection = glm::perspective(glm::radians(45.0f), environment.getSwapchainExtent().width / static_cast<float>(environment.getSwapchainExtent().height), 0.1f, 10.0f)
    };
    ubo.projection[1][1] *= -1;

    uniformBuffers[currentFrame]->uploadData(&ubo, sizeof(ubo));
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

    commandBuffer.bindVertexBuffers(0, *vertexBuffer->getBuffer(), { 0 });
    commandBuffer.bindIndexBuffer(*indexBuffer->getBuffer(), 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *renderPipeline.pipelineLayout, 0, *descriptorSets[currentFrame], nullptr);

    commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

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

vk::raii::DescriptorSetLayout MyRenderer::createDescriptorSetLayout(const vk::raii::Device& device)
{
    constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    const vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };

    return device.createDescriptorSetLayout(createInfo);
}

vk::raii::DescriptorPool MyRenderer::createDescriptorPool(const Environment& environment, const uint32_t count)
{
    const vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = count
    };

    const vk::DescriptorPoolCreateInfo createInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = count,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    return environment.device.createDescriptorPool(createInfo);
}

std::vector<vk::raii::DescriptorSet> MyRenderer::createDescriptorSets(const vk::raii::Device& device,
    const vk::raii::DescriptorPool& descriptorPool, const vk::raii::DescriptorSetLayout& descriptorSetLayout,
    const uint32_t count)
{
    const std::vector<vk::DescriptorSetLayout> layouts(count, *descriptorSetLayout);

    const vk::DescriptorSetAllocateInfo allocateInfo{
        .descriptorPool = *descriptorPool,
        .descriptorSetCount = count,
        .pSetLayouts = layouts.data()
    };

    return device.allocateDescriptorSets(allocateInfo);
}

std::vector<std::unique_ptr<IBuffer>> MyRenderer::createUniformBuffers(const Environment& environment, const uint32_t count)
{
    std::vector<std::unique_ptr<IBuffer>> uniformBuffers;
    uniformBuffers.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        uniformBuffers.emplace_back(std::make_unique<DeviceLocalBuffer>(environment, sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer));
    }

    return uniformBuffers;
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
