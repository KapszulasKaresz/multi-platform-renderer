#include "renderer/utils/inc/vulkan_buffer_utils.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

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

size_t VmaBuffer::getSize() const
{
    VmaAllocationInfo l_allocInfo;
    vmaGetAllocationInfo(m_allocator, m_allocation, &l_allocInfo);
    return l_allocInfo.size;
}

std::vector<uint8_t> VmaBuffer::copyDataToCPU()
{
    VmaAllocationInfo l_allocInfo;
    vmaGetAllocationInfo(m_allocator, m_allocation, &l_allocInfo);

    void* l_mappedData = nullptr;
    if (vmaMapMemory(m_allocator, m_allocation, &l_mappedData) != VK_SUCCESS) {
        throw std::runtime_error("VmaBuffer::copyDataToCPU() Map error");
    }

    vmaInvalidateAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE);

    std::vector<uint8_t> l_cpuData(l_allocInfo.size);
    std::memcpy(l_cpuData.data(), l_mappedData, l_allocInfo.size);

    vmaUnmapMemory(m_allocator, m_allocation);

    return l_cpuData;
}

bool VmaBuffer::isHostVisible() const
{
    VmaAllocationInfo l_allocInfo;
    vmaGetAllocationInfo(m_allocator, m_allocation, &l_allocInfo);

    return l_allocInfo.pMappedData != nullptr;
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
