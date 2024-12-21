#ifndef I_BUFFER_H
#define I_BUFFER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual const vk::raii::Buffer& getBuffer() const = 0;
    virtual void uploadData(const void* sourceData, const vk::DeviceSize dataSize) const = 0;
};


#endif //I_BUFFER_H
