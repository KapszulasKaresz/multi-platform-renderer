#ifndef WINDOW_HPP_INCLUDED
#define WINDOW_HPP_INCLUDED

#include <memory>
#include <string>

#include <glm/vec2.hpp>


namespace renderer {
namespace render_target {
    class RenderTargetWindow;
}

namespace rendering_api {
    class RenderingApi;
}
namespace window {
class Window {
public:
    Window() = default;

    virtual std::shared_ptr<render_target::RenderTargetWindow> getRenderTarget(
        rendering_api::RenderingApi* f_renderingApi
    );
    virtual std::shared_ptr<render_target::RenderTargetWindow> getRenderTarget() const;

    Window& setSize(const glm::ivec2& f_size);
    Window& setTitle(const std::string& f_title);

    virtual bool isValid() const;

    virtual Window& create() = 0;
    virtual bool    isOpen() = 0;
    virtual void    update() = 0;

    virtual ~Window() = default;

protected:
    glm::ivec2                                         m_size{ 600, 600 };
    bool                                               m_created{ false };
    std::string                                        m_title{ "empty window" };
    std::shared_ptr<render_target::RenderTargetWindow> m_renderTarget{ nullptr };
};
}   // namespace window
}   // namespace renderer

#endif
