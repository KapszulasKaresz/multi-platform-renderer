#ifndef UNIFORM_ACCELERATION_STRUCTURE_DX_HPP_INCLUDED
#define UNIFORM_ACCELERATION_STRUCTURE_DX_HPP_INCLUDED

#include <d3d12.h>

#include "renderer/uniform/inc/uniform_acceleration_structure.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceDX;
}

namespace uniform {
class UniformAccelerationStructureDX final : public UniformAccelerationStructure {
public:
    explicit UniformAccelerationStructureDX(
        rendering_device::RenderingDeviceDX* f_parentDevice
    );

    UniformAccelerationStructureDX& create() override final;
    UINT getHeapOffset() const;

    ~UniformAccelerationStructureDX();

private:
    rendering_device::RenderingDeviceDX* m_parentDevice{ nullptr };
    UINT                                 m_heapPosition{ UINT_MAX };
};
}   // namespace uniform
}   // namespace renderer

#endif
