#include "renderer/window/inc/window.hpp"

#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/rendering_api/inc/rendering_api.hpp"

#include <stdexcept>

namespace renderer {
namespace window {
std::shared_ptr<render_target::RenderTargetWindow> Window::getRenderTarget(
    rendering_api::RenderingApi* f_renderingApi
)
{
    m_renderTarget = f_renderingApi->getMainRenderingDevice()->createRenderTargetWindow(this);

    return m_renderTarget;
}

std::shared_ptr<render_target::RenderTargetWindow> Window::getRenderTarget() const
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
