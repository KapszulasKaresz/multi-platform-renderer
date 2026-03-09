#include "renderer/scene/observer/inc/controlled_observer.hpp"

namespace renderer {
namespace scene {
void ControlledObserver::update(float f_deltaTime) {}

void ControlledObserver::setWindow(window::Window* f_window)
{
    m_window = f_window;
}
}   // namespace scene
}   // namespace renderer
