#ifndef VERTEX_H
#define VERTEX_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>


class Vertex {
public:
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static constexpr size_t Size = sizeof(pos) + sizeof(color) + sizeof(texCoord);

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
};


#endif //VERTEX_H
