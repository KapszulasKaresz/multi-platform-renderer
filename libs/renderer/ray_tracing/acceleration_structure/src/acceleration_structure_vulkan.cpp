#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure_vulkan.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

#include "renderer/command_buffer/inc/command_buffer_vulkan.hpp"
#include "renderer/mesh/inc/triangle_mesh_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/scene/node/inc/node.hpp"

namespace renderer {
namespace ray_tracing {
AccelerationStructureVulkan::AccelerationStructureVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : m_parentDevice(f_parentDevice)
{}

AccelerationStructureVulkan& AccelerationStructureVulkan::setScene(
    scene::Node* f_rootNode
)
{
    m_rootNode = f_rootNode;
    return *this;
}

AccelerationStructureVulkan& AccelerationStructureVulkan::create()
{
    if (!m_parentDevice || !m_parentDevice->isValid()) {
        throw std::runtime_error(
            "AccelerationStructureVulkan::create() requires a valid Vulkan device"
        );
    }
    if (!m_rootNode) {
        throw std::runtime_error(
            "AccelerationStructureVulkan::create() requires a scene root node"
        );
    }

    scene::RayTracingVisitor l_visitor;
    m_rootNode->applyVisitor(&l_visitor);
    const auto& l_meshInstances = l_visitor.getMeshInstances();
    if (l_meshInstances.empty()) {
        throw std::runtime_error(
            "AccelerationStructureVulkan::create() found no triangle mesh instances"
        );
    }

    buildBottomLevelAccelerationStructures(l_meshInstances);
    buildTopLevelAccelerationStructure(l_meshInstances);
    m_valid = true;
    return *this;
}

const vk::raii::AccelerationStructureKHR&
    AccelerationStructureVulkan::getAccelerationStructure() const
{
    return m_accelerationStructure;
}

void AccelerationStructureVulkan::buildBottomLevelAccelerationStructures(
    const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
)
{
    std::unordered_map<mesh::TriangleMesh*, uint32_t> l_meshIndices;
    std::vector<std::shared_ptr<mesh::TriangleMesh>> l_meshes;

    for (const auto& l_instance : f_meshInstances) {
        if (!l_meshIndices.contains(l_instance.m_mesh.get())) {
            l_meshIndices.emplace(
                l_instance.m_mesh.get(), static_cast<uint32_t>(l_meshes.size())
            );
            l_meshes.push_back(l_instance.m_mesh);
        }
    }

    m_bottomLevelAccelerationStructures.clear();
    m_bottomLevelBuffers.clear();
    m_bottomLevelScratchBuffers.clear();
    m_bottomLevelAccelerationStructures.reserve(l_meshes.size());
    m_bottomLevelBuffers.reserve(l_meshes.size());
    m_bottomLevelScratchBuffers.reserve(l_meshes.size());

    std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> l_buildInfos;
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR>     l_ranges;
    std::vector<vk::DeviceSize>                                 l_scratchSizes;
    std::vector<vk::AccelerationStructureGeometryKHR>           l_geometries;
    l_buildInfos.reserve(l_meshes.size());
    l_ranges.reserve(l_meshes.size());
    l_scratchSizes.reserve(l_meshes.size());
    l_geometries.reserve(l_meshes.size());

    for (const auto& l_mesh : l_meshes) {
        auto* l_meshVulkan = dynamic_cast<mesh::TriangleMeshVulkan*>(l_mesh.get());
        if (!l_meshVulkan || l_mesh->getIndicieCount() < 3) {
            throw std::runtime_error(
                "AccelerationStructureVulkan::create() requires Vulkan triangle meshes "
                "with indices"
            );
        }

        const vk::DeviceAddress l_vertexAddress =
            m_parentDevice->getLogicalDevice().getBufferAddress(
                { .buffer = l_meshVulkan->getVertexBuffer().get() }
            );
        const vk::DeviceAddress l_indexAddress =
            m_parentDevice->getLogicalDevice().getBufferAddress(
                { .buffer = l_meshVulkan->getIndexBuffer().get() }
            );
        const vk::AccelerationStructureGeometryTrianglesDataKHR l_triangles{
            .vertexFormat             = vk::Format::eR32G32B32Sfloat,
            .vertexData               = l_vertexAddress,
            .vertexStride             = sizeof(mesh::Vertex),
            .maxVertex                = static_cast<uint32_t>(l_mesh->getVertexCount() - 1),
            .indexType                = vk::IndexType::eUint32,
            .indexData                = l_indexAddress,
            .transformData            = vk::DeviceAddress{ 0 }
        };
        l_geometries.push_back({
            .geometryType = vk::GeometryTypeKHR::eTriangles,
            .geometry    = l_triangles,
            .flags       = vk::GeometryFlagBitsKHR::eOpaque
        });
        const uint32_t l_primitiveCount =
            static_cast<uint32_t>(l_mesh->getIndicieCount() / 3);
        vk::AccelerationStructureBuildGeometryInfoKHR l_buildInfo{
            .type          = vk::AccelerationStructureTypeKHR::eBottomLevel,
            .flags         = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
            .mode          = vk::BuildAccelerationStructureModeKHR::eBuild,
            .geometryCount = 1,
            .pGeometries   = &l_geometries.back()
        };
        const auto l_sizes =
            m_parentDevice->getLogicalDevice().getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice, l_buildInfo,
                l_primitiveCount
            );

        m_bottomLevelBuffers.emplace_back(
            m_parentDevice->getVmaAllocator(), l_sizes.accelerationStructureSize,
            vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
                | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            VMA_MEMORY_USAGE_GPU_ONLY
        );
        m_bottomLevelAccelerationStructures.emplace_back(
            m_parentDevice->getLogicalDevice(),
            vk::AccelerationStructureCreateInfoKHR{
                .buffer = m_bottomLevelBuffers.back().get(),
                .size   = l_sizes.accelerationStructureSize,
                .type   = vk::AccelerationStructureTypeKHR::eBottomLevel
            }
        );

        l_buildInfo.dstAccelerationStructure = *m_bottomLevelAccelerationStructures.back();
        l_buildInfos.push_back(l_buildInfo);
        l_ranges.push_back({ .primitiveCount = l_primitiveCount });
        l_scratchSizes.push_back(l_sizes.buildScratchSize);
    }

