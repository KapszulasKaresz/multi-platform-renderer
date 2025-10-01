#ifndef RENDER_TARGET_WINDOW_VULKAN_HPP_INCLUDED
#define RENDER_TARGET_WINDOW_VULKAN_HPP_INCLUDED

#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "renderer/image/inc/image_vulkan.hpp"
#include "renderer/render_target/inc/render_target_window.hpp"

namespace renderer {
namespace rendering_device {
class RenderingDeviceVulkan;
}   // namespace rendering_device

namespace render_target {

class RenderTargetWindowVulkan : public RenderTargetWindow {
public:
    RenderTargetWindowVulkan(rendering_device::RenderingDeviceVulkan* f_parentDevice);

    RenderTargetWindowVulkan&     setWindow(window::Window* f_window) override final;
    std::shared_ptr<image::Image> getImage() override final;
    std::shared_ptr<image::ImageVulkan> getSwapChainImage();
    std::shared_ptr<image::Image>       getDepthImage() override final;
    glm::ivec2                          getSize() const override final;

    RenderTargetWindowVulkan&
        setSurface(VkSurfaceKHR f_surface, const vk::raii::Instance& f_instance);

    RenderTargetWindowVulkan& setDirectRenderTarget(bool f_directRenderTarget);

    RenderTargetWindowVulkan& create();

    size_t                  getSwapchainImageCount() const;
    void                    recreateSwapChain();
    vk::raii::SwapchainKHR& acquireSwapchain();

    vk::raii::SurfaceKHR&               getSurface();
    std::shared_ptr<image::ImageVulkan> getSwapchainImage(int f_imageIndex);

private:
    void createSwapChain();
    void cleanupSwapChain();

    void createColorResources();
    void createDepthResources();

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& f_capabilities);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& f_availableFormats
    );
    uint32_t chooseSwapMinImageCount(
        const vk::SurfaceCapabilitiesKHR& f_surfaceCapabilities
    );
    vk::PresentModeKHR chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& f_availablePresentModes
    );

    rendering_device::RenderingDeviceVulkan* m_parentDevice{ nullptr };

    vk::raii::SurfaceKHR m_surface{ nullptr };

    bool                                             m_directRenderTarget{ true };
    vk::raii::SwapchainKHR                           m_swapChain{ nullptr };
    vk::Extent2D                                     m_swapChainExtent{};
    vk::SurfaceFormatKHR                             m_swapChainSurfaceFormat{};
    std::vector<std::shared_ptr<image::ImageVulkan>> m_swapChainImages{};
    std::shared_ptr<image::ImageVulkan>              m_depthImage{ nullptr };
    std::shared_ptr<image::ImageVulkan>              m_colorImage{ nullptr };
};

}   // namespace render_target
}   // namespace renderer

#endif
