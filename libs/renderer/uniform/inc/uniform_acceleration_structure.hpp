#ifndef UNIFORM_ACCELERATION_STRUCTURE_HPP_INCLUDED
#define UNIFORM_ACCELERATION_STRUCTURE_HPP_INCLUDED

#include <memory>

#include "renderer/uniform/inc/uniform.hpp"

namespace renderer {
namespace ray_tracing {
class AccelerationStructure;
}

namespace uniform {
class UniformAccelerationStructure : public Uniform {
public:
    UniformAccelerationStructure();

    UniformAccelerationStructure& setAccelerationStructure(
        std::shared_ptr<ray_tracing::AccelerationStructure> f_accelerationStructure
    );
    std::shared_ptr<ray_tracing::AccelerationStructure> getAccelerationStructure() const;

    size_t getAlignment() const override final;
    UniformAccelerationStructure& create() override;

protected:
    std::shared_ptr<ray_tracing::AccelerationStructure> m_accelerationStructure;
};
}   // namespace uniform
}   // namespace renderer

#endif
