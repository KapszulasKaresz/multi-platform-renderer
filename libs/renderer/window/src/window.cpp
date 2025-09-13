#include "renderer/window/inc/window.hpp"

#include <stdexcept>

#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/rendering_api/inc/rendering_api.hpp"

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

int Window::getWidth() const
{
    return m_size.x;
}

int Window::getHeight() const
{
    return m_size.y;
}

bool Window::isResized() const
{
    return m_framebufferResized;
}

void Window::resizeHandled(bool f_resized)
{
    m_framebufferResized = f_resized;
}

}   // namespace window
}   // namespace renderer
