#ifndef RENDERING_API_HPP_INCLUDED
#define RENDERING_API_HPP_INCLUDED

#include <memory>

#include "renderer/render_resource/inc/render_resource.hpp"
#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace window {
class Window;
}   // namespace window

namespace rendering_api {

enum RenderingAPIType {
    RENDERING_API_TYPE_NONE,
    RENDERING_API_TYPE_VULKAN,
    RENDERING_API_DIRECTX12,
    RENDERING_API_TYPE_MAX
};

class RenderingApi : public RenderResource {
public:
    RenderingApi() = default;

    virtual rendering_device::RenderingDevice* getMainRenderingDevice();
    virtual void createMainRenderingDeviceWindow(window::Window* f_window) = 0;
    virtual std::shared_ptr<rendering_device::RenderingDevice> createRenderingDevice() = 0;

    RenderingAPIType getRenderingAPIType() const;

protected:
    RenderingAPIType m_apiType{ RENDERING_API_TYPE_NONE };
    std::shared_ptr<rendering_device::RenderingDevice> m_mainRenderingDevice{ nullptr };

    bool m_useImgui{ true };
};
}   // namespace rendering_api
}   // namespace renderer

#endif
