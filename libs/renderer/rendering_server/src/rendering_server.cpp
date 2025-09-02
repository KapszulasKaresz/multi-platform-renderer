#include "renderer/rendering_server/inc/rendering_server.hpp"

#include <stdexcept>

namespace renderer {
namespace rendering_server {

RenderingServer::RenderingServer() {}

RenderingServer& RenderingServer::getInstance()
{
    static RenderingServer s_instance;
    return s_instance;
}

void RenderingServer::setRenderMode(RenderMode f_renderMode)
{
    m_renderMode = f_renderMode;
}

RenderMode RenderingServer::getRenderMode() const
{
    return m_renderMode;
}

void RenderingServer::setRenderTarget(
    std::unique_ptr<render_target::RenderTarget>&& f_renderTarget
)
{
    m_renderTarget = std::move(f_renderTarget);
}

void RenderingServer::setWindow(std::unique_ptr<window::Window>&& f_window) {
    m_window = std::move(f_window);
}

void RenderingServer::frame() {}

void RenderingServer::mainLoop() {
    if(m_renderMode != RENDER_MODE_LOOP)
    {
        throw std::runtime_error("RenderingServer::mainLoop() renderingMode is not RENDER_MODE_LOOP");
    }
}


}   // namespace rendering_server
}   // namespace renderer
