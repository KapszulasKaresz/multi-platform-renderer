#include "renderer/rendering_api/inc/rendering_api_vulkan.hpp"

#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"

namespace renderer {
namespace rendering_api {
RenderingApiVulkan::RenderingApiVulkan() : RenderingApi()
{
    m_apiType = RENDERING_API_TYPE_VULKAN;
}

std::shared_ptr<rendering_device::RenderingDevice>
    RenderingApiVulkan::createRenderingDevice()
{
    return std::make_shared<rendering_device::RenderingDeviceVulkan>();
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

    m_valid = true;
    return *this;
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

RenderingApiVulkan::~RenderingApiVulkan() {}

}   // namespace rendering_api
}   // namespace renderer
