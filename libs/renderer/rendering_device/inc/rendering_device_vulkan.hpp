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
#define VULKAN_FEATURE_CHAIN \
    vk::PhysicalDeviceFeatures2, \
    vk::PhysicalDeviceVulkan11Features, \
    vk::PhysicalDeviceVulkan12Features, \
    vk::PhysicalDeviceVulkan13Features, \
    vk::PhysicalDeviceVulkan14Features, \
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT

class RenderingDeviceVulkan : public RenderingDevice {
public:
    RenderingDeviceVulkan(rendering_api::RenderingApiVulkan* f_parentApi);

    std::shared_ptr<render_target::RenderTargetWindow> createRenderTargetWindow(
        window::Window* f_window
    ) override final;

    using FeatureChain = vk::StructureChain<VULKAN_FEATURE_CHAIN>;

    RenderingDeviceVulkan& addExtension(const char* f_extensionName);
    RenderingDeviceVulkan& addExtensions(std::vector<const char*>& f_extensionNames);
    RenderingDeviceVulkan& addQueue(vk::QueueFlagBits f_queueType);
    RenderingDeviceVulkan& setFeatures(FeatureChain f_featureChain);
    RenderingDeviceVulkan& create();

private:
    void pickPhysicalDevice();

    rendering_api::RenderingApiVulkan* m_parentApi{ nullptr };
    vk::PhysicalDevice                 m_physicalDevice{ nullptr };

    std::vector<const char*>       m_requiredExtension{};
    std::vector<vk::QueueFlagBits> m_requiredQueues{};
    FeatureChain                   m_requiredFeatures{};
};

}   // namespace rendering_device
}   // namespace renderer

#endif
