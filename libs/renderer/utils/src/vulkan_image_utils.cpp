#include "renderer/utils/inc/vulkan_image_utils.hpp"

#include <stdexcept>

namespace renderer {
namespace utils {
VmaImage::VmaImage()
    : m_allocator(VK_NULL_HANDLE),
      m_image(VK_NULL_HANDLE),
      m_allocation(VK_NULL_HANDLE)
{}

VmaImage::VmaImage(
    VmaAllocator             f_allocator,
    vk::ImageCreateInfo      f_imageCreateInfo,
    VmaMemoryUsage           f_memoryUsage,
    VmaAllocationCreateFlags f_flags
)
    : m_allocator(f_allocator),
      m_image(VK_NULL_HANDLE),
      m_allocation(VK_NULL_HANDLE)
{
    VmaAllocationCreateInfo l_allocCreateInfo = { .flags    = f_flags,
                                                  .usage    = f_memoryUsage,
                                                  .priority = 1.0f };

    if (vmaCreateImage(
            m_allocator, f_imageCreateInfo, &l_allocCreateInfo, &m_image, &m_allocation, nullptr
        )
        != VK_SUCCESS)
    {
        throw std::runtime_error("VmaImage::VmaImage(...) Failed to create VMA buffer");
    }
}

VmaImage::~VmaImage()
{
    if (m_image != VK_NULL_HANDLE) {
        vmaDestroyImage(m_allocator, m_image, m_allocation);
    }
}

VmaImage::VmaImage(VmaImage&& f_other) noexcept
    : m_allocator(f_other.m_allocator),
      m_image(f_other.m_image),
      m_allocation(f_other.m_allocation)
{
    f_other.m_image      = VK_NULL_HANDLE;
    f_other.m_allocation = VK_NULL_HANDLE;
}

VmaImage& VmaImage::operator=(VmaImage&& f_other) noexcept
{
    if (this != &f_other) {
        m_allocator          = f_other.m_allocator;
        m_image              = f_other.m_image;
        m_allocation         = f_other.m_allocation;
        f_other.m_image      = VK_NULL_HANDLE;
        f_other.m_allocation = VK_NULL_HANDLE;
    }
    return *this;
}

VkImage VmaImage::get() const
{
    return m_image;
}

VmaAllocation VmaImage::getAllocation() const
{
    return m_allocation;
}

}   // namespace utils
}   // namespace renderer
