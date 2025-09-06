#ifndef RENDERING_API_HPP_INCLUDED
#define RENDERING_API_HPP_INCLUDED

#include <memory>

#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace rendering_api {

enum RenderingAPIType {
    RENDERING_API_TYPE_NONE,
    RENDERING_API_TYPE_VULKAN,
    RENDERING_API_TYPE_MAX
};

class RenderingApi {
public:
    RenderingApi() = default;

    virtual std::shared_ptr<rendering_device::RenderingDevice> getMainRenderingDevice();
    virtual std::shared_ptr<rendering_device::RenderingDevice> createRenderingDevice() = 0;

    RenderingAPIType getRenderingAPIType() const;
    bool isValid() const;
protected:
    bool m_valid{false};
    RenderingAPIType m_apiType{RENDERING_API_TYPE_NONE};
    std::shared_ptr<rendering_device::RenderingDevice> m_mainRenderingDevice{nullptr};
};
}   // namespace rendering_api
}   // namespace renderer

#endif
