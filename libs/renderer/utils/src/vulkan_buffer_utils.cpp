#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

#include <stdexcept>

namespace renderer {
namespace utils {
VmaBuffer::VmaBuffer()
    : m_allocator(VK_NULL_HANDLE),
      m_buffer(VK_NULL_HANDLE),
      m_allocation(VK_NULL_HANDLE)
{}

VmaBuffer::VmaBuffer(
    VmaAllocator                       f_allocator,
    VkDeviceSize                       f_size,
    vk::Flags<vk::BufferUsageFlagBits> f_usage,
    VmaMemoryUsage                     f_memoryUsage,
    VmaAllocationCreateFlags           f_flags
)
    : m_allocator(f_allocator),
      m_buffer(VK_NULL_HANDLE),
      m_allocation(VK_NULL_HANDLE)
{
    VkBufferCreateInfo l_bufferInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                     .size  = f_size,
                                     .usage = static_cast<VkBufferUsageFlags>(f_usage),
                                     .sharingMode = VK_SHARING_MODE_EXCLUSIVE };

    VmaAllocationCreateInfo allocInfo{ .flags = f_flags, .usage = f_memoryUsage };

    if (vmaCreateBuffer(
            f_allocator, &l_bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr
        )
        != VK_SUCCESS)
    {
        throw std::runtime_error("VmaBuffer::VmaBuffer(...) Failed to create VMA buffer");
    }
}

VmaBuffer::~VmaBuffer()
{
    if (m_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
    }
}

VmaBuffer::VmaBuffer(VmaBuffer&& f_other) noexcept
    : m_allocator(f_other.m_allocator),
      m_buffer(f_other.m_buffer),
      m_allocation(f_other.m_allocation)
{
    f_other.m_buffer     = VK_NULL_HANDLE;
    f_other.m_allocation = VK_NULL_HANDLE;
}

VmaBuffer& VmaBuffer::operator=(VmaBuffer&& f_other) noexcept
{
    if (this != &f_other) {
        if (m_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
        }
        m_allocator          = f_other.m_allocator;
        m_buffer             = f_other.m_buffer;
        m_allocation         = f_other.m_allocation;
        f_other.m_buffer     = VK_NULL_HANDLE;
        f_other.m_allocation = VK_NULL_HANDLE;
    }
    return *this;
}

VkBuffer VmaBuffer::get() const
{
    return m_buffer;
}

VmaAllocation VmaBuffer::getAllocation() const
{
    return m_allocation;
}

void VmaBuffer::upload(const void* f_src, VkDeviceSize f_size, VkDeviceSize f_offset)
{
    if (vmaCopyMemoryToAllocation(m_allocator, f_src, m_allocation, f_offset, f_size)
        != VK_SUCCESS)
    {
        throw std::runtime_error("VmaBuffer::upload(...) failed to upload data to GPU");
    }
}

}   // namespace utils
}   // namespace  renderer
