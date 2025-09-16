#include "renderer/uniform/inc/uniform_collection_vulkan.hpp"

namespace renderer {
namespace uniform {
size_t UniformCollectionVulkan::getAlignment() const
{
    return 16;   // Default memory alignment for structs in vulkan
}
}   // namespace uniform
}   // namespace renderer
