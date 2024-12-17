#ifndef DEVICE_LOCAL_IMAGE_H
#define DEVICE_LOCAL_IMAGE_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"


class DeviceLocalImage {
private:
    std::reference_wrapper<const Environment> environment;
    vk::Extent2D extent;
    vk::DeviceSize size;
    vk::ImageLayout currentLayout;
    vk::raii::Image image;
    vk::raii::DeviceMemory imageMemory;
public:
    vk::raii::ImageView imageView;

public:
    DeviceLocalImage(const Environment& environment, const vk::Extent2D extent, const vk::Format format, const vk::ImageUsageFlags usage);
    ~DeviceLocalImage();

    DeviceLocalImage(const DeviceLocalImage&) = delete;
    DeviceLocalImage& operator=(const DeviceLocalImage&) = delete;

    DeviceLocalImage(DeviceLocalImage&& other) noexcept;
    DeviceLocalImage& operator=(DeviceLocalImage&& other) noexcept;

    const vk::raii::Image& getImage() const;
    void uploadData(const void* sourceData, const vk::DeviceSize dataSize);
    void transitionImageLayout(const vk::ImageLayout newLayout);

private:
    vk::raii::Image createImage(const vk::Extent2D extent, const vk::Format format, const vk::ImageUsageFlags usage) const;
    vk::raii::DeviceMemory allocateImageMemory(const vk::MemoryPropertyFlags properties) const;
    vk::raii::ImageView createImageView(const vk::Format format) const;
};


#endif //DEVICE_LOCAL_IMAGE_H
