#ifndef ACCELERATION_STRUCTURE_VULKAN_HPP_INCLUDED
#define ACCELERATION_STRUCTURE_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure.hpp"
#include "renderer/scene/node_visitor/inc/ray_tracing_visitor.hpp"
#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

namespace renderer {
namespace scene {
class Node;
}

namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace ray_tracing {
class AccelerationStructureVulkan : public AccelerationStructure {
public:
    AccelerationStructureVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    AccelerationStructureVulkan& setScene(scene::Node* f_rootNode) override final;
    AccelerationStructureVulkan& create();

    const vk::raii::AccelerationStructureKHR& getAccelerationStructure() const;

protected:
    void buildBottomLevelAccelerationStructures(
        const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
    );
    void buildTopLevelAccelerationStructure(
        const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
    );

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };
    scene::Node*                            m_rootNode{ nullptr };

    vk::raii::AccelerationStructureKHR m_accelerationStructure{ nullptr };

    utils::VmaBuffer m_accelerationStructureBuffer{};
    utils::VmaBuffer m_scratchBuffer{};
    utils::VmaBuffer m_instanceBuffer{};

    std::vector<vk::raii::AccelerationStructureKHR> m_bottomLevelAccelerationStructures;
    std::vector<utils::VmaBuffer>                   m_bottomLevelBuffers;
    std::vector<utils::VmaBuffer>                   m_bottomLevelScratchBuffers;
};
}   // namespace ray_tracing
}   // namespace renderer

#endif
