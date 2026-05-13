#include "renderer/uniform/inc/uniform_storage_buffer.hpp"

namespace renderer {
namespace uniform {
UniformStorageBuffer::UniformStorageBuffer() : Uniform()
{
    m_type = UniformType::UNIFORM_TYPE_STORAGE_BUFFER;
}

size_t UniformStorageBuffer::getSize() const
{
    return m_size;
}
}   // namespace uniform
}   // namespace renderer
