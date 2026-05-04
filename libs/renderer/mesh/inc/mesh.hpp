#ifndef MESH_HPP_INCLUDED
#define MESH_HPP_INCLUDED

#include <array>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace mesh {
struct Vertex {
    glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_normal{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_tangent{ 0.0f, 0.0f, 0.0f };
    glm::vec2 m_texCoord{ 0.0f, 0.0f };
    glm::vec3 m_color{ 0.0f, 0.0f, 0.0f };

    static vk::VertexInputBindingDescription                  getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions();
};

class Mesh : public RenderResource {
public:
    virtual Mesh& create() = 0;

protected:
};
}   // namespace mesh
}   // namespace renderer

#endif
