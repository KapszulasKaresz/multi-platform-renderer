#ifndef WINDOW_HPP_INCLUDED
#define WINDOW_HPP_INCLUDED

#include "renderer/render_target/inc/render_target.hpp"
#include <memory>

namespace renderer {
namespace window {
class Window : public render_target::RenderTarget {
public:
    Window() = default;

    virtual std::shared_ptr<render_target::RenderTarget> getRenderTarget() const = 0;

    virtual ~Window() = default;
private:

};
}   // namespace window
}   // namespace renderer

#endif
