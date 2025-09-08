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

render_target::RenderTargetWindow* RenderingDeviceVulkan::getRenderTargetWindow()
{
    if (!isValid()) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::getRenderTargetWindow() device needs to be valid"
        );
    }

    if (!m_window) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::getRenderTargetWindow() needs a window "
                "attached to the device"
            );
    }

    if (!m_renderTargetWindow) {
        throw std::runtime_error(
            "RenderingDeviceVulkan::getRenderTargetWindow() no render target window is available"
        );
    }

    return m_renderTargetWindow.get();
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
                "only add queues before device creation"
            );
    }
    m_requiredQueues.push_back(f_queueType);
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::setFeatures(FeatureChain f_featureChain)
{
    if (m_valid) {
        throw std::
            runtime_error(
                "RenderingDeviceVulkan::setFeatures(FeatureChain f_featureChain) you can "
                "only set features before device creation"
            );
    }
    m_requiredFeatures = f_featureChain;
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

RenderingDeviceVulkan& RenderingDeviceVulkan::create()
{
    createRenderTargetWindow();
    pickPhysicalDevice();
    m_valid = true;
    return *this;
}

void RenderingDeviceVulkan::createRenderTargetWindow()
{
    if (!m_window) {
        return;
    }
    m_renderTargetWindow = std::make_unique<render_target::RenderTargetWindowVulkan>();
    m_renderTargetWindow->setSurface(
        m_window->createVulkanSurface(m_parentApi->getNativeHandle()),
        m_parentApi->getNativeHandle()
    );
}

namespace {

template <typename T>
bool satisfies(const T& f_requested, const T& f_supported)
{
    constexpr size_t l_headerSize = offsetof(T, pNext) + sizeof(void*);
    static_assert(std::is_standard_layout_v<T>, "Features must be POD");

    const char* l_reqBytes = reinterpret_cast<const char*>(&f_requested);
    const char* l_supBytes = reinterpret_cast<const char*>(&f_supported);

    const VkBool32* l_reqFlags =
        reinterpret_cast<const VkBool32*>(l_reqBytes + l_headerSize);
    const VkBool32* l_supFlags =
        reinterpret_cast<const VkBool32*>(l_supBytes + l_headerSize);

    size_t l_count = (sizeof(T) - l_headerSize) / sizeof(VkBool32) - 1;
    for (size_t i = 0; i < l_count; i++) {
        if (l_reqFlags[i] && !l_supFlags[i]) {
            return false;
        }
    }
    return true;
}

template <typename... Ts>
bool satisfiesFeatures(
    const vk::StructureChain<Ts...>& f_requested,
    const vk::StructureChain<Ts...>& f_supported
)
{
    return (
        satisfies(f_requested.template get<Ts>(), f_supported.template get<Ts>()) && ...
    );
}
}   // namespace

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

        auto l_features = f_device.template getFeatures2<VULKAN_FEATURE_CHAIN>();
        bool l_supportsRequiredFeatures =
            satisfiesFeatures(m_requiredFeatures, l_features);

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

void RenderingDeviceVulkan::createLogicalDevice()
{
    std::vector<vk::QueueFamilyProperties> l_queueFamilyProperties =
        m_physicalDevice.getQueueFamilyProperties();
}

}   // namespace rendering_device
}   // namespace renderer
