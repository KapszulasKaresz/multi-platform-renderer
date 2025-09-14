#include "renderer/mesh/inc/mesh.hpp"

namespace renderer {
namespace mesh {
vk::VertexInputBindingDescription Vertex::getBindingDescription()
{
    return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
}

std::array<vk::VertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions()
{
    return { vk::VertexInputAttributeDescription(
                 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, m_position)
             ),
             vk::VertexInputAttributeDescription(
                 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, m_normal)
             ),
             vk::VertexInputAttributeDescription(
                 2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, m_texCoord)
             ),
             vk::VertexInputAttributeDescription(
                 3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, m_color)
             ) };
}
}   // namespace mesh
}   // namespace renderer
