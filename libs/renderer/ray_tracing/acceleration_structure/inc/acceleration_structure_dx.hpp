#ifndef ACCELERATION_STRUCTURE_DX_HPP_INCLUDED
#define ACCELERATION_STRUCTURE_DX_HPP_INCLUDED

#include <d3d12.h>
#include <D3D12MemAlloc.h>
#include <wrl.h>

#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure.hpp"
#include "renderer/scene/node_visitor/inc/ray_tracing_visitor.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}

namespace scene {
class Node;
}

namespace ray_tracing {
class AccelerationStructureDX final : public AccelerationStructure {
public:
    explicit AccelerationStructureDX(rendering_device::RenderingDeviceDX* f_parentDevice);

    AccelerationStructureDX& setScene(scene::Node* f_rootNode) override final;
    AccelerationStructureDX& create() override final;

    ID3D12Resource* getAccelerationStructure() const;

private:
    void buildBottomLevelAccelerationStructures(
        const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
    );
    void buildTopLevelAccelerationStructure(
        const std::vector<scene::RayTracingVisitor::MeshInstance>& f_meshInstances
    );

    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };
    scene::Node*                        m_rootNode{ nullptr };

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_accelerationStructure{ nullptr };
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_instanceBuffer{ nullptr };
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_scratchBuffer{ nullptr };
    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>>
        m_bottomLevelAccelerationStructures;
    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> m_bottomLevelScratchBuffers;
};
}   // namespace ray_tracing
}   // namespace renderer

#endif
