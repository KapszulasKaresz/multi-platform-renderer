#include "renderer/uniform/inc/uniform_single.hpp"

namespace renderer {
namespace uniform {

UniformSingle& UniformSingle::setType(UniformType f_type)
{
    Uniform::setType(f_type);
    return *this;
}

UniformSingle& UniformSingle::setName(const std::string& f_name)
{
    Uniform::setName(f_name);
    return *this;
}

UniformSingle& UniformSingle::setValue(const UniformValue& f_value)
{
    m_value = f_value;
    return *this;
}

UniformValue& UniformSingle::getValue()
{
    return m_value;
}

void* UniformSingle::valueAsVoid()
{
    return std::visit(
        [](auto& f_val) -> void* { return static_cast<void*>(&f_val); }, m_value
    );
}
}   // namespace uniform
}   // namespace renderer
