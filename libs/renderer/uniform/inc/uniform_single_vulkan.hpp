#ifndef UNIFORM_VULKAN_HPP_INCLUDED
#define UNIFORM_VULKAN_HPP_INCLUDED

#include "renderer/uniform/inc/uniform_single.hpp"

namespace renderer {
namespace uniform {
class UniformSingleVulkan : public UniformSingle {
public:
    size_t getAlignment() const override final;
};

}   // namespace uniform
}   // namespace renderer
#endif
