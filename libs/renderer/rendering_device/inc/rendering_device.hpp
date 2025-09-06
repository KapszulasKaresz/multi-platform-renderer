#ifndef RENDERING_DEVICE_HPP_INCLUDED
#define RENDERING_DEVICE_HPP_INCLUDED

#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/window/inc/window.hpp"

#include <memory>

namespace renderer {
namespace rendering_device {
class RenderingDevice {
public:

    virtual std::shared_ptr<render_target::RenderTargetWindow> createRenderTargetWindow(const window::Window* f_window) = 0;

private:
};

}   // namespace rendering_device
}   // namespace renderer

#endif
