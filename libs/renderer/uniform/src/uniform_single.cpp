#include "renderer/uniform/inc/uniform_single.hpp"

namespace renderer {
namespace uniform {
template <typename T>
UniformSingle& UniformSingle::setValue(const T& f_value)
{
    m_value = f_value;
    return *this;
}
}   // namespace uniform
}   // namespace renderer
