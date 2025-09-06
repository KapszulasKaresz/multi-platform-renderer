#ifndef RENDERING_API_VULKAN_HPP_INCLUDED
#define RENDERING_API_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/rendering_api/inc/rendering_api.hpp"

namespace renderer {
namespace rendering_api {

class RenderingApiVulkan : public RenderingApi {
public:

    std::shared_ptr<rendering_device::RenderingDevice> createRenderingDevice() override final;
    
    RenderingApiVulkan& create();

    ~RenderingApiVulkan();
private:
    vk::raii::Instance m_instance{nullptr};

};
}   // namespace rendering_api
}   // namespace renderer

#endif