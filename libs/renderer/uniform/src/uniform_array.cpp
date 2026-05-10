#include "renderer/uniform/inc/uniform_array.hpp"

#include <stdexcept>

#include "renderer/uniform/inc/uniform_collection.hpp"

namespace renderer {
namespace uniform {
UniformArray::UniformArray() : Uniform()
{
    m_type = UNIFORM_TYPE_ARRAY;
}

UniformArray& UniformArray::setType(UniformType f_type)
{
    return *this;
}

UniformArray& UniformArray::setName(const std::string& f_name)
{
    Uniform::setName(f_name);
    return *this;
}

UniformArray& UniformArray::setMaxElementCount(size_t f_elementCount)
{
    if (m_valid) {
        throw std::
            runtime_error(
                "UniformArray::setMaxElementCount(size_t f_elementCount) you can only "
                "set max element count on uncreated UniformArray"
            );
    }
    m_maxElementCount = f_elementCount;
    m_elements.reserve(f_elementCount);
    return *this;
}

size_t UniformArray::getMaxElementCount() const
{
    return m_maxElementCount;
}

UniformArray& UniformArray::addElement(std::shared_ptr<UniformCollection> f_element)
{
    if (m_currentElementCount >= m_maxElementCount) {
        throw std::runtime_error(
            "UniformArray::addElement(std::shared_ptr<UniformCollection> f_element) "
            "cannot add " "more elements than the max element count"
        );
    }
    if (f_element->getType() != UNIFORM_TYPE_ARRAY_MEMBER) {
        throw std::
            runtime_error(
                "UniformArray::addElement(std::shared_ptr<Uniform> f_element) element "
                "type does not match UNIFORM_TYPE_ARRAY_MEMBER"
            );
    }
    if (m_valid) {
        throw std::runtime_error(
            "UniformArray::addElement(std::shared_ptr<UniformCollection> f_element) "
            "cannot add elements to an already created UniformArray"
        );
    }
    m_elements.push_back(std::move(f_element));
    m_currentElementCount++;
    return *this;
}

UniformArray& UniformArray::removeLastElement()
{
    if (m_currentElementCount > 0) {
        m_currentElementCount--;
    }
    return *this;
}

UniformArray& UniformArray::resize(size_t f_newElementCount)
{
    if (f_newElementCount > m_maxElementCount) {
        throw std::runtime_error(
            "UniformArray::resize(size_t f_newElementCount) new element count cannot "
            "exceed max element count"
        );
    }
    m_currentElementCount = f_newElementCount;
    return *this;
}

size_t UniformArray::getCurrentElementCount() const
{
    return m_currentElementCount;
}

std::shared_ptr<UniformCollection> UniformArray::getElement(size_t f_index)
{
    if (f_index >= m_currentElementCount) {
        throw std::runtime_error(
            "UniformArray::getElement(size_t f_index) index out of bounds"
        );
    }
    return m_elements[f_index];
}

}   // namespace uniform
}   // namespace renderer
