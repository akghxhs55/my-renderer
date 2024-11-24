#include "command_manager.h"


CommandManager::CommandManager(const vk::raii::Device& device, const uint32_t queueFamilyIndex, const uint32_t count) :
    commandPool(createCommandPool(device, queueFamilyIndex)),
    commandBuffers(createCommandBuffers(device, count))
{
}

CommandManager::~CommandManager() = default;

void CommandManager::resetCommandBuffer(const uint32_t index) const
{
    commandBuffers[index].reset();
}

void CommandManager::recordCommandBuffer(const uint32_t index, const RenderPipeline& renderPipeline, const uint32_t imageIndex,
                                        const vk::Extent2D& swapchainExtent, const std::vector<vk::Buffer>& vertexBuffers, const uint32_t vertexCount) const
{
    constexpr vk::CommandBufferBeginInfo beginInfo{
        .pInheritanceInfo = nullptr
    };

    try
    {
        commandBuffers[index].begin(beginInfo);
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

    commandBuffers[index].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffers[index].bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPipeline.pipeline);

    const vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    commandBuffers[index].setViewport(0, viewport);

    const vk::Rect2D scissor{
        .offset = { 0, 0 },
        .extent = swapchainExtent
    };
    commandBuffers[index].setScissor(0, scissor);

    commandBuffers[index].bindVertexBuffers(0, vertexBuffers, { 0 });

    commandBuffers[index].draw(vertexCount, 1, 0, 0);

    commandBuffers[index].endRenderPass();

    try
    {
        commandBuffers[index].end();
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to end recording command buffer with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::CommandPool CommandManager::createCommandPool(const vk::raii::Device& device,
                                                              const uint32_t queueFamilyIndex)
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

std::vector<vk::raii::CommandBuffer> CommandManager::createCommandBuffers(const vk::raii::Device& device, const uint32_t count) const
{
    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = count
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


