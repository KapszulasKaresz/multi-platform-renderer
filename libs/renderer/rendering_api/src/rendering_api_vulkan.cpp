#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"

#include <iostream>

#include "renderer/render_target/inc/render_target_window_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace rendering_api {
RenderingApiVulkan::RenderingApiVulkan() : RenderingApi()
{
    m_apiType = RENDERING_API_TYPE_VULKAN;
}

std::shared_ptr<rendering_device::RenderingDevice>
    RenderingApiVulkan::getMainRenderingDevice()
{
    if (!m_mainRenderingDevice) {
        auto l_renderingDevice = createRenderingDevice();
        auto l_renderingDeviceVulkanRaw =
            dynamic_cast<rendering_device::RenderingDeviceVulkan*>(
                l_renderingDevice.get()
            );
        if (l_renderingDeviceVulkanRaw == nullptr) {
            throw std::
                runtime_error(
                    "RenderingApiVulkan::getMainRenderingDevice() somehow "
                    "renderingdevice " "isn't a vulkan rendering device"
                );
        }
        l_renderingDeviceVulkanRaw->addQueue(vk::QueueFlagBits::eGraphics)
            .addQueue(vk::QueueFlagBits::eCompute)
            .addExtension(vk::KHRSwapchainExtensionName)
            .addExtension(vk::KHRSpirv14ExtensionName)
            .addExtension(vk::KHRSynchronization2ExtensionName)
            .addExtension(vk::KHRCreateRenderpass2ExtensionName)
            .setFeatures(
                rendering_device::RenderingDeviceVulkan::FeatureChain{
                    {}, // vk::PhysicalDeviceFeatures2
                    {}, // vk::PhysicalDeviceVulkan11Features
                    {}, // vk::PhysicalDeviceVulkan12Features
                    { .synchronization2 = true,
                     .dynamicRendering = true }, // vk::PhysicalDeviceVulkan13Features
                    {}, // vk::PhysicalDeviceVulkan14Features
                    { .extendedDynamicState =
                          true }  // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        }
            )
            .create();

        m_mainRenderingDevice = l_renderingDevice;
    }

    return m_mainRenderingDevice;
}

void RenderingApiVulkan::createMainRenderingDeviceWindow(window::Window* f_window)
{
    if (!m_mainRenderingDevice) {
        auto l_renderingDevice = createRenderingDevice();
        auto l_renderingDeviceVulkanRaw =
            dynamic_cast<rendering_device::RenderingDeviceVulkan*>(
                l_renderingDevice.get()
            );
        if (l_renderingDeviceVulkanRaw == nullptr) {
            throw std::
                runtime_error(
                    "RenderingApiVulkan::createMainRenderingDeviceWindow() somehow "
                    "renderingdevice " "isn't a vulkan rendering device"
                );
        }
        l_renderingDeviceVulkanRaw->addQueue(vk::QueueFlagBits::eGraphics)
            .addQueue(vk::QueueFlagBits::eCompute)
            .addExtension(vk::KHRSwapchainExtensionName)
            .addExtension(vk::KHRSpirv14ExtensionName)
            .addExtension(vk::KHRSynchronization2ExtensionName)
            .addExtension(vk::KHRCreateRenderpass2ExtensionName)
            .setFeatures(
                rendering_device::RenderingDeviceVulkan::FeatureChain{
                    {}, // vk::PhysicalDeviceFeatures2
                    {}, // vk::PhysicalDeviceVulkan11Features
                    {}, // vk::PhysicalDeviceVulkan12Features
                    { .synchronization2 = true,
                     .dynamicRendering = true }, // vk::PhysicalDeviceVulkan13Features
                    {}, // vk::PhysicalDeviceVulkan14Features
                    { .extendedDynamicState =
                          true }  // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        }
            )
            .setWindow(f_window)
            .create();

        m_mainRenderingDevice = l_renderingDevice;
    }
    else {
        throw std::
            runtime_error(
                "RenderingApiVulkan::createMainRenderingDeviceWindow() main "
                "renderingdevice " "already created"
            );
    }
}

std::shared_ptr<rendering_device::RenderingDevice>
    RenderingApiVulkan::createRenderingDevice()
{
    return std::make_shared<rendering_device::RenderingDeviceVulkan>(this);
}

RenderingApiVulkan& RenderingApiVulkan::enableValidationLayers(bool f_enable)
{
    m_validationLayersEnabled = f_enable;
    return *this;
}

RenderingApiVulkan& RenderingApiVulkan::addValidationLayer(
    const char* f_validationLayerName
)
{
    m_validationLayers.push_back(f_validationLayerName);
    return *this;
}

RenderingApiVulkan& RenderingApiVulkan::addExtension(const char* f_extensionName)
{
    m_extensions.push_back(f_extensionName);
    return *this;
}

RenderingApiVulkan& RenderingApiVulkan::addExtensions(
    std::vector<const char*>& f_extensionNames
)
{
    m_extensions.insert(
        m_extensions.end(), f_extensionNames.begin(), f_extensionNames.end()
    );
    return *this;
}

RenderingApiVulkan& RenderingApiVulkan::create()
{
    createInstance();
    setupDebugMessenger();

    m_valid = true;
    return *this;
}

vk::raii::Instance& RenderingApiVulkan::getNativeHandle()
{
    return m_instance;
}

void RenderingApiVulkan::createInstance()
{
    vk::ApplicationInfo l_applicationInfo{ .apiVersion = vk::ApiVersion14 };

    // Check if validation layers are available
    if (m_validationLayersEnabled) {
        auto l_layerProperties = m_context.enumerateInstanceLayerProperties();
        for (const auto& l_requiredLayer : m_validationLayers) {
            if (std::ranges::none_of(
                    l_layerProperties, [l_requiredLayer](const auto& f_layerProperty) {
                        return strcmp(f_layerProperty.layerName, l_requiredLayer) == 0;
                    }
                ))
            {
                throw std::runtime_error(
                    "RenderingApiVulkan::createInstance() Required layer isn't "
                    "supported: "
                    + std::string(l_requiredLayer)
                );
            }
        }
    }

    // Check for extension availabilty
    auto l_extensionProperties = m_context.enumerateInstanceExtensionProperties();
    for (const auto& l_requiredExtension : m_extensions) {
        if (std::ranges::none_of(
                l_extensionProperties,
                [l_requiredExtension](const auto& f_extensionProperty) {
                    return strcmp(f_extensionProperty.extensionName, l_requiredExtension)
                        == 0;
                }
            ))
        {
            throw std::runtime_error(
                "RenderingApiVulkan::createInstance() Required extension isn't "
                "supported: "
                + std::string(l_requiredExtension)
            );
        }
    }

    vk::InstanceCreateInfo l_createInfo{
        .pApplicationInfo        = &l_applicationInfo,
        .enabledLayerCount       = static_cast<uint32_t>(m_validationLayers.size()),
        .ppEnabledLayerNames     = m_validationLayers.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(m_extensions.size()),
        .ppEnabledExtensionNames = m_extensions.data()
    };

    m_instance = vk::raii::Instance(m_context, l_createInfo);
}

namespace {
VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT      f_severity,
    vk::DebugUtilsMessageTypeFlagsEXT             f_type,
    const vk::DebugUtilsMessengerCallbackDataEXT* f_CallbackData,
    void*
)
{
    if (f_severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        || f_severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
        std::cout << "validation layer: type " << to_string(f_type)
                  << " msg: " << f_CallbackData->pMessage << std::endl;
    }

    return vk::False;
}
}   // namespace

void RenderingApiVulkan::setupDebugMessenger()
{
    if (!m_validationLayersEnabled) {
        return;
    }

    vk::DebugUtilsMessageSeverityFlagsEXT l_severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );
    vk::DebugUtilsMessageTypeFlagsEXT l_messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    );
    vk::DebugUtilsMessengerCreateInfoEXT l_debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = l_severityFlags,
        .messageType     = l_messageTypeFlags,
        .pfnUserCallback = &debugCallback
    };
    m_debugMessenger =
        m_instance.createDebugUtilsMessengerEXT(l_debugUtilsMessengerCreateInfoEXT);
}

RenderingApiVulkan::~RenderingApiVulkan() {}

}   // namespace rendering_api
}   // namespace renderer
