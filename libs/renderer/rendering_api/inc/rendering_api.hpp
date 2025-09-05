#ifndef RENDERING_API_HPP_INCLUDED
#define RENDERING_API_HPP_INCLUDED

#include <memory>

#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace rendering_api {
class RenderingApi {
public:
    RenderingApi() = default;

    virtual std::shared_ptr<rendering_device::RenderingDevice> getMainRenderingDevice();
    virtual std::shared_ptr<rendering_device::RenderingDevice> createRenderingDevice() = 0;
private:
    std::shared_ptr<rendering_device::RenderingDevice> m_mainRenderingDevice{nullptr};
};
}   // namespace rendering_api
}   // namespace renderer

#endif
