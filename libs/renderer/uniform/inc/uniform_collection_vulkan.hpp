#ifndef UNIFORM_COLLECTION_VULKAN_HPP_INCLUDED
#define UNIFORM_COLLECTION_VULKAN_HPP_INCLUDED

#include "renderer/uniform/inc/uniform_collection.hpp"

namespace renderer {
namespace uniform {
class UniformCollectionVulkan : public UniformCollection {
public:
    size_t getAlignment() const override final;

private:
};

}   // namespace uniform
}   // namespace renderer

#endif