    for (const auto l_scratchSize : l_scratchSizes) {
        m_bottomLevelScratchBuffers.emplace_back(
            m_parentDevice->getVmaAllocator(), l_scratchSize,
            vk::BufferUsageFlagBits::eStorageBuffer
                | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            VMA_MEMORY_USAGE_GPU_ONLY
        );
    }

    auto l_commandBuffer = m_parentDevice->createSingleUseCommandBuffer();
    auto* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());
    l_commandBufferVulkan->begin();
    for (size_t i = 0; i < l_buildInfos.size(); ++i) {
        l_buildInfos[i].scratchData =
            m_parentDevice->getLogicalDevice().getBufferAddress(
                { .buffer = m_bottomLevelScratchBuffers[i].get() }
            );
        const vk::AccelerationStructureBuildRangeInfoKHR* l_range = &l_ranges[i];
        l_commandBufferVulkan->getNativeHandle().front().buildAccelerationStructuresKHR(
            l_buildInfos[i], l_range
        );
    }
    l_commandBufferVulkan->end().submit();
}

void AccelerationStructureVulkan::buildTopLevelAccelerationStructure(
    const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
)
{
    std::unordered_map<mesh::TriangleMesh*, uint32_t> l_meshIndices;
    for (const auto& l_instance : f_meshInstances) {
        if (!l_meshIndices.contains(l_instance.m_mesh.get())) {
            l_meshIndices.emplace(
                l_instance.m_mesh.get(), static_cast<uint32_t>(l_meshIndices.size())
            );
        }
    }

    std::vector<vk::AccelerationStructureInstanceKHR> l_instances;
    l_instances.reserve(f_meshInstances.size());
    for (const auto& l_instance : f_meshInstances) {
        const auto l_meshIndex = l_meshIndices.at(l_instance.m_mesh.get());
        const auto& l_blas = m_bottomLevelAccelerationStructures[l_meshIndex];
        const glm::mat4& l_transform = l_instance.m_transform;
        vk::TransformMatrixKHR l_vkTransform{};
        for (uint32_t row = 0; row < 3; ++row) {
            for (uint32_t column = 0; column < 4; ++column) {
                l_vkTransform.matrix[row][column] = l_transform[column][row];
            }
        }
        vk::AccelerationStructureInstanceKHR l_vkInstance{
            .transform                              = l_vkTransform,
            .instanceCustomIndex                    = l_instance.m_materialId,
            .mask                                    = 0xFF,
            .instanceShaderBindingTableRecordOffset = 0,
            .flags = static_cast<VkGeometryInstanceFlagsKHR>(
                vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable
            ),
            .accelerationStructureReference =
                m_parentDevice->getLogicalDevice().getAccelerationStructureAddressKHR(
                    { .accelerationStructure = *l_blas }
                )
        };
        l_instances.push_back(l_vkInstance);
    }

    m_instanceBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(),
        sizeof(vk::AccelerationStructureInstanceKHR) * l_instances.size(),
        vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
            | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );
    m_instanceBuffer.upload(l_instances.data(), m_instanceBuffer.getSize());

    const vk::AccelerationStructureGeometryInstancesDataKHR l_instancesData{
        .arrayOfPointers = false,
        .data = m_parentDevice->getLogicalDevice().getBufferAddress(
            { .buffer = m_instanceBuffer.get() }
        )
    };
    const vk::AccelerationStructureGeometryKHR l_geometry{
        .geometryType = vk::GeometryTypeKHR::eInstances,
        .geometry     = l_instancesData
    };
    vk::AccelerationStructureBuildGeometryInfoKHR l_buildInfo{
        .type          = vk::AccelerationStructureTypeKHR::eTopLevel,
        .flags         = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
        .mode          = vk::BuildAccelerationStructureModeKHR::eBuild,
        .geometryCount = 1,
        .pGeometries   = &l_geometry
    };
    const uint32_t l_instanceCount = static_cast<uint32_t>(l_instances.size());
    const auto l_sizes =
        m_parentDevice->getLogicalDevice().getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, l_buildInfo,
            l_instanceCount
        );
    m_accelerationStructureBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(), l_sizes.accelerationStructureSize,
        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
            | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY
    );
    m_accelerationStructure = vk::raii::AccelerationStructureKHR(
        m_parentDevice->getLogicalDevice(),
        { .buffer = m_accelerationStructureBuffer.get(),
          .size   = l_sizes.accelerationStructureSize,
          .type   = vk::AccelerationStructureTypeKHR::eTopLevel }
    );
    m_scratchBuffer = utils::VmaBuffer(
        m_parentDevice->getVmaAllocator(), l_sizes.buildScratchSize,
        vk::BufferUsageFlagBits::eStorageBuffer
            | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY
    );
    l_buildInfo.dstAccelerationStructure = *m_accelerationStructure;
    l_buildInfo.scratchData =
        m_parentDevice->getLogicalDevice().getBufferAddress(
            { .buffer = m_scratchBuffer.get() }
        );
    const vk::AccelerationStructureBuildRangeInfoKHR l_range{
        .primitiveCount = l_instanceCount
    };

    auto l_commandBuffer = m_parentDevice->createSingleUseCommandBuffer();
    auto* l_commandBufferVulkan =
        dynamic_cast<command_buffer::CommandBufferVulkan*>(l_commandBuffer.get());
    l_commandBufferVulkan->begin();
    l_commandBufferVulkan->getNativeHandle().front().buildAccelerationStructuresKHR(
        l_buildInfo, &l_range
    );
    l_commandBufferVulkan->end().submit();
}

}   // namespace ray_tracing
}   // namespace renderer
