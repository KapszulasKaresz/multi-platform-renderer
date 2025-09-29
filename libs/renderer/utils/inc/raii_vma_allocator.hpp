#ifndef RAII_VMA_ALLOCATOR
#define RAII_VMA_ALLOCATOR

#include <vk_mem_alloc.h>

namespace renderer {
namespace utils {
class RaiiVmaAllocator {
public:
    RaiiVmaAllocator();

    RaiiVmaAllocator(VmaAllocatorCreateInfo f_allocatorCreateInfo);

    ~RaiiVmaAllocator();

    RaiiVmaAllocator(const RaiiVmaAllocator&)            = delete;
    RaiiVmaAllocator& operator=(const RaiiVmaAllocator&) = delete;

    RaiiVmaAllocator(RaiiVmaAllocator&& f_other) noexcept;
    RaiiVmaAllocator& operator=(RaiiVmaAllocator&& f_other) noexcept;

    VmaAllocator& get();

private:
    VmaAllocator m_allocator;
};

}   // namespace utils
}   // namespace renderer

#endif
