#include "renderer/rendering_server/inc/rendering_server.hpp"

#include <stdexcept>

#include "renderer/command_buffer/inc/command_buffer.hpp"
#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/rendering_api/inc/rendering_api.hpp"
#include "renderer/scene/inc/test_scene.hpp"
#include "renderer/window/inc/window.hpp"

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

RenderingServer& RenderingServer::setWindow(std::unique_ptr<window::Window>&& f_window)
{
    m_window = std::move(f_window);
    return *this;
}

RenderingServer& RenderingServer::setScene(std::shared_ptr<scene::TestScene> f_scene)
{
    m_scene = f_scene;
    return *this;
}

RenderingServer& RenderingServer::setRenderingApi(
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

    return *this;
}

RenderingServer& RenderingServer::create()
{
    if (!m_renderingApi) {
        throw std::runtime_error("RenderingServer::create() no rendering API is set");
    }

    if (!m_renderingApi->isValid()) {
        throw std::runtime_error("RenderingServer::create() renderin API is invalid");
    }

    if (m_window) {
        m_renderingApi->createMainRenderingDeviceWindow(m_window.get());
    }
    else {
        auto _ignoreReturn = m_renderingApi->getMainRenderingDevice();
    }

    m_valid = true;

    return *this;
}

std::shared_ptr<rendering_device::RenderingDevice>
    RenderingServer::getMainRenderingDevice()
{
    return m_renderingApi->getMainRenderingDevice();
}

std::shared_ptr<rendering_device::RenderingDevice> RenderingServer::createRenderingDevice()
{
    return m_renderingApi->createRenderingDevice();
}

void RenderingServer::frame()
{
    if (!m_valid) {
        throw std::runtime_error(
            "RenderingServer::frame() rendering server isn't created yet"
        );
    }
    auto l_renderingDevice = m_renderingApi->getMainRenderingDevice();

    auto l_commandBuffer = l_renderingDevice->getRenderingCommandBuffer();
    if (l_renderingDevice->preFrame()) {
        l_commandBuffer->reset();
        l_commandBuffer->begin();
        l_commandBuffer->beginRendering(
            l_renderingDevice->getRenderTargetWindow(), glm::vec4(1.0, 0.0, 0.0, 1.0)
        );
        m_scene->recordCommandBuffer(l_commandBuffer.get());
        l_commandBuffer->endRendering();
        l_commandBuffer->end();
        l_commandBuffer->submit();
        l_renderingDevice->postFrame();
    }
}

void RenderingServer::mainLoop()
{
    if (!m_valid) {
        throw std::runtime_error(
            "RenderingServer::mainLoop() rendering server isn't created yet"
        );
    }

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
        m_renderingApi->getMainRenderingDevice()->finishRendering();
    }
}


}   // namespace rendering_server
}   // namespace renderer
