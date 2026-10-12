#include "renderer/uniform/inc/uniform_acceleration_structure.hpp"

namespace renderer {
namespace uniform {
UniformAccelerationStructure::UniformAccelerationStructure()
{
    m_type = UNIFORM_TYPE_ACCELERATION_STRUCTURE;
}

UniformAccelerationStructure& UniformAccelerationStructure::setAccelerationStructure(
    std::shared_ptr<ray_tracing::AccelerationStructure> f_accelerationStructure
)
{
    m_accelerationStructure = std::move(f_accelerationStructure);
    return *this;
}

std::shared_ptr<ray_tracing::AccelerationStructure>
    UniformAccelerationStructure::getAccelerationStructure() const
{
    return m_accelerationStructure;
}

size_t UniformAccelerationStructure::getAlignment() const
{
    return alignof(void*);
}

UniformAccelerationStructure& UniformAccelerationStructure::create()
{
    Uniform::create();
    return *this;
}
}   // namespace uniform
}   // namespace renderer
