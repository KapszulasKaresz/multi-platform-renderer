#include "renderer/rendering_server/inc/rendering_server.hpp"

#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/rendering_api/inc/rendering_api.hpp"
#include "renderer/window/inc/window.hpp"

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

void RenderingServer::setWindow(std::unique_ptr<window::Window>&& f_window)
{
    if (!m_renderingApi || !m_renderingApi->isValid()) {
        throw std::runtime_error(
            "RenderingServer::setWindow() a valid renderingAPI instance is needed to set "
            "up window"
        );
    }
    m_window = std::move(f_window);
    m_renderTarget = m_window->getRenderTarget(m_renderingApi.get());
}

void RenderingServer::setRenderingApi(
    std::unique_ptr<rendering_api::RenderingApi>&& f_renderingApi
)
{
    if (!m_renderingApi) {
        m_renderingApi = std::move(f_renderingApi);
    }
    else {
        throw std::runtime_error(
            "RenderingServer::setRenderingApi() you can only set rendering API once"
        );
    }
}

void RenderingServer::frame()
{
    if (!m_renderingApi) {
        throw std::runtime_error("RenderingServer::frame() no rendering API is set");
    }

    if (!m_renderingApi->isValid()) {
        throw std::runtime_error("RenderingServer::frame() renderin API is invalid");
    }

    auto l_renderingDevice = m_renderingApi->getMainRenderingDevice();
}

void RenderingServer::mainLoop()
{
    if (m_renderMode != RENDER_MODE_LOOP) {
        throw std::runtime_error(
            "RenderingServer::mainLoop() renderingMode is not RENDER_MODE_LOOP"
        );
    }

    if (m_window) {
        m_mainLoopRunning = true;
        while (m_window->isOpen() && m_renderMode == RENDER_MODE_LOOP) {
            m_window->update();
            frame();
        }
        m_mainLoopRunning = false;
    }
}


}   // namespace rendering_server
}   // namespace renderer
