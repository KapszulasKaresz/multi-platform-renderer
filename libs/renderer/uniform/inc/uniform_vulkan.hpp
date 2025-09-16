#ifndef UNIFORM_VULKAN_HPP_INCLUDED
#define UNIFORM_VULKAN_HPP_INCLUDED

#include "renderer/uniform/inc/uniform.hpp"

namespace renderer {
namespace uniform {
class UniformVulkan : public Uniform {
public:
    size_t getAlignment() const override final;
};

}   // namespace uniform
}   // namespace renderer
#endif
