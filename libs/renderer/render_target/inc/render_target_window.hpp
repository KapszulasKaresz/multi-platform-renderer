#ifndef RENDER_TARGET_WINDOW_HPP_INCLUDED
#define RENDER_TARGET_WINDOW_HPP_INCLUDED

#include "renderer/render_target/inc/render_target.hpp"

namespace renderer {
namespace window {
    class Window;
}
namespace render_target {

class RenderTargetWindow : public RenderTarget{
public:
    virtual RenderTargetWindow& setWindow(window::Window* f_window) = 0;

protected:
    window::Window* m_window;
};

}   // namespace render_target
}   // namespace renderer

#endif
