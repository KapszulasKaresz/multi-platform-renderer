#ifndef TRIANGLE_MESH_VULKAN_HPP_INCLUDED
#define TRIANGLE_MESH_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#include <vk_mem_alloc.h>

#include "renderer/mesh/inc/triangle_mesh.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace mesh {

class TriangleMeshVulkan : public TriangleMesh {
public:
    TriangleMeshVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    TriangleMeshVulkan& create() override final;

    utils::VmaBuffer& getVertexBuffer();
    utils::VmaBuffer& getIndexBuffer();

protected:
    void createVertexBuffer();
    void createIndexBuffer();

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    utils::VmaBuffer m_vertexBuffer{};
    utils::VmaBuffer m_indexBuffer{};
};
}   // namespace mesh
}   // namespace renderer

#endif
