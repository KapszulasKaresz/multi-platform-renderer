#ifndef RENDERING_SERVER_HPP_INCLUDED
#define RENDERING_SERVER_HPP_INCLUDED

#include <memory>

#include "renderer/render_target/inc/render_target.hpp"
#include "renderer/rendering_api/inc/rendering_api.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace rendering_server {

enum RenderMode {
    RENDER_MODE_LOOP,
    RENDER_MODE_ON_DEMAND,
    RENDER_MODE_MAX
};

class RenderingServer {
private:
    RenderingServer();

    RenderingServer(const RenderingServer&)            = delete;
    RenderingServer& operator=(const RenderingServer&) = delete;

public:
    static RenderingServer& getInstance();

    void       setRenderMode(RenderMode f_renderMode);
    RenderMode getRenderMode() const;

    void setRenderTarget(std::unique_ptr<render_target::RenderTarget>&& f_renderTarget);
    void setWindow(std::unique_ptr<window::Window>&& f_renderTarget);
    void setRenderingApi(std::unique_ptr<rendering_api::RenderingApi>&& f_renderingApi);

    void frame();
    void mainLoop();

private:
    RenderMode m_renderMode{ RENDER_MODE_LOOP };
    bool       m_mainLoopRunning{ false };

    std::shared_ptr<render_target::RenderTarget> m_renderTarget{ nullptr };
    std::unique_ptr<window::Window>              m_window{ nullptr };
    std::unique_ptr<rendering_api::RenderingApi> m_renderingApi{ nullptr };
};
}   // namespace rendering_server
}   // namespace renderer


#endif
