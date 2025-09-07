#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

#include "renderer/render_target/inc/render_target_window.hpp"
#include "renderer/render_target/inc/render_target_window_vulkan.hpp"
#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace rendering_device {
RenderingDeviceVulkan::RenderingDeviceVulkan(
    rendering_api::RenderingApiVulkan* f_parentApi
)
    : RenderingDevice(),
      m_parentApi(f_parentApi)
{}

std::shared_ptr<render_target::RenderTargetWindow>
    RenderingDeviceVulkan::createRenderTargetWindow(window::Window* f_window)
{
    if (!isValid()) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::createRenderTargetWindow(window::Window* "
                "f_window) " "device needs to be valid"
            );
    }
    auto l_renderingWindow = std::make_shared<render_target::RenderTargetWindowVulkan>();
    l_renderingWindow->setSurface(
        f_window->createVulkanSurface(m_parentApi->getNativeHandle()),
        m_parentApi->getNativeHandle()
    );
    return l_renderingWindow;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::addExtension(const char* f_extensionName)
{
    if (m_valid) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::addExtension(const char* f_extensionName) you "
                "can " "only add extensions prior device creation"
            );
    }
    m_requiredExtension.push_back(f_extensionName);
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::addExtensions(
    std::vector<const char*>& f_extensionNames
)
{
    if (m_valid) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::addExtension(std::vector<const char*>& "
            "f_extensionNames) you can " "only add extensions prior device creation"
        );
    }
    m_requiredExtension.insert(
        m_requiredExtension.end(), f_extensionNames.begin(), f_extensionNames.end()
    );
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::addQueue(vk::QueueFlagBits f_queueType)
{
    if (m_valid) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::addQueue(vk::QueueFlagBits f_queueType) you can "
                "only " "add queues before device creation"
            );
    }
    m_requiredQueues.push_back(f_queueType);
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::create()
{
    pickPhysicalDevice();
    m_valid = true;
    return *this;
}

void RenderingDeviceVulkan::pickPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> l_devices =
        m_parentApi->getNativeHandle().enumeratePhysicalDevices();
    const auto l_devIter = std::ranges::find_if(l_devices, [&](const auto& f_device) {
        // Check if the device supports the Vulkan 1.4 API version
        bool l_supportsVulkan1_4 = f_device.getProperties().apiVersion
                                >= VK_API_VERSION_1_4;

        // Check if any of the queue families support graphics operations
        auto l_queueFamilies = f_device.getQueueFamilyProperties();
        bool l_supportsRequestedQueues =
            std::ranges::all_of(m_requiredQueues, [&](vk::QueueFlagBits f_flag) {
                return std::ranges::any_of(l_queueFamilies, [&](const auto& f_qfp) {
                    return !!(f_qfp.queueFlags & f_flag);
                });
            });
        // Check if all required device extensions are available
        auto l_availableDeviceExtensions = f_device.enumerateDeviceExtensionProperties();
        bool l_supportsAllRequiredExtensions = std::ranges::all_of(
            m_requiredExtension,
            [&l_availableDeviceExtensions](const auto& f_requiredDeviceExtension) {
                return std::ranges::any_of(
                    l_availableDeviceExtensions,
                    [f_requiredDeviceExtension](const auto& f_availableDeviceExtension) {
                        return strcmp(
                                   f_availableDeviceExtension.extensionName,
                                   f_requiredDeviceExtension
                               )
                            == 0;
                    }
                );
            }
        );

        auto l_features = f_device.template getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool l_supportsRequiredFeatures =
            l_features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering
            && l_features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
                   .extendedDynamicState;

        return l_supportsVulkan1_4 && l_supportsRequestedQueues
            && l_supportsAllRequiredExtensions && l_supportsRequiredFeatures;
    });
    if (l_devIter != l_devices.end()) {
        m_physicalDevice = *l_devIter;
    }
    else {
        throw std::runtime_error(
            "RenderingDeviceVulkan::pickPhysicalDevice() failed to find a suitable GPU!"
        );
    }
}

}   // namespace rendering_device
}   // namespace renderer
