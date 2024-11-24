#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "render_pipeline.h"


class CommandManager {
private:
    const vk::raii::CommandPool commandPool;
public:
    const std::vector<vk::raii::CommandBuffer> commandBuffers;

    CommandManager(const vk::raii::Device& device, const uint32_t queueFamilyIndex, const uint32_t count);
    ~CommandManager();

    void resetCommandBuffer(const uint32_t index) const;
    void recordCommandBuffer(const uint32_t index, const RenderPipeline& renderPipeline, const uint32_t imageIndex,
                const vk::Extent2D& swapchainExtent, const std::vector<vk::Buffer>& vertexBuffers, const uint32_t vertexCount) const;

private:
    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, const uint32_t queueFamilyIndex);
    std::vector<vk::raii::CommandBuffer> createCommandBuffers(const vk::raii::Device& device, const uint32_t count) const;
};


#endif //COMMAND_MANAGER_H
