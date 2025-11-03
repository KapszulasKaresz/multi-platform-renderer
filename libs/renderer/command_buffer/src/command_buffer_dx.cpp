#include "renderer/command_buffer/inc/command_buffer_dx.hpp"

#include <stdexcept>

#include "renderer/rendering_device/inc/rendering_device_dx.hpp"

namespace renderer {
namespace command_buffer {
CommandBufferDX::CommandBufferDX(rendering_device::RenderingDeviceDX* f_parentDevice)
    : CommandBuffer(),
      m_parentDevice(f_parentDevice)
{}

CommandBufferDX& CommandBufferDX::setRendering(bool f_rendering)
{
    m_rendering = f_rendering;
    return *this;
}

CommandBufferDX& CommandBufferDX::setUsage(bool f_usage)
{
    m_singleUse = f_usage;
    return *this;
}

CommandBufferDX& CommandBufferDX::create()
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::submit()
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::reset()
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::begin()
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::end()
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::beginRendering(const RenderBeginInfo& f_renderBeginInfo)
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::endRendering()
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::useMaterial(
    std::shared_ptr<material::Material> f_material
)
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::useViewport(const ViewportInfo& f_viewportInfo)
{
    throw std::logic_error("Function not yet implemented");
}

CommandBufferDX& CommandBufferDX::draw(std::shared_ptr<mesh::TriangleMesh> f_mesh)
{
    throw std::logic_error("Function not yet implemented");
}
}   // namespace command_buffer
}   // namespace renderer
