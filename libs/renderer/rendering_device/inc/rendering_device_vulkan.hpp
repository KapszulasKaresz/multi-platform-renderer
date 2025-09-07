#ifndef RENDERING_DEVICE_VULKAN_HPP_INCLUDED
#define RENDERING_DEVICE_VULKAN_HPP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/rendering_device/inc/rendering_device.hpp"

namespace renderer {
namespace rendering_api {
class RenderingApiVulkan;
}   // namespace rendering_api

namespace rendering_device {
class RenderingDeviceVulkan : public RenderingDevice {
public:
    RenderingDeviceVulkan(rendering_api::RenderingApiVulkan* f_parentApi);

    std::shared_ptr<render_target::RenderTargetWindow> createRenderTargetWindow(
        window::Window* f_window
    ) override final;

    RenderingDeviceVulkan& addExtension(const char* f_extensionName);
    RenderingDeviceVulkan& addExtensions(std::vector<const char*>& f_extensionNames);
    RenderingDeviceVulkan& addQueue(vk::QueueFlagBits f_queueType);
    RenderingDeviceVulkan& create();

private:
    void pickPhysicalDevice();

    rendering_api::RenderingApiVulkan* m_parentApi{ nullptr };
    vk::PhysicalDevice                 m_physicalDevice{ nullptr };

    std::vector<const char*> m_requiredExtension{};
    std::vector<vk::QueueFlagBits> m_requiredQueues{};
};

}   // namespace rendering_device
}   // namespace renderer

#endif
