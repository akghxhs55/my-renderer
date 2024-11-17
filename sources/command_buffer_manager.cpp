#include "command_buffer_manager.h"


CommandBufferManager::CommandBufferManager(const vk::raii::Device& device, const uint32_t& queueFamilyIndex, const uint32_t& maxFramesInFlight) :
    commandPool(createCommandPool(device, queueFamilyIndex)),
    commandBuffers(createCommandBuffers(device, maxFramesInFlight))
{
}

CommandBufferManager::~CommandBufferManager() = default;

void CommandBufferManager::resetCommandBuffer(const uint32_t& index) const
{
    commandBuffers[index].reset();
}

void CommandBufferManager::recordCommandBuffer(const uint32_t& bufferIndex, const RenderPipeline& renderPipeline, const uint32_t& imageIndex,
                                               const vk::Extent2D& swapchainExtent, const std::vector<vk::Buffer>& vertexBuffers, const uint32_t& vertexCount) const
{
    constexpr vk::CommandBufferBeginInfo beginInfo{
        .pInheritanceInfo = nullptr
    };

    try
    {
        commandBuffers[bufferIndex].begin(beginInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to begin recording command buffer with error code: " + std::to_string(error.code().value()));
    }

    constexpr vk::ClearValue clearColor{ std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };

    const vk::RenderPassBeginInfo renderPassBeginInfo{
        .renderPass = *renderPipeline.renderPass,
        .framebuffer = *renderPipeline.getSwapchainFramebuffer(imageIndex),
        .renderArea = {
            .offset = { 0, 0 },
            .extent = swapchainExtent
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    commandBuffers[bufferIndex].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffers[bufferIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPipeline.pipeline);

    const vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    commandBuffers[bufferIndex].setViewport(0, viewport);

    const vk::Rect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainExtent
    };
    commandBuffers[bufferIndex].setScissor(0, scissor);

    commandBuffers[bufferIndex].bindVertexBuffers(0, vertexBuffers, { 0 });

    commandBuffers[bufferIndex].draw(vertexCount, 1, 0, 0);

    commandBuffers[bufferIndex].endRenderPass();

    try
    {
        commandBuffers[bufferIndex].end();
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to end recording command buffer with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::CommandPool CommandBufferManager::createCommandPool(const vk::raii::Device& device,
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

std::vector<vk::raii::CommandBuffer> CommandBufferManager::createCommandBuffers(const vk::raii::Device& device, const uint32_t& maxFramesInFlight) const
{
    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = maxFramesInFlight
    };

    try
    {
        return device.allocateCommandBuffers(allocateInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to allocate command buffers with error code: " + std::to_string(error.code().value()));
    }
}


