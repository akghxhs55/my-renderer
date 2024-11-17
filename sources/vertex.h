#ifndef VERTEX_H
#define VERTEX_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>


class Vertex {
public:
    glm::vec2 pos;
    glm::vec3 color;

    static constexpr size_t Size = sizeof(pos) + sizeof(color);

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
};


#endif //VERTEX_H
