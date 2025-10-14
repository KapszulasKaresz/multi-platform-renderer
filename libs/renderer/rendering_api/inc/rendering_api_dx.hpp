#ifndef RENDERING_API_DX_HPP_INCLUDED
#define RENDERING_API_DX_HPP_INCLUDED

#include "renderer/rendering_api/inc/rendering_api.hpp"

namespace renderer {
namespace rendering_api {

class RenderingApiDX : public RenderingApi {
public:
    RenderingApiDX();

    std::shared_ptr<rendering_device::RenderingDevice>
        getMainRenderingDevice() override final;

    void createMainRenderingDeviceWindow(window::Window* f_window) override final;

    std::shared_ptr<rendering_device::RenderingDevice>
        createRenderingDevice() override final;

    RenderingApiDX& create();

private:
};
}   // namespace rendering_api
}   // namespace renderer

#endif
