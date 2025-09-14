#ifndef VULKAN_BUFFER_UTILS_HPP_INCLUDED
#define VULKAN_BUFFER_UTILS_HPP_INCLUDED

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>

namespace renderer {
namespace utils {
class VmaBuffer {
public:
    VmaBuffer();

    VmaBuffer(
        VmaAllocator                       f_allocator,
        VkDeviceSize                       f_size,
        vk::Flags<vk::BufferUsageFlagBits> f_usage,
        VmaMemoryUsage                     f_memoryUsage,
        VmaAllocationCreateFlags           f_flags
    );

    ~VmaBuffer();

    VmaBuffer(const VmaBuffer&)            = delete;
    VmaBuffer& operator=(const VmaBuffer&) = delete;

    VmaBuffer(VmaBuffer&& f_other) noexcept;
    VmaBuffer& operator=(VmaBuffer&& f_other) noexcept;

    VkBuffer      get() const;
    VmaAllocation getAllocation() const;

    void upload(const void* f_src, VkDeviceSize f_size, VkDeviceSize f_offset = 0);

private:
    VmaAllocator  m_allocator;
    VkBuffer      m_buffer;
    VmaAllocation m_allocation;
};

}   // namespace utils
}   // namespace renderer


#endif
