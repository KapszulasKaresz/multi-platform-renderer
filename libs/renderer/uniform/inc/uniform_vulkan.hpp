#ifndef UNIFORM_VULKAN_HPP_INCLUDED
#define UNIFORM_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace renderer {
namespace uniform {
class UniformVulkan {
public:
    virtual vk::DescriptorSetLayout  getDescriptorSetLayout() const = 0;
    virtual vk::raii::DescriptorSet& getDescriptorSet()             = 0;
};

}   // namespace uniform
}   // namespace renderer

#endif
