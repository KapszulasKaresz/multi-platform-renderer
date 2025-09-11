#include "renderer/render_target/inc/render_target_window_vulkan.hpp"

#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/rendering_device/inc/rendering_device_vulkan.hpp"
#include "renderer/window/inc/window.hpp"

namespace renderer {
namespace render_target {
RenderTargetWindowVulkan::RenderTargetWindowVulkan(
    rendering_device::RenderingDeviceVulkan* f_parentDevice
)
    : RenderTargetWindow(),
      m_parentDevice(f_parentDevice)
{}

RenderTargetWindowVulkan& RenderTargetWindowVulkan::setWindow(window::Window* f_window)
{
    m_window = f_window;
    return *this;
}

RenderTargetWindowVulkan& RenderTargetWindowVulkan::setSurface(
    VkSurfaceKHR              f_surface,
    const vk::raii::Instance& f_instance
)
{
    m_surface = vk::raii::SurfaceKHR(f_instance, f_surface);
    return *this;
}

RenderTargetWindowVulkan& RenderTargetWindowVulkan::setDirectRenderTarget(
    bool f_directRenderTarget
)
{
    if (isValid()) {
        throw std::runtime_error(
            "RenderTargetWindowVulkan::setDirectRenderTarget(bool f_directRenderTarget) "
            "you cannot set " "target directness on an already created target"
        );
    }

    m_directRenderTarget = f_directRenderTarget;
    return *this;
}

RenderTargetWindowVulkan& RenderTargetWindowVulkan::create()
{
    createSwapChain();
    m_valid = true;
    return *this;
}

vk::raii::SurfaceKHR& RenderTargetWindowVulkan::getSurface()
{
    return m_surface;
}

void RenderTargetWindowVulkan::createSwapChain()
{
    auto l_surfaceCapabilities =
        m_parentDevice->getPhysicalDevice().getSurfaceCapabilitiesKHR(m_surface);
    m_swapChainExtent        = chooseSwapExtent(l_surfaceCapabilities);
    m_swapChainSurfaceFormat = chooseSwapSurfaceFormat(
        m_parentDevice->getPhysicalDevice().getSurfaceFormatsKHR(m_surface)
    );
    vk::SwapchainCreateInfoKHR l_swapChainCreateInfo{
        .surface          = *m_surface,
        .minImageCount    = chooseSwapMinImageCount(l_surfaceCapabilities),
        .imageFormat      = m_swapChainSurfaceFormat.format,
        .imageColorSpace  = m_swapChainSurfaceFormat.colorSpace,
        .imageExtent      = m_swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = m_directRenderTarget ? vk::ImageUsageFlagBits::eColorAttachment
                                           : vk::ImageUsageFlagBits::eTransferDst,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform     = l_surfaceCapabilities.currentTransform,
        .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode      = chooseSwapPresentMode(
            m_parentDevice->getPhysicalDevice().getSurfacePresentModesKHR(*m_surface)
        ),
        .clipped = true
    };

    m_swapChain =
        vk::raii::SwapchainKHR(m_parentDevice->getLogicalDevice(), l_swapChainCreateInfo);
}

vk::Extent2D RenderTargetWindowVulkan::chooseSwapExtent(
    const vk::SurfaceCapabilitiesKHR& f_capabilities
)
{
    if (f_capabilities.currentExtent.width != 0xFF'FF'FF'FF) {
        return f_capabilities.currentExtent;
    }

    return { std::clamp<uint32_t>(
                 m_window->getWidth(),
                 f_capabilities.minImageExtent.width,
                 f_capabilities.maxImageExtent.width
             ),
             std::clamp<uint32_t>(
                 m_window->getHeight(),
                 f_capabilities.minImageExtent.height,
                 f_capabilities.maxImageExtent.height
             ) };
}

vk::SurfaceFormatKHR RenderTargetWindowVulkan::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& f_availableFormats
)
{
    assert(!f_availableFormats.empty());
    const auto l_formatIt =
        std::ranges::find_if(f_availableFormats, [&](const auto& f_format) {
            return f_format.format == image::ImageVulkan::convertToVkFormat(m_format)
                && f_format.colorSpace
                       == image::ImageVulkan::convertToVkColorSpace(m_colorSpace);
        });
    return l_formatIt != f_availableFormats.end() ? *l_formatIt : f_availableFormats[0];
}

uint32_t RenderTargetWindowVulkan::chooseSwapMinImageCount(
    const vk::SurfaceCapabilitiesKHR& f_surfaceCapabilities
)
{
    auto l_minImageCount = std::max(3u, f_surfaceCapabilities.minImageCount);
    if ((0 < f_surfaceCapabilities.maxImageCount)
        && (f_surfaceCapabilities.maxImageCount < l_minImageCount))
    {
        l_minImageCount = f_surfaceCapabilities.maxImageCount;
    }
    return l_minImageCount;
}

vk::PresentModeKHR RenderTargetWindowVulkan::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& f_availablePresentModes
)
{
    assert(std::ranges::any_of(f_availablePresentModes, [](auto f_presentMode) {
        return f_presentMode == vk::PresentModeKHR::eFifo;
    }));
    return std::ranges::any_of(
               f_availablePresentModes,
               [](const vk::PresentModeKHR f_value) {
                   return vk::PresentModeKHR::eMailbox == f_value;
               }
           )
             ? vk::PresentModeKHR::eMailbox
             : vk::PresentModeKHR::eFifo;
}

}   // namespace render_target
}   // namespace renderer
