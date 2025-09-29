#include "renderer/utils/inc/raii_vma_allocator.hpp"

#include <stdexcept>

namespace renderer {
namespace utils {
RaiiVmaAllocator::RaiiVmaAllocator() : m_allocator(nullptr) {}

RaiiVmaAllocator::RaiiVmaAllocator(VmaAllocatorCreateInfo f_allocatorCreateInfo)
{
    if (vmaCreateAllocator(&f_allocatorCreateInfo, &m_allocator) != VK_SUCCESS) {
        throw std::runtime_error(
            "RaiiVmaAllocator::RaiiVmaAllocator Failed to create VMA allocator!"
        );
    }
}

RaiiVmaAllocator::~RaiiVmaAllocator()
{
    if (m_allocator != nullptr) {
        vmaDestroyAllocator(m_allocator);
    }
}

RaiiVmaAllocator::RaiiVmaAllocator(RaiiVmaAllocator&& f_other) noexcept
    : m_allocator(f_other.m_allocator)
{
    f_other.m_allocator = nullptr;
}

RaiiVmaAllocator& RaiiVmaAllocator::operator=(RaiiVmaAllocator&& f_other) noexcept
{
    if (this != &f_other) {
        m_allocator         = f_other.m_allocator;
        f_other.m_allocator = nullptr;
    }
    return *this;
}

VmaAllocator& RaiiVmaAllocator::get()
{
    return m_allocator;
}

}   // namespace utils
}   // namespace renderer
