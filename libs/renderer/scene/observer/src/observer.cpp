#include "renderer/scene/observer/inc/observer.hpp"

namespace renderer {
namespace scene {
void Observer::setCamera(Camera* f_camera)
{
    m_camera = f_camera;
}

Camera* Observer::getCamera() const
{
    return m_camera;
}
}   // namespace scene
}   // namespace renderer
