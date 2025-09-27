#ifndef VULKAN_IMAGE_UTILS_HPP_INCLUDED
#define VULKAN_IMAGE_UTILS_HPP_INCLUDED

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>

namespace renderer {
namespace utils {
class VmaImage {
public:
    VmaImage();

    VmaImage(
        VmaAllocator             f_allocator,
        vk::ImageCreateInfo      f_imageCreateInfo,
        VmaMemoryUsage           f_memoryUsage,
        VmaAllocationCreateFlags f_flags = 0
    );

    ~VmaImage();

    VmaImage(const VmaImage&)            = delete;
    VmaImage& operator=(const VmaImage&) = delete;

    VmaImage(VmaImage&& f_other) noexcept;
    VmaImage& operator=(VmaImage&& f_other) noexcept;

    VkImage       get() const;
    VmaAllocation getAllocation() const;

private:
    VmaAllocator  m_allocator;
    VkImage       m_image;
    VmaAllocation m_allocation;
};

}   // namespace utils
}   // namespace renderer


#endif
