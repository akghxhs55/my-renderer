#ifndef VERTEX_H
#define VERTEX_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>


class Vertex {
public:
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static constexpr size_t Size = sizeof(pos) + sizeof(color) + sizeof(texCoord);

    bool operator==(const Vertex& other) const;

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
};


template<>
struct std::hash<Vertex>
{
    size_t operator()(Vertex const& vertex) const noexcept;
};



#endif //VERTEX_H
