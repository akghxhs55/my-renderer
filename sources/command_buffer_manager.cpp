#include "command_buffer_manager.h"


CommandBufferManager::CommandBufferManager(const vk::raii::Device& device, const uint32_t& queueFamilyIndex) :
    commandPool(createCommandPool(device, queueFamilyIndex)),
    commandBuffer(createCommandBuffer(device))
{
}

CommandBufferManager::~CommandBufferManager() = default;

void CommandBufferManager::recordCommandBuffer(const uint32_t& imageIndex, const RenderPipeline& renderPipeline,
                                  const vk::Extent2D& swapchainExtent) const
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

vk::raii::CommandBuffer CommandBufferManager::createCommandBuffer(const vk::raii::Device& device) const
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


