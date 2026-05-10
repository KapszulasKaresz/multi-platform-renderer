#ifndef UNIFORM_ARRAY_HPP_INCLUDED
#define UNIFORM_ARRAY_HPP_INCLUDED

#include "renderer/uniform/inc/uniform.hpp"

namespace renderer {
namespace uniform {
class UniformCollection;

class UniformArray : public Uniform {
public:
    UniformArray();

    UniformArray& setType(UniformType f_type) override final;
    UniformArray& setName(const std::string& f_name) override final;

    UniformArray& setMaxElementCount(size_t f_elementCount);
    size_t        getMaxElementCount() const;

    UniformArray& addElement(std::shared_ptr<UniformCollection> f_element);
    UniformArray& removeLastElement();
    UniformArray& resize(size_t f_newElementCount);
    size_t        getCurrentElementCount() const;

    std::shared_ptr<UniformCollection> getElement(size_t f_index);

    virtual void update() = 0;

protected:
    UniformType m_elementType;
    size_t      m_maxElementCount{ 0 };
    size_t      m_currentElementCount{ 0 };

    std::vector<std::shared_ptr<UniformCollection>> m_elements;
};

}   // namespace uniform
}   // namespace renderer

#endif
