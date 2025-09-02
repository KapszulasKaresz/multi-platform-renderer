#ifndef WINDOW_HPP_INCLUDED
#define WINDOW_HPP_INCLUDED

#include <memory>
#include <string>

#include "glm/vec2.hpp"
#include "renderer/render_target/inc/render_target.hpp"

namespace renderer {
namespace window {
class Window {
public:
    Window() = default;

    virtual std::shared_ptr<render_target::RenderTarget> getRenderTarget() const = 0;

    Window& setSize(const glm::ivec2& f_size);
    Window& setTitle(const std::string& f_title);

    virtual bool isValid() const;

    virtual Window& create() = 0;

    virtual ~Window() = default;

protected:
    glm::ivec2  m_size{ 600, 600 };
    bool        m_created{ false };
    std::string m_title{ "empty window" };
};
}   // namespace window
}   // namespace renderer

#endif
