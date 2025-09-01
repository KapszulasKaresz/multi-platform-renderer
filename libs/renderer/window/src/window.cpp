#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace window {
Window& Window::setSize(const glm::ivec2& f_size)
{
    m_size = f_size;
    return *this;
}

Window& Window::setTitle(const std::string& f_title)
{
    m_title = f_title;
    return *this;
}

bool Window::isValid() const
{
    return m_created;
}

}   // namespace window
}   // namespace renderer
