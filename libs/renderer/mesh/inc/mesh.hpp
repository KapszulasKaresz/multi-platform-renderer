#ifndef MESH_HPP_INCLUDED
#define MESH_HPP_INCLUDED

#include <array>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include "renderer/render_resource/inc/render_resource.hpp"

namespace renderer {
namespace mesh {
struct Vertex {
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec2 m_texCoord;
    glm::vec3 m_color;

    static vk::VertexInputBindingDescription                  getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions();
};

class Mesh : public RenderResource {
public:
    virtual Mesh& create() = 0;

protected:
};
}   // namespace mesh
}   // namespace renderer

#endif
