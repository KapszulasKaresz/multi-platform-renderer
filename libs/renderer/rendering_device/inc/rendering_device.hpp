#ifndef RENDERING_DEVICE_HPP_INCLUDED
#define RENDERING_DEVICE_HPP_INCLUDED

#include <memory>

namespace renderer {
namespace render_target {
class RenderTargetWindow;
}   // namespace render_target

namespace window {
class Window;
}   // namespace window

namespace rendering_device {
class RenderingDevice {
public:
    virtual std::shared_ptr<render_target::RenderTargetWindow> createRenderTargetWindow(
        const window::Window* f_window
    ) = 0;

private:
};

}   // namespace rendering_device
}   // namespace renderer

#endif
