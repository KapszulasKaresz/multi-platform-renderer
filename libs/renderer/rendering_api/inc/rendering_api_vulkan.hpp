#ifndef RENDERING_API_VULKAN_HPP_INCLUDED
#define RENDERING_API_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/rendering_api/inc/rendering_api.hpp"

namespace renderer {
namespace rendering_api {

class RenderingApiVulkan : public RenderingApi {
public:
    RenderingApiVulkan();

    rendering_device::RenderingDevice* getMainRenderingDevice() override final;

    void createMainRenderingDeviceWindow(window::Window* f_window) override final;

    std::shared_ptr<rendering_device::RenderingDevice>
        createRenderingDevice() override final;

    RenderingApiVulkan& enableValidationLayers(bool f_enable);
    RenderingApiVulkan& addValidationLayer(const char* f_validationLayerName);
    RenderingApiVulkan& addExtension(const char* f_extensionName);
    RenderingApiVulkan& addExtensions(std::vector<const char*>& f_extensionNames);
    RenderingApiVulkan& create();

    vk::raii::Instance& getNativeHandle();

    ~RenderingApiVulkan();

private:
    void createInstance();
    void setupDebugMessenger();

    vk::raii::Context                m_context{};
    vk::raii::Instance               m_instance{ nullptr };
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger{ nullptr };

    bool                     m_validationLayersEnabled{ false };
    std::vector<const char*> m_validationLayers{};

    std::vector<const char*> m_extensions{};
};
}   // namespace rendering_api
}   // namespace renderer

#endif
