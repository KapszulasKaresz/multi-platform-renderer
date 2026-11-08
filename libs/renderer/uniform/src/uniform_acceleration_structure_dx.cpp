#include "renderer/uniform/inc/uniform_acceleration_structure_dx.hpp"

#include <stdexcept>

#include "renderer/ray_tracing/acceleration_structure/inc/acceleration_structure_dx.hpp"
#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace uniform {
UniformAccelerationStructureDX::UniformAccelerationStructureDX(
    rendering_device::RenderingDeviceDX* f_parentDevice
)
    : m_parentDevice(f_parentDevice)
{}

UniformAccelerationStructureDX& UniformAccelerationStructureDX::create()
{
    if (!m_parentDevice || !m_parentDevice->isValid()) {
        throw std::runtime_error(
            "UniformAccelerationStructureDX::create() requires a valid DirectX device"
        );
    }
    if (!m_accelerationStructure) {
        throw std::runtime_error(
            "UniformAccelerationStructureDX::create() requires an acceleration "
            "structure"
        );
    }

    auto* l_accelerationStructure =
        dynamic_cast<ray_tracing::AccelerationStructureDX*>(
            m_accelerationStructure.get()
        );
    if (!l_accelerationStructure || !l_accelerationStructure->isValid()) {
        throw std::runtime_error(
            "UniformAccelerationStructureDX::create() requires a valid DirectX "
            "acceleration structure"
        );
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};
    l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    l_srvDesc.RaytracingAccelerationStructure.Location =
        l_accelerationStructure->getAccelerationStructure()->GetGPUVirtualAddress();

    m_heapPosition = m_parentDevice->getCommonDescriptorHeapManager()->addSRV(
        nullptr, l_srvDesc
    );
    m_valid = true;
    return *this;
}

UINT UniformAccelerationStructureDX::getHeapOffset() const
{
    return m_heapPosition;
}

UniformAccelerationStructureDX::~UniformAccelerationStructureDX()
{
    if (m_parentDevice && m_heapPosition != UINT_MAX) {
        m_parentDevice->getCommonDescriptorHeapManager()->free(m_heapPosition);
    }
}
}   // namespace uniform
}   // namespace renderer
