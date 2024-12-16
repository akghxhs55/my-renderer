#ifndef ABSTRACT_BUFFER_H
#define ABSTRACT_BUFFER_H


#include "i_buffer.h"
#include "environment.h"


class AbstractBuffer : public IBuffer
{
protected:
    std::reference_wrapper<const Environment> environment;
    vk::DeviceSize size;
    vk::raii::Buffer buffer;

public:
    AbstractBuffer(const Environment& environment, const vk::DeviceSize size, const vk::BufferUsageFlags usage);

    AbstractBuffer(const AbstractBuffer&) = delete;
    AbstractBuffer& operator=(const AbstractBuffer&) = delete;

    AbstractBuffer(AbstractBuffer&& other) noexcept;
    AbstractBuffer& operator=(AbstractBuffer&& other) noexcept;

    const vk::raii::Buffer& getBuffer() const override;

protected:
    vk::raii::Buffer createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage) const;
    vk::raii::DeviceMemory allocateBufferMemory(const vk::MemoryPropertyFlags properties) const;
};


#endif //ABSTRACT_BUFFER_H
