#include "vertex.h"


vk::VertexInputBindingDescription Vertex::getBindingDescription()
{
    return vk::VertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex
    };
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
{
    return {
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, pos)
        },
        vk::VertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, color)
        },
        vk::VertexInputAttributeDescription{
            .location = 2,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, texCoord)
        }
    };
}
