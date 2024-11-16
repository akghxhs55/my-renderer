#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "physical_device_manager.h"


class DeviceManager {
public:
    const vk::raii::Device device;
    const vk::raii::Queue graphicsQueue;
    const vk::raii::Queue presentQueue;

    explicit DeviceManager(const PhysicalDeviceManager& physicalDeviceManager);
    ~DeviceManager();

private:
    static vk::raii::Device createDevice(const PhysicalDeviceManager& physicalDeviceManager);
};


#endif //DEVICE_MANAGER_H
