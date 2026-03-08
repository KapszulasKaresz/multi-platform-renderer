#include "renderer/command_buffer/inc/command_buffer.hpp"

#include "renderer/mesh/inc/mesh.hpp"
#include "renderer/mesh/inc/triangle_mesh.hpp"

namespace renderer {
namespace command_buffer {
CommandBuffer& CommandBuffer::draw(std::shared_ptr<mesh::Mesh> f_mesh)
{
    if (auto triangleMesh = std::dynamic_pointer_cast<mesh::TriangleMesh>(f_mesh)) {
        return draw(triangleMesh);
    }
    return *this;
}
}   // namespace command_buffer
}   // namespace renderer
