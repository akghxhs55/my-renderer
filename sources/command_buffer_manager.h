#ifndef COMMAND_BUFFER_MANAGER_H
#define COMMAND_BUFFER_MANAGER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "render_pipeline.h"


class CommandBufferManager {
private:
    const vk::raii::CommandPool commandPool;
public:
    const vk::raii::CommandBuffer commandBuffer;

    CommandBufferManager(const vk::raii::Device& device, const uint32_t& queueFamilyIndex);
    ~CommandBufferManager();

    void recordCommandBuffer(const uint32_t& imageIndex, const RenderPipeline& renderPipeline,
                const vk::Extent2D& swapchainExtent) const;

private:
    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, const uint32_t& queueFamilyIndex);
    vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device) const;
};


#endif //COMMAND_BUFFER_MANAGER_H
