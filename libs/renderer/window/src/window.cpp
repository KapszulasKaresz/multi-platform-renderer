#include "renderer/window/inc/window.hpp"

#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/rendering_api/inc/rendering_api.hpp"

#include <stdexcept>

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

int Window::getWidth() const
{
    return m_size.x;
}
int Window::getHeight() const
{
    return m_size.y;
}

}   // namespace window
}   // namespace renderer
