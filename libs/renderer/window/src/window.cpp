#include "renderer/window/inc/window.hpp"

#include <stdexcept>

namespace renderer {
namespace window {
std::shared_ptr<render_target::RenderTarget> Window::getRenderTarget() const
{
    if (m_renderTarget) {
        return m_renderTarget;
    }
    else {
        throw std::runtime_error(
            "Window::getRenderTarget() you cannot get the renderTarget without creating "
            "first with the renderingApi"
        );
    }
}

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
