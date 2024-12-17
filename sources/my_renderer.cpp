#include "my_renderer.h"


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "device_local_buffer.h"
#include "host_visible_buffer.h"

#include <chrono>


MyRenderer::MyRenderer() :
    window(WindowTitle, WindowWidth, WindowHeight),
    environment(window, ApplicationName, ApplicationVersion, MaxFramesInFlight),
    renderPipeline(environment),
    vertexBuffer(std::make_unique<DeviceLocalBuffer>(environment, Vertex::Size * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer)),
    indexBuffer(std::make_unique<DeviceLocalBuffer>(environment, sizeof(indices), vk::BufferUsageFlagBits::eIndexBuffer)),
    uniformBuffers(createUniformBuffers(environment, MaxFramesInFlight)),
    textureImage(createTextureImage(environment)),
    textureSampler(createTextureSampler(environment)),
    descriptorSets(environment.createDescriptorSets(renderPipeline.descriptorSetLayout, MaxFramesInFlight)),
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

std::vector<std::unique_ptr<IBuffer>> MyRenderer::createUniformBuffers(const Environment& environment, const uint32_t count)
{
    std::vector<std::unique_ptr<IBuffer>> uniformBuffers;
    uniformBuffers.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        uniformBuffers.emplace_back(std::make_unique<HostVisibleBuffer>(environment, sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer));
    }

    return uniformBuffers;
}

DeviceLocalImage MyRenderer::createTextureImage(const Environment& environment)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("../textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const vk::DeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image.");
    }

    DeviceLocalImage image{environment, {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)}, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled};
    image.uploadData(pixels, imageSize);
    image.transitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    stbi_image_free(pixels);

    return image;
}

vk::raii::Sampler MyRenderer::createTextureSampler(const Environment& environment)
{
    const vk::SamplerCreateInfo createInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = environment.physicalDevice.getProperties().limits.maxSamplerAnisotropy,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f
    };

    return environment.device.createSampler(createInfo);
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
