#include "renderer/mesh/inc/triangle_mesh_vulkan.hpp"

#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace mesh {
TriangleMeshVulkan::TriangleMeshVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : TriangleMesh(),
      m_parentDevice(f_parentDevice)
{}

TriangleMeshVulkan& TriangleMeshVulkan::create()
{
    TriangleMesh::create();
    createVertexBuffer();
    createIndexBuffer();
    m_valid = true;
    return *this;
}

utils::VmaBuffer& TriangleMeshVulkan::getVertexBuffer()
{
    return m_vertexBuffer;
}

utils::VmaBuffer& TriangleMeshVulkan::getIndexBuffer()
{
    return m_indexBuffer;
}

void TriangleMeshVulkan::createVertexBuffer()
{
    auto l_dataSize =
        static_cast<vk::DeviceSize>(sizeof(m_vertices[0]) * m_vertices.size());

    m_vertexBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        l_dataSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        VMA_MEMORY_USAGE_GPU_ONLY,
        0
    );

    auto l_stagingBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        l_dataSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        0
    );

    l_stagingBuffer.upload(m_vertices.data(), l_dataSize);

    auto l_commandBuffer = m_parentDevice->createCommandBuffer();
    command_buffer::CommandBufferVulkan* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());

    l_commandBufferVulkan->begin();
    l_commandBufferVulkan->copyBuffer(l_stagingBuffer, m_vertexBuffer, l_dataSize);
    l_commandBufferVulkan->end();
    l_commandBufferVulkan->submit();
}

void TriangleMeshVulkan::createIndexBuffer()
{
    auto l_dataSize = static_cast<vk::DeviceSize>(sizeof(m_indices[0]) * m_indices.size());

    m_indexBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        l_dataSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        VMA_MEMORY_USAGE_GPU_ONLY,
        0
    );

    auto l_stagingBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        l_dataSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        0
    );

    l_stagingBuffer.upload(m_indices.data(), l_dataSize);

    auto l_commandBuffer = m_parentDevice->createCommandBuffer();
    command_buffer::CommandBufferVulkan* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());

    l_commandBufferVulkan->begin();
    l_commandBufferVulkan->copyBuffer(l_stagingBuffer, m_indexBuffer, l_dataSize);
    l_commandBufferVulkan->end();
    l_commandBufferVulkan->submit();
}
}   // namespace mesh
}   // namespace renderer
